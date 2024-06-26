#include "vtkA2PacketInterpreter.h"

#include "vtkHelper.h"
#include <vtkDoubleArray.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkTransform.h>

#include <bitset>
#include <boost/foreach.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include <fenv.h>
#include <math.h>

#include <chrono>
#include <vtkDelimitedTextReader.h>

#define TEST_LASER_NUM (128) /* Just for testing */

//#define FIX_WRONG_PCAP_PROBLEM

using std::chrono::duration;
using std::chrono::duration_cast;
using std::chrono::high_resolution_clock;
using std::chrono::milliseconds;

namespace
{

double degreeToRadian(double degree)
{
  return degree * vtkMath::Pi() / 180.0;
}
}

//! @todo this method are actually usefull for every Interpreter and should go to the top
template<typename T>
vtkSmartPointer<T> vtkA2PacketInterpreter::CreateDataArray(bool isAdvanced, const char* name,
  vtkIdType vtkNotUsed(np), vtkIdType prereserved_np, vtkPolyData* pd)
{
  if (isAdvanced && !this->EnableAdvancedArrays)
  {
    return nullptr;
  }
  vtkSmartPointer<T> array = vtkSmartPointer<T>::New();
  array->SetNumberOfValues(prereserved_np);
  array->SetName(name);
  if (pd)
  {
    pd->GetPointData()->AddArray(array);
  }

  array->FillValue(0);
  return array;
}

template<typename T, typename U>
void TrySetValue(T& array, int pos, U value)
{
  if (array != nullptr)
  {
    array->SetValue(pos, value);
  }
}

//-----------------------------------------------------------------------------
vtkStandardNewMacro(vtkA2PacketInterpreter)

//-----------------------------------------------------------------------------
vtkA2PacketInterpreter::vtkA2PacketInterpreter()
{
  std::cout << "Size of A2 Packet = " << sizeof(A2Packet) << std::endl;

  // Initialize Elevation and Azimuth correction
  for (int i = 0; i < TEST_LASER_NUM; i++)
  {
    if (elev_angle[i])
    {
      this->ElevationCorrection.push_back(elev_angle[i]);
    }
    if (azimuth_offset[i])
    {
      this->AzimuthCorrection.push_back(azimuth_offset[i]);
    }
  }

  // SpecificFrameInformation placeholder for specific sensor implementation
  // Meta data required to correctly parse the data contained within the udp packets
  this->ParserMetaData.SpecificInformation = std::make_shared<AsensingSpecificFrameInformation>();

  for (int j = 0; j < CIRCLE; j++)
  {
    double angle = j / 100.0;
    this->Cos_all_angle.push_back(std::cos(degreeToRadian(angle)));
    this->Sin_all_angle.push_back(std::sin(degreeToRadian(angle)));
  }
}

//-----------------------------------------------------------------------------
vtkA2PacketInterpreter::~vtkA2PacketInterpreter() {}

