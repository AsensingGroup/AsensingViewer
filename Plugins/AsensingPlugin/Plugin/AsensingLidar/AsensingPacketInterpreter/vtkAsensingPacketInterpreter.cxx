#include "vtkAsensingPacketInterpreter.h"
#include "AsensingPacketFormat.h"
//#include "private_format.h"
#include "vtkHelper.h"

#include <vtkPoints.h>
#include <vtkPointData.h>
#include <vtkDoubleArray.h>
#include <vtkTransform.h>

#include <bitset>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/foreach.hpp>

#include <math.h>
#include <fenv.h>

#include <vtkDelimitedTextReader.h>
#include <chrono>

#define PANDAR128_LASER_NUM (128)

using std::chrono::high_resolution_clock;
using std::chrono::duration_cast;
using std::chrono::duration;
using std::chrono::milliseconds;

namespace  {

double degreeToRadian(double degree) { return degree * vtkMath::Pi() / 180.0; }
}

//! @todo this method are actually usefull for every Interpreter and should go to the top
template<typename T>
vtkSmartPointer<T> vtkAsensingPacketInterpreter::CreateDataArray(bool isAdvanced,
                                                              const char* name,
                                                              vtkIdType vtkNotUsed(np),
                                                              vtkIdType prereserved_np,
                                                              vtkPolyData* pd)
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
    array->SetValue(pos,   value);
  }
}

//-----------------------------------------------------------------------------
vtkStandardNewMacro(vtkAsensingPacketInterpreter)

