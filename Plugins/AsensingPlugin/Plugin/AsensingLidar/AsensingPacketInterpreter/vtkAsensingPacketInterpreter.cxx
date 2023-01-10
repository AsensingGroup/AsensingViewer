#include "vtkAsensingPacketInterpreter.h"

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

#define PACKET_STAT_DEBUG
#define TEST_LASER_NUM (128) /* Just for testing */

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
vtkSmartPointer<T> vtkAsensingPacketInterpreter::CreateDataArray(bool isAdvanced, const char* name,
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
vtkStandardNewMacro(vtkAsensingPacketInterpreter)

  //-----------------------------------------------------------------------------
  vtkAsensingPacketInterpreter::vtkAsensingPacketInterpreter()
{
  std::cout << "Size of AsensingPacket = " << sizeof(AsensingPacket) << std::endl;

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
vtkAsensingPacketInterpreter::~vtkAsensingPacketInterpreter() {}

#include "cJSON.h"
//-----------------------------------------------------------------------------
void vtkAsensingPacketInterpreter::LoadCalibration(const std::string& filename)
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

  std::cout << "Load Calibration File: " << filename << std::endl;

  std::string keyword("No-Correction");
  std::size_t found = filename.find(keyword);
  if (std::string::npos != found)
  {
    std::cout << "Do not need to correct data" << std::endl;
    this->CalibEnabled = false;
    this->IsCalibrated = true;
    return;
  }

  /* Correct point cloud */
  std::cout << "Version " << CJSON_VERSION_MAJOR << "." << CJSON_VERSION_MINOR << "."
            << CJSON_VERSION_PATCH << std::endl;

  FILE* fp = fopen(filename.c_str(), "rb");
  if (fp == NULL)
  {
    std::cout << "Can not open " << filename << std::endl;
  }

  fseek(fp, 0, SEEK_SET);
  long begin = ftell(fp);
  fseek(fp, 0, SEEK_END);
  long end = ftell(fp);
  long filesize = end - begin;
  fseek(fp, 0, SEEK_SET);

  char* jsonData = (char*)malloc(filesize);

  std::cout << "Open " << filename << std::endl;

  while (fread(jsonData, filesize, 1, fp) == 1)
  {
  }

  cJSON* root = cJSON_Parse(jsonData);
  // printf("%s\n\n", cJSON_Print(root));

  cJSON* RT0 = cJSON_GetObjectItemCaseSensitive(root, "module_RT0");
  cJSON* RT1 = cJSON_GetObjectItemCaseSensitive(root, "module_RT1");
  cJSON* RT2 = cJSON_GetObjectItemCaseSensitive(root, "module_RT2");
  cJSON* RT3 = cJSON_GetObjectItemCaseSensitive(root, "module_RT3");

  cJSON* module0 = cJSON_GetObjectItemCaseSensitive(root, "module_0");
  cJSON* module1 = cJSON_GetObjectItemCaseSensitive(root, "module_1");
  cJSON* module2 = cJSON_GetObjectItemCaseSensitive(root, "module_2");
  cJSON* module3 = cJSON_GetObjectItemCaseSensitive(root, "module_3");

  bool NoLaserAngle = false;
  bool NoRTMatrix = false;

  if (!(cJSON_IsArray(module0) && cJSON_IsArray(module1) && cJSON_IsArray(module2) &&
        cJSON_IsArray(module3)))
  {
    std::cout << "Warning: Please check module data" << std::endl;
    NoLaserAngle = true;
  }

  if (!(cJSON_IsArray(RT0) && cJSON_IsArray(RT1) && cJSON_IsArray(RT2) && cJSON_IsArray(RT3)))
  {
    std::cout << "Warning: Please check RT matrix" << std::endl;
    NoRTMatrix = true;
  }

  /* fill in RT matrix */
  if (!NoRTMatrix)
  {

    for (int i = 0; i < 4; i++)
    {
      for (int j = 0; j < 4; j++)
      {
        this->matrix_RT0[i][j] = cJSON_GetArrayItem(RT0, i * 4 + j)->valuedouble;
      }
    }

    for (int i = 0; i < 4; i++)
    {
      for (int j = 0; j < 4; j++)
      {
        this->matrix_RT1[i][j] = cJSON_GetArrayItem(RT1, i * 4 + j)->valuedouble;
      }
    }

    for (int i = 0; i < 4; i++)
    {
      for (int j = 0; j < 4; j++)
      {
        this->matrix_RT2[i][j] = cJSON_GetArrayItem(RT2, i * 4 + j)->valuedouble;
      }
    }

    for (int i = 0; i < 4; i++)
    {
      for (int j = 0; j < 4; j++)
      {
        this->matrix_RT3[i][j] = cJSON_GetArrayItem(RT3, i * 4 + j)->valuedouble;
      }
    }

    this->RTMatEnabled = true;

    /* Print all RT matrix */

    std::cout << "RT matrix 0 :" << std::endl;
    for (int i = 0; i < 4; i++)
    {
      for (int j = 0; j < 4; j++)
      {
        std::cout << this->matrix_RT0[i][j] << ", ";
      }
      std::cout << std::endl;
    }

    std::cout << "RT matrix 1 :" << std::endl;
    for (int i = 0; i < 4; i++)
    {
      for (int j = 0; j < 4; j++)
      {
        std::cout << this->matrix_RT1[i][j] << ", ";
      }
      std::cout << std::endl;
    }

    std::cout << "RT matrix 2 :" << std::endl;
    for (int i = 0; i < 4; i++)
    {
      for (int j = 0; j < 4; j++)
      {
        std::cout << this->matrix_RT2[i][j] << ", ";
      }
      std::cout << std::endl;
    }

    std::cout << "RT matrix 3 :" << std::endl;
    for (int i = 0; i < 4; i++)
    {
      for (int j = 0; j < 4; j++)
      {
        std::cout << this->matrix_RT3[i][j] << ", ";
      }
      std::cout << std::endl;
    }
  }

  /* If no Laser angle data */

  if (!module0)
  {
    cJSON_Delete(root);
    free(jsonData);
    fclose(fp);
    this->IsCalibrated = true;
    this->CalibEnabled = false;
    return;
  }

  /* fill in Laser angle */

  long arraySize = cJSON_GetArraySize(module0); // 9600
  printf("Array Size = %ld\n", arraySize);

  this->XCorrection.clear();
  this->XCorrection.resize(arraySize * 4); // 38400
  this->YCorrection.clear();
  this->YCorrection.resize(arraySize * 4);
  this->ZCorrection.clear();
  this->ZCorrection.resize(arraySize * 4);

  // Channel 0-3
  struct cJSON* element0;
  struct cJSON* element1;
  struct cJSON* element2;
  struct cJSON* element3;
  element0 = module0->child;
  element1 = module1->child;
  element2 = module2->child;
  element3 = module3->child;

  bool isValid = true;
  int counter = 0;

  vtkIdType pointIndex = 0;

  for (vtkIdType index = 0; index < arraySize; ++index)
  {
    if (index != 0 && index % 200 == 0)
    {
      isValid = isValid ? false : true;
    }

    if (!isValid)
    {
      element0 = element0->next;
      element1 = element1->next;
      element2 = element2->next;
      element3 = element3->next;
      // if (element0 == NULL) break;
      continue;
    }

    this->XCorrection[pointIndex * 8 + 0] = cJSON_GetArrayItem(element0, 0)->valuedouble;
    this->YCorrection[pointIndex * 8 + 0] = cJSON_GetArrayItem(element0, 1)->valuedouble;
    this->ZCorrection[pointIndex * 8 + 0] = cJSON_GetArrayItem(element0, 2)->valuedouble;

    this->XCorrection[pointIndex * 8 + 1] = cJSON_GetArrayItem(element0, 0)->valuedouble;
    this->YCorrection[pointIndex * 8 + 1] = cJSON_GetArrayItem(element0, 1)->valuedouble;
    this->ZCorrection[pointIndex * 8 + 1] = cJSON_GetArrayItem(element0, 3)->valuedouble;

    this->XCorrection[pointIndex * 8 + 2] = cJSON_GetArrayItem(element1, 0)->valuedouble;
    this->YCorrection[pointIndex * 8 + 2] = cJSON_GetArrayItem(element1, 1)->valuedouble;
    this->ZCorrection[pointIndex * 8 + 2] = cJSON_GetArrayItem(element1, 2)->valuedouble;

    this->XCorrection[pointIndex * 8 + 3] = cJSON_GetArrayItem(element1, 0)->valuedouble;
    this->YCorrection[pointIndex * 8 + 3] = cJSON_GetArrayItem(element1, 1)->valuedouble;
    this->ZCorrection[pointIndex * 8 + 3] = cJSON_GetArrayItem(element1, 3)->valuedouble;

    this->XCorrection[pointIndex * 8 + 4] = cJSON_GetArrayItem(element2, 0)->valuedouble;
    this->YCorrection[pointIndex * 8 + 4] = cJSON_GetArrayItem(element2, 1)->valuedouble;
    this->ZCorrection[pointIndex * 8 + 4] = cJSON_GetArrayItem(element2, 2)->valuedouble;

    this->XCorrection[pointIndex * 8 + 5] = cJSON_GetArrayItem(element2, 0)->valuedouble;
    this->YCorrection[pointIndex * 8 + 5] = cJSON_GetArrayItem(element2, 1)->valuedouble;
    this->ZCorrection[pointIndex * 8 + 5] = cJSON_GetArrayItem(element2, 3)->valuedouble;

    this->XCorrection[pointIndex * 8 + 6] = cJSON_GetArrayItem(element3, 0)->valuedouble;
    this->YCorrection[pointIndex * 8 + 6] = cJSON_GetArrayItem(element3, 1)->valuedouble;
    this->ZCorrection[pointIndex * 8 + 6] = cJSON_GetArrayItem(element3, 2)->valuedouble;

    this->XCorrection[pointIndex * 8 + 7] = cJSON_GetArrayItem(element3, 0)->valuedouble;
    this->YCorrection[pointIndex * 8 + 7] = cJSON_GetArrayItem(element3, 1)->valuedouble;
    this->ZCorrection[pointIndex * 8 + 7] = cJSON_GetArrayItem(element3, 3)->valuedouble;

    element0 = element0->next;
    element1 = element1->next;
    element2 = element2->next;
    element3 = element3->next;

    if (element0 == NULL || element1 == NULL || element2 == NULL || element3 == NULL)
    {
      std::cout << "Error: Points do not match!" << std::endl;
      break;
    }

    pointIndex++;
  }

  cJSON_Delete(root);
  free(jsonData);
  fclose(fp);

  std::cout << "Close " << filename << std::endl;

  std::cout << "[0] x' = " << XCorrection[0] << ", y' = " << YCorrection[0]
            << ", z' = " << ZCorrection[0] << std::endl;

  this->IsCalibrated = true;
  this->CalibEnabled = true;
}

void vtkAsensingPacketInterpreter::ProcessPacket(unsigned char const* data, unsigned int dataLength)
{
  auto start = high_resolution_clock::now();
  if (!this->IsLidarPacket(data, dataLength))
  {
    return;
  }

  const AsensingPacket* dataPacket = reinterpret_cast<const AsensingPacket*>(data);

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

  int mode = 0;
  int state = 0;
  int returnMode = 0;

  // Timestamp contains in the packet
  // roll back every second, probably in microsecond
  uint32_t timestampPacket = dataPacket->header.GetTimestamp();

  // Timestamp in second of the packet
  double timestamp = unix_second + (timestampPacket / 1000000.0);

  echo_count = dataPacket->header.GetEchoCount();
  current_frame_id = dataPacket->header.GetFrameID();
  current_seq_num = dataPacket->header.GetSeqNum();
  seq_num_counter++;

  if (dataPacket->header.GetPointNum() > 0) {
    this->points_per_frame = dataPacket->header.GetPointNum();
  }

  // [HACK start] Proccess only one return in case of dual mode for performance issue
  int start_block = 0;
  int end_block = ASENSING_BLOCK_NUM;
  if (returnMode == 0x39 || returnMode == 0x3B || returnMode == 0x3C)
  {
    end_block = 1;
  }
  // [HACK end]

  for (int blockID = start_block; blockID < end_block; blockID++)
  {
    AsensingBlock currentBlock = dataPacket->blocks[blockID];

    AsensingSpecificFrameInformation* frameInfo =
      reinterpret_cast<AsensingSpecificFrameInformation*>(
        this->ParserMetaData.SpecificInformation.get());
    if (frameInfo->IsNewFrame(1, current_frame_id))
    {
#ifdef PACKET_STAT_DEBUG
      if (current_frame_id > 0 && this->seq_num_counter < (this->points_per_frame / TEST_POINT_PER_PACKET)) {

          vtkWarningMacro(<< "Incomplete frame (id: " << (current_frame_id - 1)
                          << ", packets: " << seq_num_counter
                          << ", total: " << (this->points_per_frame / TEST_POINT_PER_PACKET)
                          << ", lsn: " << this->last_seq_num 
                          << ", points: " << this->points_per_frame << ")" );
      }
#endif
      std::cout << "Split Frame =>> " << "FrameID: " << this->current_frame_id << ", total: " << this->points_per_frame  << std::endl; 
      this->SplitFrame();
      this->seq_num_counter = 0;
    }

    for (int laserID = 0; laserID < ASENSING_LASER_NUM; laserID++)
    {
      double x, y, z;

      double distance =
        static_cast<double>(currentBlock.units[laserID].GetDistance()) * ASENSING_DISTANCE_UNIT;

      if (this->CalibEnabled)
      {
        x = distance * this->XCorrection[current_pt_id];
        y = distance * this->YCorrection[current_pt_id];
        z = distance * this->ZCorrection[current_pt_id];
      }
      else
      {
        // double azimuth_correction = this->AzimuthCorrection[laserID];
        // double elevation_correction = this->ElevationCorrection[laserID];

        float azimuth = static_cast<float>(currentBlock.units[laserID].GetAzimuth()) * ASENSING_AZIMUTH_UNIT;
        float pitch = static_cast<float>(currentBlock.units[laserID].GetElevation()) * ASENSING_ELEVATION_UNIT;

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
      } /* End of this->CalibEnabled */

#if USING_RT_MATRIX
      /* Matrix processing */
      if (this->RTMatEnabled)
      {
        double x_ = x, y_ = y, z_ = z;

        if (laserID == 0 || laserID == 1)
        {
          x = this->matrix_RT0[0][0] * x_ + this->matrix_RT0[0][1] * y_ +
            this->matrix_RT0[0][2] * z_ + this->matrix_RT0[0][3];
          y = this->matrix_RT0[1][0] * x_ + this->matrix_RT0[1][1] * y_ +
            this->matrix_RT0[1][2] * z_ + this->matrix_RT0[1][3];
          z = this->matrix_RT0[2][0] * x_ + this->matrix_RT0[2][1] * y_ +
            this->matrix_RT0[2][2] * z_ + this->matrix_RT0[2][3];
        }
        else if (laserID == 2 || laserID == 3)
        {
          x = this->matrix_RT1[0][0] * x_ + this->matrix_RT1[0][1] * y_ +
            this->matrix_RT1[0][2] * z_ + this->matrix_RT1[0][3];
          y = this->matrix_RT1[1][0] * x_ + this->matrix_RT1[1][1] * y_ +
            this->matrix_RT1[1][2] * z_ + this->matrix_RT1[1][3];
          z = this->matrix_RT1[2][0] * x_ + this->matrix_RT1[2][1] * y_ +
            this->matrix_RT1[2][2] * z_ + this->matrix_RT1[2][3];
        }
        else if (laserID == 4 || laserID == 5)
        {
          x = this->matrix_RT2[0][0] * x_ + this->matrix_RT2[0][1] * y_ +
            this->matrix_RT2[0][2] * z_ + this->matrix_RT2[0][3];
          y = this->matrix_RT2[1][0] * x_ + this->matrix_RT2[1][1] * y_ +
            this->matrix_RT2[1][2] * z_ + this->matrix_RT2[1][3];
          z = this->matrix_RT2[2][0] * x_ + this->matrix_RT2[2][1] * y_ +
            this->matrix_RT2[2][2] * z_ + this->matrix_RT2[2][3];
        }
        else if (laserID == 6 || laserID == 7)
        {
          x = this->matrix_RT3[0][0] * x_ + this->matrix_RT3[0][1] * y_ +
            this->matrix_RT3[0][2] * z_ + this->matrix_RT3[0][3];
          y = this->matrix_RT3[1][0] * x_ + this->matrix_RT3[1][1] * y_ +
            this->matrix_RT3[1][2] * z_ + this->matrix_RT3[1][3];
          z = this->matrix_RT3[2][0] * x_ + this->matrix_RT3[2][1] * y_ +
            this->matrix_RT3[2][2] * z_ + this->matrix_RT3[2][3];
        }
      }
#endif /* USING_RT_MATRIX */

      uint8_t intensity = currentBlock.units[laserID].GetIntensity();

      int offset = currentBlock.GettimeOffSet();

      // Compute timestamp of the point
      timestamp += offset;

#if DEBUG
       //std::cout << "Point " << current_frame_id << ": " << distance << ", " << azimuth << ", " << pitch << ", " << x << ", " << y << ", " << z << std::endl;
       std::cout << "Point " << current_frame_id << ": " << currentBlock.units[laserID].GetDistance() 
                 << ", " << currentBlock.units[laserID].GetAzimuth() 
                 << ", " << currentBlock.units[laserID].GetElevation() << ", " << x << ", " << y << ", " << z << std::endl;
#endif

      if (current_pt_id >= this->points_per_frame)
      {
#ifdef PACKET_STAT_DEBUG
        // SplitFrame for safety to not overflow allcoated arrays
        vtkWarningMacro(<< "Received more datapoints than expected" << " (" << current_pt_id << ", " << current_frame_id << ")");

        if (current_frame_id > 0 && this->seq_num_counter < (this->points_per_frame / TEST_POINT_PER_PACKET)) {

          vtkWarningMacro(<< "Incomplete frame 2 (id: " << (current_frame_id - 1)
                          << ", packets: " << seq_num_counter
                          << ", total: " << (this->points_per_frame / TEST_POINT_PER_PACKET)
                          << ", lsn: " << this->last_seq_num 
                          << ", points: " << this->points_per_frame << ")" );
        }
#endif
        this->SplitFrame();
        this->seq_num_counter = 0;
      }

      this->Points->SetPoint(current_pt_id, x, y, z);

      TrySetValue(this->PointsX, current_pt_id, x);
      TrySetValue(this->PointsY, current_pt_id, y);
      TrySetValue(this->PointsZ, current_pt_id, z);
      TrySetValue(this->PointID, current_pt_id, current_pt_id);
      TrySetValue(this->LaserID, current_pt_id, laserID);
      TrySetValue(this->Intensities, current_pt_id, intensity);
      TrySetValue(this->Timestamps, current_pt_id, timestamp);
      TrySetValue(this->Distances, current_pt_id, distance);
      current_pt_id++;
    }
  }

  this->last_seq_num = current_seq_num;

  auto stop = high_resolution_clock::now();
  duration<double, std::micro> ms_double = stop - start;
  // std::cout << ms_double.count() << "micro seconds\n";
}

//-----------------------------------------------------------------------------
bool vtkAsensingPacketInterpreter::IsLidarPacket(
  unsigned char const* data, unsigned int dataLength)
{
#if CHECK_LIDAR_PACKET
  const AsensingPacket* dataPacket = reinterpret_cast<const AsensingPacket*>(data);

  if (dataLength < 4) {
    return false;
  }

  /* Check sob flag of packet header */
  uint32_t sob = htole32(0x5AA555AA); /* 0xAA, 0x55, 0xA5, 0x5A */
  if (sob != dataPacket->header.GetSob()) {
    vtkWarningMacro("Not a vaild point-cloud data packet");
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
vtkSmartPointer<vtkPolyData> vtkAsensingPacketInterpreter::CreateNewEmptyFrame(
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

  this->PointID = CreateDataArray<vtkUnsignedIntArray>(
    false, "PointID", numberOfPoints, defaultPrereservedNumberOfPointsPerFrame, polyData);
  this->LaserID = CreateDataArray<vtkUnsignedIntArray>(
    false, "LaserID", numberOfPoints, defaultPrereservedNumberOfPointsPerFrame, polyData);
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
bool vtkAsensingPacketInterpreter::PreProcessPacket(unsigned char const* data,
  unsigned int vtkNotUsed(dataLength), fpos_t filePosition, double packetNetworkTime,
  std::vector<FrameInformation>* frameCatalog)
{
  bool isNewFrame = false;

  this->ParserMetaData.FilePosition = filePosition;
  this->ParserMetaData.FirstPacketDataTime = 0.0; // TODO
  this->ParserMetaData.FirstPacketNetworkTime = packetNetworkTime;

  const AsensingPacket* dataPacket = reinterpret_cast<const AsensingPacket*>(data);

  for (int blockID = 0; blockID < ASENSING_BLOCK_NUM; blockID++)
  {
    AsensingBlock currentBlock = dataPacket->blocks[blockID];

    AsensingSpecificFrameInformation* frameInfo =
      reinterpret_cast<AsensingSpecificFrameInformation*>(
        this->ParserMetaData.SpecificInformation.get());
    if (frameInfo->IsNewFrame(0, dataPacket->header.GetFrameID()) && frameCatalog)
    {
      isNewFrame = true;
      frameCatalog->push_back(this->ParserMetaData);
    }
  }

  return isNewFrame;
}

//-----------------------------------------------------------------------------
std::string vtkAsensingPacketInterpreter::GetSensorInformation(bool vtkNotUsed(shortVersion))
{
  return "Asensing LiDAR Sensor";
}