#include "cJSON.h"
//-----------------------------------------------------------------------------
void vtkA2PacketInterpreter::LoadCalibration(const std::string& filename)
{
  if (filename.empty())
  {
    this->IsCalibrated = false;
    return;
  }

  // Load the JSON file
  // 1st Column = X vector
  // 2nd Column = Y vector
  // 3rd Column = Z vector
  // 4th Column = Z' vector
  // Notes: Effective for each 200 separate arrays

  std::cout << "[A2] Load Calibration File: " << filename << std::endl;

  std::string keyword("A2-Correction");
  std::size_t found = filename.find(keyword);
  if (std::string::npos == found)
  {
    std::cout << "[A2] Do not need to correct data" << std::endl;
    this->CalibEnabled = false;
    this->IsCalibrated = true;
    return;
  }

  /* Correct point cloud */
  FILE* fp = fopen(filename.c_str(), "rb");
  if (fp == NULL)
  {
    std::cout << "[A2] Can not open " << filename << std::endl;
  }

  std::cout << "[A2] Parse calibration file ..." << std::endl;

  fseek(fp, 0, SEEK_SET);
  long begin = ftell(fp);
  fseek(fp, 0, SEEK_END);
  long end = ftell(fp);
  long filesize = end - begin;
  fseek(fp, 0, SEEK_SET);

  char* jsonData = (char*)malloc(filesize);

  std::cout << "Open " << filename << std::endl;

  while (fread(jsonData, 1, filesize, fp) == 1)
  {
  }

  cJSON* root = cJSON_Parse(jsonData);
  cJSON* elevations = cJSON_GetObjectItem(root, "elevation");
  cJSON* azimuths = cJSON_GetObjectItem(root, "azimuth");
  cJSON* faces = cJSON_GetObjectItem(root, "face");
  cJSON* elevation_mirror_offset = cJSON_GetObjectItem(root, "elevation_mirror_offset");
  cJSON* channels = cJSON_GetObjectItem(root, "channel");
  cJSON* filter_point_id = cJSON_GetObjectItem(root, "filter_point_id");
  cJSON* elevation_mirror_offset_enable = cJSON_GetObjectItem(root, "elevation_mirror_offset_enable");

  if (cJSON_IsArray(elevations)) 
  {
    printf("elevation offset:\n%s\n\n", cJSON_Print(elevations));

    int ElevationSize = cJSON_GetArraySize(elevations);
    if (A2_CHANNEL_NUM == ElevationSize) {
      for (int i = 0; i < A2_CHANNEL_NUM; i++)
      {
        elevation_offset_[i] = cJSON_GetArrayItem(elevations, i)->valuedouble;
      }
    }
    else {
      vtkWarningMacro("[A2] Invalid calibration data");
    }
  }

  if (cJSON_IsArray(azimuths)) 
  {
    printf("azimuth offset:\n%s\n\n", cJSON_Print(azimuths));

    int AzimuthSize = cJSON_GetArraySize(azimuths);
    if (A2_CHANNEL_NUM == AzimuthSize) {
      for (int i = 0; i < A2_CHANNEL_NUM; i++)
      {
        azimuth_offset_[i] = cJSON_GetArrayItem(azimuths, i)->valuedouble;
      }
    }
    else {
      vtkWarningMacro("[A2] Invalid calibration data");
    }
  }

  if (cJSON_IsArray(faces))
  {
    printf("face offset:\n%s\n\n", cJSON_Print(faces));

    int FaceSize = cJSON_GetArraySize(faces);
    if (4 == FaceSize) {
      for (int i = 0; i < 4; i++)
      {
        this->faces[i] = cJSON_GetArrayItem(faces, i)->valueint;
      }
    }
    else {
      vtkWarningMacro("[A2] Invalid calibration data");
    }
  }

  if (cJSON_IsArray(elevation_mirror_offset))
  {
    printf("elevation offset:\n%s\n\n", cJSON_Print(elevation_mirror_offset));

    int size = cJSON_GetArraySize(elevation_mirror_offset);
    if (4 == size) {
      for (int i = 0; i < 4; i++)
      {
        this->elevation_mirror_offset[i] = cJSON_GetArrayItem(elevation_mirror_offset, i)->valuedouble;
      }
    }
    else {
      vtkWarningMacro("[A2] Invalid calibration data");
    }
  }

  if (cJSON_IsArray(channels))
  {
    printf("channels :\n%s\n\n", cJSON_Print(channels));

    int channelSize = cJSON_GetArraySize(channels);
    if (A2_CHANNEL_NUM == channelSize) {
      for (int i = 0; i < A2_CHANNEL_NUM; i++)
      {
        this->channels[i] = cJSON_GetArrayItem(channels, i)->valueint;
      }
    }
    else {
      vtkWarningMacro("[A2] Invalid calibration data");
    }
  }

  this->filter_point_id = filter_point_id->valueint;
  this->elevation_mirror_offset_enable = elevation_mirror_offset_enable->valueint;

  cJSON_Delete(root);
  free(jsonData);
  fclose(fp);

  this->IsCalibrated = true;
  this->CalibEnabled = true;
}