//-----------------------------------------------------------------------------
vtkAsensingPacketInterpreter::vtkAsensingPacketInterpreter()
{
  std::cout << "Size of AsensingPacket = " << sizeof(AsensingPacket) << std::endl;
  
  // Initialize Elevation and Azimuth correction
  for(int i = 0; i < PANDAR128_LASER_NUM; i++)
  {
    if(elev_angle[i])
    {
      this->ElevationCorrection.push_back(elev_angle[i]);
    }
    if(azimuth_offset[i])
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
vtkAsensingPacketInterpreter::~vtkAsensingPacketInterpreter()
{
}

#include "cJSON.h"
//-----------------------------------------------------------------------------
void vtkAsensingPacketInterpreter::LoadCalibration(const std::string& filename)
{
  if (filename.empty())
  {
    this->IsCalibrated = false;
    return;
  }

  std::cout << "Load Calibration File: " << filename << std::endl;

  std::string keyword("No-Correction");
  std::size_t found = filename.find(keyword);
  if (std::string::npos != found) {
    std::cout << "Do not need to correct data" << std::endl;
    this->CalibEnabled = false;
    this->IsCalibrated = true;
    return;
  }

  /* Correct point cloud */
  std::cout << "Version " << CJSON_VERSION_MAJOR << "." << CJSON_VERSION_MINOR << "." << CJSON_VERSION_PATCH << std::endl;

  FILE *fp = fopen(filename.c_str(), "rb");
  if (fp == NULL) {
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
  
  while (fread(jsonData, filesize, 1, fp) == 1) {}
  
  cJSON *root = cJSON_Parse(jsonData);
  //printf("%s\n\n", cJSON_Print(root));

  cJSON *module0 = cJSON_GetObjectItemCaseSensitive(root, "module_0");
  cJSON *module1 = cJSON_GetObjectItemCaseSensitive(root, "module_1");
  cJSON *module2 = cJSON_GetObjectItemCaseSensitive(root, "module_2");
  cJSON *module3 = cJSON_GetObjectItemCaseSensitive(root, "module_3");

  if (!(cJSON_IsArray(module0) && cJSON_IsArray(module1) && cJSON_IsArray(module2) && cJSON_IsArray(module3))) {
    std::cout << "Warning: Please check module data" << std::endl;
  }

  long arraySize = cJSON_GetArraySize(module0); // 9600
  printf("Array Size = %ld\n", arraySize);

  this->XCorrection.clear();
  this->XCorrection.resize(arraySize * 4);  // 38400
  this->YCorrection.clear();
  this->YCorrection.resize(arraySize * 4);
  this->ZCorrection.clear();
  this->ZCorrection.resize(arraySize * 4);

  // Channel 0
  struct cJSON *element0;
  struct cJSON *element1;
  struct cJSON *element2;
  struct cJSON *element3;
  element0 = module0->child;
  element1 = module1->child;
  element2 = module2->child;
  element3 = module3->child;

  bool isValid = true;
  int channelFlag = 0;
  int counter = 0;

  vtkIdType pointIndex = 0;

  for (vtkIdType index = 0; index < arraySize; ++index)
  {
    if (index != 0 && index % 400 == 0) {
      isValid = isValid ? false : true;
    }

    if (!isValid) {
      element0 = element0->next;
      element1 = element1->next;
      element2 = element2->next;
      element3 = element3->next;
      //if (element0 == NULL) break;
      continue;
    }

    if (index != 0 && index % 200 == 0) {
      channelFlag = channelFlag ? 0 : 1;
    }

    if (1) {
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
    }

    element0 = element0->next;
    element1 = element1->next;
    element2 = element2->next;
    element3 = element3->next;
    if (element0 == NULL || element1 == NULL || element2 == NULL || element3 == NULL) {
      std::cout << "Error: Points do not match!" << std::endl;
      break;
    }

    // std::cout << pointIndex << " ";
    // if (pointIndex % 10 == 0) {
    //   std::cout << std::endl;
    // }

    pointIndex++;
  }

  cJSON_Delete(root);
  free(jsonData);
  fclose(fp);

  std::cout << "Close " << filename << std::endl;

  std::cout << "[0] x' = " << XCorrection[0] << ", y' = " << YCorrection[0] << ", z' = " << ZCorrection[0] << std::endl;

  // Load the JSON file

#if 0
  // Load the CSV file.
  // 2nd Column = Azimuth (Horizontal) Correction
  // 3rd Column = Elevation (Vertical) Correction

  vtkNew<vtkDelimitedTextReader> reader;
  reader->SetFileName(filename.c_str());
  reader->DetectNumericColumnsOn();
  reader->SetHaveHeaders(true);
  reader->SetFieldDelimiterCharacters(",");
  reader->SetStringDelimiter('"');
  reader->Update();

  // Extract the table.
  vtkTable * csvTable = reader->GetOutput();
  vtkIdType nRows = csvTable->GetNumberOfRows();

  this->CalibrationData->ShallowCopy(csvTable);

  this->ElevationCorrection.clear();
  this->ElevationCorrection.resize(nRows);
  this->AzimuthCorrection.clear();
  this->AzimuthCorrection.resize(nRows);

  for (vtkIdType indexRow = 0; indexRow < nRows; ++indexRow)
  {
    double elevation = this->CalibrationData->GetValue(indexRow, 1).ToDouble();
    double azimuth = this->CalibrationData->GetValue(indexRow, 2).ToDouble();

    this->ElevationCorrection[indexRow] = elevation;
    this->AzimuthCorrection[indexRow] = azimuth;

  }
#endif
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
  time_t  unix_second = (mktime(&t));

  int mode = 0;
  int state = 0;
  int returnMode = 0;
  //int mode =  dataPacket->tail.GetShutdownFlag() & 0x03;
  //int state = (dataPacket->tail.GetShutdownFlag() & 0xF0) >> 4;
  //int returnMode = dataPacket->tail.GetReturnMode();

  // Timestamp contains in the packet
  // roll back every second, probably in microsecond
  uint32_t timestampPacket = dataPacket->header.GetTimestamp();

  // Timestamp in second of the packet
  double timestamp = unix_second + (timestampPacket / 1000000.0);

  current_frame_id = dataPacket->header.GetFrameID();

  // [HACK start] Proccess only one return in case of dual mode for performance issue
  int start_block = 0;
  int end_block = ASENSING_BLOCK_NUM;
  if (returnMode == 0x39 ||
      returnMode == 0x3B  ||
      returnMode == 0x3C)
  {
    end_block = 1;
  }
  // [HACK end]


  for (int blockID = start_block; blockID < end_block; blockID++)
  {
     AsensingBlock currentBlock = dataPacket->blocks[blockID];

     AsensingSpecificFrameInformation* frameInfo =
         reinterpret_cast<AsensingSpecificFrameInformation*>(this->ParserMetaData.SpecificInformation.get());
     if(frameInfo->IsNewFrame(1, current_frame_id))
     {
       std::cout << "Split Frame =>> FrameID: " << current_frame_id << std::endl;
       this->SplitFrame();
     }

     for (int laserID = 0; laserID < ASENSING_LASER_NUM; laserID++)
     {
       double x, y, z;

       double distance = static_cast<double>(currentBlock.units[laserID].GetDistance()) * ASENSING_DISTANCE_UNIT;

       if (this->CalibEnabled) {
         x = distance * this->XCorrection[current_pt_id];
         y = distance * this->YCorrection[current_pt_id];
         z = distance * this->ZCorrection[current_pt_id];
       }
       else {
       //double azimuth_correction = this->AzimuthCorrection[laserID];
       //double elevation_correction = this->ElevationCorrection[laserID];


       //float azimuth = azimuth_correction + (static_cast<float>(currentBlock.GetAzimuth()) / 100.0f);
       //float originAzimuth = azimuth;
       float azimuth = static_cast<float>(currentBlock.units[laserID].GetAzimuth()) * ASENSING_AZIMUTH_UNIT;

       //float pitch = elevation_correction;
       float pitch = static_cast<float>(currentBlock.units[laserID].GetElevation()) * ASENSING_ELEVATION_UNIT;

       //int offset = this->LaserOffset.getTSOffset(laserID, mode, state, distance, dataPacket->header.GetVersionMajor());
       //offset = currentBlock.GettimeOffSet();

       //azimuth += this->LaserOffset.getAngleOffset(offset, 1 /* dataPacket->tail.GetMotorSpeed() */, dataPacket->header.GetVersionMajor());

       //pitch += this->LaserOffset.getPitchOffset("", pitch, distance);

       if (pitch < 0)
       {
         pitch += 360.0f;
       }
       else if (pitch >= 360.0f)
       {
         pitch -= 360.0f;
       }

       float xyDistance = distance * this->Cos_all_angle[static_cast<int>(pitch * 100 + 0.5)];
       //azimuth += this->LaserOffset.getAzimuthOffset("", originAzimuth, azimuth, xyDistance);

       int azimuthIdx = static_cast<int>(azimuth * 100 + 0.5);
       if (azimuthIdx >= CIRCLE)
       {
         azimuthIdx -= CIRCLE;
       }
       else if (azimuthIdx < 0)
       {
         azimuthIdx += CIRCLE;
       }

#if 0
       double x = xyDistance * sin(degreeToRadian(azimuth)); // this->Sin_all_angle[azimuthIdx];
       double y = xyDistance * cos(degreeToRadian(azimuth)); // this->Cos_all_angle[azimuthIdx];
       double z = distance * sin(degreeToRadian(pitch));     // this->Sin_all_angle[static_cast<int>(pitch * 100 + 0.5)];
#else
       x = xyDistance * this->Cos_all_angle[azimuthIdx];
       y = xyDistance * this->Sin_all_angle[azimuthIdx];
       z = distance * this->Sin_all_angle[static_cast<int>(pitch * 100 + 0.5)];
#endif
       }

       uint8_t intensity = currentBlock.units[laserID].GetIntensity();

       int offset = currentBlock.GettimeOffSet();

       // Compute timestamp of the point
       //timestamp += this->LaserOffset.getBlockTS(blockID, returnMode, mode, PANDAR128_LIDAR_NUM) / 1000000000.0 + offset / 1000000000.0;
       timestamp += offset;

#if 0
       //std::cout << "Point " << current_frame_id << ": " << distance << ", " << azimuth << ", " << pitch << ", " << x << ", " << y << ", " << z << std::endl;
       std::cout << "Point " << current_frame_id << ": " << currentBlock.units[laserID].GetDistance() 
                 << ", " << currentBlock.units[laserID].GetAzimuth() 
                 << ", " << currentBlock.units[laserID].GetElevation() << ", " << x << ", " << y << ", " << z << std::endl;
#endif

#if 1
       if (current_pt_id >= ASENSING_POINT_NUM)
       {
          // SplitFrame for safety to not overflow allcoated arrays
          //vtkWarningMacro("Received more datapoints than expected");
          //std::cout << "Split Frame =>> FrameID: " << current_frame_id << std::endl;
          this->SplitFrame();
       }
#endif
       this->Points->SetPoint(current_pt_id, x, y, z);

       TrySetValue(this->PointsX, current_pt_id, x);
       TrySetValue(this->PointsY, current_pt_id, y);
       TrySetValue(this->PointsZ, current_pt_id, z);

       TrySetValue(this->LaserID, current_pt_id, laserID);
       TrySetValue(this->Intensities, current_pt_id, intensity);
       TrySetValue(this->Timestamps, current_pt_id, timestamp);
       TrySetValue(this->Distances, current_pt_id, distance);
       current_pt_id++;

       /*int index;
       if (LIDAR_RETURN_BLOCK_SIZE_2 == m_iReturnBlockSize)
       {
         index = (block.fAzimuth - start_angle_) / m_iAngleSize * PANDAR128_LASER_NUM *
                     m_iReturnBlockSize +
                 PANDAR128_LASER_NUM * blockid + i;
         // ROS_WARN("block 2 index:[%d]",index);
       } else {
         index = (block.fAzimuth - start_angle_) / m_iAngleSize * PANDAR128_LASER_NUM + i;
       }
       cld->points[index] = point;*/
     }
  }

  auto stop = high_resolution_clock::now();
  duration<double, std::micro> ms_double = stop - start;
  //std::cout << ms_double.count() << "micro seconds\n";
}

//-----------------------------------------------------------------------------
bool vtkAsensingPacketInterpreter::IsLidarPacket(unsigned char const * vtkNotUsed(data),
                                              unsigned int vtkNotUsed(dataLength))
{
  //std::cout << "Process Packet, Size = " << dataLength << std::endl;
  //std::cout << "dataLength  " << dataLength << std::endl;
  //std::cout << "ASENSING_PACKET_SIZE  " << ASENSING_PACKET_SIZE << std::endl;
  //std::cout << "sizeof(AsensingPacket)  " << sizeof(AsensingPacket) << std::endl;

  return true; //dataLength == PACKET_SIZE;
}



//-----------------------------------------------------------------------------
vtkSmartPointer<vtkPolyData> vtkAsensingPacketInterpreter::CreateNewEmptyFrame(vtkIdType numberOfPoints,
                                                                            vtkIdType vtkNotUsed(prereservedNumberOfPoints))
{
  const int defaultPrereservedNumberOfPointsPerFrame = ASENSING_POINT_NUM;
  vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();

  // points
  current_pt_id = 0;
  vtkNew<vtkPoints> points;
  points->SetDataTypeToFloat();
  points->SetNumberOfPoints(defaultPrereservedNumberOfPointsPerFrame);
  points->GetData()->SetName("Points_m_XYZ");
  polyData->SetPoints(points.GetPointer());

  // intensity
  this->Points = points.GetPointer();
  this->PointsX = CreateDataArray<vtkDoubleArray>(true, "X", numberOfPoints,
                                                  defaultPrereservedNumberOfPointsPerFrame, polyData);
  this->PointsY = CreateDataArray<vtkDoubleArray>(true, "Y", numberOfPoints,
                                                  defaultPrereservedNumberOfPointsPerFrame, polyData);
  this->PointsZ = CreateDataArray<vtkDoubleArray>(true, "Z", numberOfPoints,
                                                  defaultPrereservedNumberOfPointsPerFrame, polyData);

  this->LaserID = CreateDataArray<vtkUnsignedIntArray>(false, "LaserID", numberOfPoints,
                                                       defaultPrereservedNumberOfPointsPerFrame, polyData);
  this->Intensities = CreateDataArray<vtkUnsignedCharArray>(false, "Intensity", numberOfPoints,
                                                      defaultPrereservedNumberOfPointsPerFrame, polyData);
  this->Timestamps = CreateDataArray<vtkDoubleArray>(false, "Timestamp", numberOfPoints,
                                                     defaultPrereservedNumberOfPointsPerFrame, polyData);
  this->Distances = CreateDataArray<vtkDoubleArray>(false, "Distance", numberOfPoints,
                                                    defaultPrereservedNumberOfPointsPerFrame, polyData);
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


  for(int blockID = 0; blockID < ASENSING_BLOCK_NUM ; blockID++)
  {
    AsensingBlock currentBlock = dataPacket->blocks[blockID];

    AsensingSpecificFrameInformation* frameInfo =
        reinterpret_cast<AsensingSpecificFrameInformation*>(this->ParserMetaData.SpecificInformation.get());
    if(frameInfo->IsNewFrame(0, dataPacket->header.GetFrameID()) && frameCatalog)
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