void vtkA2PacketInterpreter::ProcessPacket(unsigned char const* data, unsigned int dataLength)
{
  auto start = high_resolution_clock::now();
  if (!this->IsLidarPacket(data, dataLength))
  {
    return;
  }

  const A2Packet* dataPacket = reinterpret_cast<const A2Packet*>(data);
  current_frame_id = dataPacket->header.GetFrameID();
  auto face_id = current_frame_id % 4;
  if(this->faces[face_id] == 0) return ;

  struct tm t;
  t.tm_year = dataPacket->header.GetUTCTime0();
  t.tm_mon = dataPacket->header.GetUTCTime1() - 1;
  t.tm_mday = dataPacket->header.GetUTCTime2();
  t.tm_hour = dataPacket->header.GetUTCTime3();
  t.tm_min = dataPacket->header.GetUTCTime4();
  t.tm_sec = dataPacket->header.GetUTCTime5();
  t.tm_isdst = 0;

  // Time in second of the packets
  time_t unix_second = (mktime(&t));
  int returnMode = 0;

  // Timestamp contains in the packet
  // roll back every second, probably in microsecond
  uint32_t timestampPacket = dataPacket->header.GetTimestamp();

  // Timestamp in second of the packet
  double timestamp = unix_second + (timestampPacket / 1000000.0);
  channel_num = dataPacket->header.GetChannelNum();
  current_seq_num = dataPacket->header.GetSeqNum();
  seq_num_counter++;

  if (dataPacket->header.GetPointNum() > 0) {
    this->points_per_frame = dataPacket->header.GetPointNum();
  }

  // [HACK start] Proccess only one return in case of dual mode for performance issue
  int start_block = 0;
  int end_block = A2_BLOCK_NUM;
  if (returnMode == 0x39 || returnMode == 0x3B || returnMode == 0x3C)
  {
    end_block = 1;
  }
  // [HACK end]

  for (int blockID = start_block; blockID < end_block; blockID++)
  {
      A2Block currentBlock = dataPacket->blocks[blockID];

      AsensingSpecificFrameInformation* frameInfo = reinterpret_cast<AsensingSpecificFrameInformation*>(this->ParserMetaData.SpecificInformation.get());
      if (frameInfo->IsNewFrame(1, current_frame_id))
      {
      #if PACKET_STAT_DEBUG
          /* If fewer UDP packets are received than expected, means packet loss */
          if (current_frame_id > 0 && this->seq_num_counter < (this->points_per_frame / A2_POINT_PER_PACKET)) {
              vtkWarningMacro(<< "Incomplete frame (id: " << (current_frame_id - 1)
                              << ", packets: " << seq_num_counter
                              << ", total: " << (this->points_per_frame / A2_POINT_PER_PACKET)
                              << ", lsn: " << this->last_seq_num
                              << ", points: " << this->points_per_frame << ")" );
          }
      #endif
          this->SplitFrame();
          this->seq_num_counter = 0;
      }

      for (int chan = 0; chan < A2_CHANNEL_NUM; chan++)
      {
          const A2Unit &unit = currentBlock.units[chan];

          double x, y, z, azimuth, pitch;
          double distance = static_cast<double>(unit.GetDistance()) * ASENSING_DISTANCE_UNIT;
          auto confidence = unit.GetConfidence();
          if (0 == currentBlock.GetAzimuth() && 0 == unit.GetDistance() && 0 == unit.GetIntensity()) {
              continue;
          }

          {
              auto evevation_offset = dataPacket->header.GetReserved1() * ASENSING_ELEVATION_UNIT;
              azimuth = static_cast<float>(currentBlock.GetAzimuth()) * ASENSING_AZIMUTH_UNIT + azimuth_offset_[chan];
			  pitch = elevation_offset_[chan] + (this->elevation_mirror_offset_enable ? this->elevation_mirror_offset[face_id] : evevation_offset);
			  if (pitch < 0)
			  {
				  pitch += 360.0f;
			  }
			  else if (pitch >= 360.0f)
			  {
				  pitch -= 360.0f;
			  }
			  float xyDistance = distance * this->Cos_all_angle[static_cast<int>(pitch * 100 + 0.5)];
              int azimuthIdx = static_cast<int>(azimuth * 100 + 0.5);
              if (azimuthIdx >= CIRCLE)
              {
                  azimuthIdx -= CIRCLE;
              }
              else if (azimuthIdx < 0)
              {
                  azimuthIdx += CIRCLE;
              }

          #if USING_MATH_LIB
              double x = xyDistance * sin(degreeToRadian(azimuth)); // this->Sin_all_angle[azimuthIdx];
              double y = xyDistance * cos(degreeToRadian(azimuth)); // this->Cos_all_angle[azimuthIdx];
              double z = distance * sin(degreeToRadian(pitch));     // this->Sin_all_angle[static_cast<int>(pitch * 100 + 0.5)];
          #else
              x = xyDistance * this->Cos_all_angle[azimuthIdx];
              y = xyDistance * this->Sin_all_angle[azimuthIdx];
              z = distance * this->Sin_all_angle[static_cast<int>(pitch * 100 + 0.5)];
          #endif
			  if (azimuth > 180)
				  azimuth -= 360.0f;

			  if (pitch > 180)
				  pitch -= 360.0f;
          } /* End of this->CalibEnabled */

          uint8_t intensity = unit.GetIntensity();

          // Compute timestamp of the point
          // int offset = currentBlock.GettimeOffSet();
          // timestamp += offset;

          #if DEBUG
          //std::cout << "Point " << current_frame_id << ": " << distance << ", " << azimuth << ", " << pitch << ", " << x << ", " << y << ", " << z << std::endl;
          std::cout << "Point " << current_frame_id << ": " << currentBlock.units[chan].GetDistance()
                    << ", " << currentBlock.units[chan].GetAzimuth()
                    << ", " << currentBlock.units[chan].GetElevation() << ", " << x << ", " << y << ", " << z << std::endl;
          #endif

          if (current_pt_id >= this->points_per_frame)
          {
          #if PACKET_STAT_DEBUG
              // SplitFrame for safety to not overflow allcoated arrays
              vtkWarningMacro(<< "Received more datapoints than expected" << " (" << current_pt_id << ", " << current_frame_id << ")");

              if (current_frame_id > 0 && this->seq_num_counter < (this->points_per_frame / A2_POINT_PER_PACKET)) {

                  vtkWarningMacro(<< "Incomplete frame 2 (id: " << (current_frame_id - 1)
                                  << ", packets: " << seq_num_counter
                                  << ", total: " << (this->points_per_frame / A2_POINT_PER_PACKET)
                                  << ", lsn: " << this->last_seq_num
                                  << ", points: " << this->points_per_frame << ")" );
              }
          #endif
              this->SplitFrame();
              this->seq_num_counter = 0;
          }

          if(this->channels[chan] == 0) {
              this->Points->SetPoint(current_pt_id, NAN, NAN, NAN);
              TrySetValue(this->PointsX, current_pt_id, NAN);
              TrySetValue(this->PointsY, current_pt_id, NAN);
              TrySetValue(this->PointsZ, current_pt_id, NAN);
              TrySetValue(this->Azimuth, current_pt_id, NAN);
              TrySetValue(this->Elevation, current_pt_id, NAN);
              TrySetValue(this->PointID, current_pt_id, current_pt_id);
              TrySetValue(this->Seq, current_pt_id, NAN);
              TrySetValue(this->FaceID, current_pt_id, NAN);
              TrySetValue(this->Channel, current_pt_id, NAN);
              TrySetValue(this->Confidence, current_pt_id, NAN);
              TrySetValue(this->Intensities, current_pt_id, NAN);
              TrySetValue(this->Timestamps, current_pt_id, NAN);
              TrySetValue(this->Distances, current_pt_id, NAN);
          }
          else {
              if((this->filter_point_id != -1 && filter_point_id == (int)current_pt_id) || this->filter_point_id == -1) {
                  this->Points->SetPoint(current_pt_id, x, y, z);
                  TrySetValue(this->PointsX, current_pt_id, x);
                  TrySetValue(this->PointsY, current_pt_id, y);
                  TrySetValue(this->PointsZ, current_pt_id, z);
                  TrySetValue(this->Azimuth, current_pt_id, azimuth);
                  TrySetValue(this->Elevation, current_pt_id, pitch);
                  TrySetValue(this->PointID, current_pt_id, current_pt_id);
                  TrySetValue(this->Seq, current_pt_id, current_seq_num);
                  TrySetValue(this->FaceID, current_pt_id, face_id);
                  TrySetValue(this->Channel, current_pt_id, chan);
                  TrySetValue(this->Confidence, current_pt_id, confidence);
                  TrySetValue(this->Intensities, current_pt_id, intensity);
                  TrySetValue(this->Timestamps, current_pt_id, timestamp);
                  TrySetValue(this->Distances, current_pt_id, distance);
              }
              else {
                  this->Points->SetPoint(current_pt_id, NAN, NAN, NAN);
                  TrySetValue(this->PointsX, current_pt_id, NAN);
                  TrySetValue(this->PointsY, current_pt_id, NAN);
                  TrySetValue(this->PointsZ, current_pt_id, NAN);
                  TrySetValue(this->Azimuth, current_pt_id, NAN);
                  TrySetValue(this->Elevation, current_pt_id, NAN);
                  TrySetValue(this->PointID, current_pt_id, current_pt_id);
                  TrySetValue(this->Seq, current_pt_id, NAN);
                  TrySetValue(this->FaceID, current_pt_id, NAN);
                  TrySetValue(this->Channel, current_pt_id, NAN);
                  TrySetValue(this->Confidence, current_pt_id, NAN);
                  TrySetValue(this->Intensities, current_pt_id, NAN);
                  TrySetValue(this->Timestamps, current_pt_id, NAN);
                  TrySetValue(this->Distances, current_pt_id, NAN);
              }
          }
          current_pt_id++;
      }
  }

  this->last_seq_num = current_seq_num;

  auto stop = high_resolution_clock::now();
  duration<double, std::micro> ms_double = stop - start;
  // std::cout << ms_double.count() << "micro seconds\n";
}

//-----------------------------------------------------------------------------
bool vtkA2PacketInterpreter::IsLidarPacket(
  unsigned char const* data, unsigned int dataLength)
{
#if CHECK_LIDAR_PACKET
  if (dataLength != sizeof(struct A2Packet)) {
      vtkWarningMacro("Invaild point cloud data packet (length mismatch)");
      return false;
  }

  /* Check sob flag of packet header */
  if (data[0] == 0x55 && data[1] == 0xa2)
  {
      return true;
  }
  else
  {
      vtkWarningMacro("Invaild point cloud data packet (header flag mismatch)");
      return false;
  }
#endif /* CHECK_LIDAR_PACKET */

  // std::cout << "Process Packet, Size = " << dataLength << std::endl;
  // std::cout << "dataLength  " << dataLength << std::endl;
  // std::cout << "ASENSING_PACKET_SIZE  " << ASENSING_PACKET_SIZE << std::endl;
  // std::cout << "sizeof(AsensingPacket)  " << sizeof(AsensingPacket) << std::endl;

  return true; // dataLength == PACKET_SIZE;
}

//-----------------------------------------------------------------------------
vtkSmartPointer<vtkPolyData> vtkA2PacketInterpreter::CreateNewEmptyFrame(
  vtkIdType numberOfPoints, vtkIdType vtkNotUsed(prereservedNumberOfPoints))
{
  const int defaultPrereservedNumberOfPointsPerFrame = this->points_per_frame;
  vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();

  // points
  current_pt_id = 0;
  current_seq_num = 0;
  vtkNew<vtkPoints> points;
  points->SetDataTypeToFloat();
  points->SetNumberOfPoints(defaultPrereservedNumberOfPointsPerFrame);
  points->GetData()->SetName("Points_m_XYZ");
  polyData->SetPoints(points.GetPointer());

  // intensity
  this->Points = points.GetPointer();
  this->PointsX = CreateDataArray<vtkDoubleArray>(
    true, "X", numberOfPoints, defaultPrereservedNumberOfPointsPerFrame, polyData);
  this->PointsY = CreateDataArray<vtkDoubleArray>(
    true, "Y", numberOfPoints, defaultPrereservedNumberOfPointsPerFrame, polyData);
  this->PointsZ = CreateDataArray<vtkDoubleArray>(
    true, "Z", numberOfPoints, defaultPrereservedNumberOfPointsPerFrame, polyData);
  this->Azimuth = CreateDataArray<vtkDoubleArray>(
    false, "Azimuth", numberOfPoints, defaultPrereservedNumberOfPointsPerFrame, polyData);
  this->Elevation = CreateDataArray<vtkDoubleArray>(
    false, "Elevation", numberOfPoints, defaultPrereservedNumberOfPointsPerFrame, polyData);

  this->PointID = CreateDataArray<vtkUnsignedIntArray>(
    false, "PointID", numberOfPoints, defaultPrereservedNumberOfPointsPerFrame, polyData);
  this->Seq = CreateDataArray<vtkUnsignedIntArray>(
    false, "Seq", numberOfPoints, defaultPrereservedNumberOfPointsPerFrame, polyData);
  this->FaceID = CreateDataArray<vtkUnsignedCharArray>(
    false, "FaceID", numberOfPoints, defaultPrereservedNumberOfPointsPerFrame, polyData);
  this->Channel = CreateDataArray<vtkUnsignedCharArray>(
    false, "Channel", numberOfPoints, defaultPrereservedNumberOfPointsPerFrame, polyData);
  this->Confidence = CreateDataArray<vtkUnsignedCharArray>(
    false, "Confidence", numberOfPoints, defaultPrereservedNumberOfPointsPerFrame, polyData);
  this->Intensities = CreateDataArray<vtkUnsignedCharArray>(
    false, "Intensity", numberOfPoints, defaultPrereservedNumberOfPointsPerFrame, polyData);
  this->Timestamps = CreateDataArray<vtkDoubleArray>(
    false, "Timestamp", numberOfPoints, defaultPrereservedNumberOfPointsPerFrame, polyData);
  this->Distances = CreateDataArray<vtkDoubleArray>(
    false, "Distance", numberOfPoints, defaultPrereservedNumberOfPointsPerFrame, polyData);
  polyData->GetPointData()->SetActiveScalars("Intensity");
  return polyData;
}

//-----------------------------------------------------------------------------
bool vtkA2PacketInterpreter::PreProcessPacket(unsigned char const* data,
  unsigned int vtkNotUsed(dataLength), fpos_t filePosition, double packetNetworkTime,
  std::vector<FrameInformation>* frameCatalog)
{
  bool isNewFrame = false;

  this->ParserMetaData.FilePosition = filePosition;
  this->ParserMetaData.FirstPacketDataTime = 0.0; // TODO
  this->ParserMetaData.FirstPacketNetworkTime = packetNetworkTime;

  const A2Packet* dataPacket = reinterpret_cast<const A2Packet*>(data);

  for (int blockID = 0; blockID < A2_BLOCK_NUM; blockID++)
  {
    //A2Block currentBlock = dataPacket->blocks[blockID];

    AsensingSpecificFrameInformation* frameInfo =
      reinterpret_cast<AsensingSpecificFrameInformation*>(
        this->ParserMetaData.SpecificInformation.get());
  #ifdef FIX_WRONG_PCAP_PROBLEM
    if (dataPacket->blocks[0].units->GetAzimuth() == 33500 && dataPacket->blocks[0].units->GetElevation() == 1250 && frameCatalog)
    {
      isNewFrame = true;
      frameCatalog->push_back(this->ParserMetaData);
      break;
    }
  #else
    if (frameInfo->IsNewFrame(0, dataPacket->header.GetFrameID()) && frameCatalog)
    {
      isNewFrame = true;
      frameCatalog->push_back(this->ParserMetaData);
    }
  #endif
  }

  return isNewFrame;
}

//-----------------------------------------------------------------------------
std::string vtkA2PacketInterpreter::GetSensorInformation(bool vtkNotUsed(shortVersion))
{
  return "Asensing A2 LiDAR Sensor";
}
