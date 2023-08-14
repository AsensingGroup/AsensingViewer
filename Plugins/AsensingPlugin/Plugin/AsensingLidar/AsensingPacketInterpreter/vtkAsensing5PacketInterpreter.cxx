#include "vtkAsensing5PacketInterpreter.h"

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
vtkSmartPointer<T> vtkAsensing5PacketInterpreter::CreateDataArray(bool isAdvanced, const char* name,
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
vtkStandardNewMacro(vtkAsensing5PacketInterpreter)

//-----------------------------------------------------------------------------
vtkAsensing5PacketInterpreter::vtkAsensing5PacketInterpreter()
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
vtkAsensing5PacketInterpreter::~vtkAsensing5PacketInterpreter() {}

#include "cJSON.h"
//-----------------------------------------------------------------------------
void vtkAsensing5PacketInterpreter::LoadCalibration(const std::string& filename)
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

  std::string keyword("A0-Correction-5");
  std::size_t found = filename.find(keyword);
  if (std::string::npos == found)
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

  cJSON* module_angles = cJSON_GetObjectItemCaseSensitive(root, "module_angles");
  for (int i = 0; i < ANGLE_SIZE; i++)
  {
    m_angles[i] = cJSON_GetArrayItem(module_angles, i)->valuedouble;
    std::cout << "angle " << i << " : " << m_angles[i] << std::endl;
  }

  cJSON* channels = cJSON_GetObjectItem(root, "module");
  if (cJSON_IsArray(channels))
  {
    printf("channels :\n%s\n\n", cJSON_Print(channels));

    int channelSize = cJSON_GetArraySize(channels);
    if (ASENSING_LASER_NUM == channelSize) {
      for (int i = 0; i < ASENSING_LASER_NUM; i++)
      {
        this->channels[i] = cJSON_GetArrayItem(channels, i)->valueint;
      }
    }
    else {
      vtkWarningMacro("[A0] Invalid calibration data");
    }
  }

  bool NoLaserAngle = false;
  bool NoRTMatrix = false;

  if (!(cJSON_IsArray(module0) && cJSON_IsArray(module1) && cJSON_IsArray(module2) &&
        cJSON_IsArray(module3)))
  {
    std::cout << "Warning: Please check module data" << std::endl;
    NoLaserAngle = true;
  }

#if USING_RT_MATRIX
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

    this->RTMatEnabled = cJSON_GetObjectItem(root, "RT_enable")->valueint ? true : false;

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
#endif

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

void vtkAsensing5PacketInterpreter::ProcessPacket(unsigned char const* data, unsigned int dataLength)
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

  laser_num = dataPacket->header.GetLaserNum(); // ASENSING_LASER_NUM
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

  AsensingSpecificFrameInformation* frameInfo = reinterpret_cast<AsensingSpecificFrameInformation*>(this->ParserMetaData.SpecificInformation.get());
  if (frameInfo->IsNewFrame(1, current_frame_id))
  {
#if PACKET_STAT_DEBUG
    /* If fewer UDP packets are received than expected, means packet loss */
    if (current_frame_id > 0 && this->seq_num_counter < (this->points_per_frame / ASENSING_POINT_PER_PACKET)) {
        vtkWarningMacro(<< "Incomplete frame (id: " << (current_frame_id - 1)
                        << ", packets: " << seq_num_counter
                        << ", total: " << (this->points_per_frame / ASENSING_POINT_PER_PACKET)
                        << ", lsn: " << this->last_seq_num 
                        << ", points: " << this->points_per_frame << ")" );
    }
#endif
    //std::cout << "Split Frame =>> " << "FrameID: " << this->current_frame_id << ", total: " << this->points_per_frame  << std::endl; 
    this->SplitFrame();
  }

  if (current_seq_num != 0 && current_seq_num != last_seq_num+1) { 
    structured_pt_id += ASENSING_POINT_PER_PACKET;
  }

  for (int blockID = start_block; blockID < end_block; blockID++)
  {
    AsensingBlock currentBlock = dataPacket->blocks[blockID];

    for (int laserID = 0; laserID < ASENSING_LASER_NUM; laserID++)
    {
      if(this->channels[laserID] == 1) {
          /* Eliminate invalid points */
          if (0 == currentBlock.units[laserID].GetAzimuth() &&
              0 == currentBlock.units[laserID].GetElevation() &&
              0 == currentBlock.units[laserID].GetDistance() &&
              0 == currentBlock.units[laserID].GetIntensity()) {
                continue;
          }

          double x, y, z, azimuth, pitch;

          double distance = static_cast<double>(currentBlock.units[laserID].GetDistance()) * ASENSING_DISTANCE_UNIT;

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

            azimuth = static_cast<float>(currentBlock.units[laserID].GetAzimuth()) * ASENSING_AZIMUTH_UNIT;
            pitch = static_cast<float>(currentBlock.units[laserID].GetElevation()) * ASENSING_ELEVATION_UNIT;

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
    #if PACKET_STAT_DEBUG
            // SplitFrame for safety to not overflow allcoated arrays
            vtkWarningMacro(<< "Received more datapoints than expected" << " (" << current_pt_id << ", " << current_frame_id << ")");

            if (current_frame_id > 0 && this->seq_num_counter < (this->points_per_frame / ASENSING_POINT_PER_PACKET)) {

              vtkWarningMacro(<< "Incomplete frame 2 (id: " << (current_frame_id - 1)
                              << ", packets: " << seq_num_counter
                              << ", total: " << (this->points_per_frame / ASENSING_POINT_PER_PACKET)
                              << ", lsn: " << this->last_seq_num
                              << ", points: " << this->points_per_frame << ")" );
            }
    #endif
            this->SplitFrame();
          }

          // 角度算法处理x，y，z
          auto type = dataPacket->header.GetLidarInfo() >> 6;
          if(type == 0x02 || type == 0x03)  // 0x02 -> 0°, 0x03 -> 25°
          {
              // 入射向量求解
              float vector[VECTOR_SIZE] = {0};
              float theta = degreeToRadian(m_angles[laserID / 2]);
              float gamma0 = degreeToRadian(m_angles[ANGLE_SIZE-1]);
              float sin_gamma0 = std::sin(gamma0);
              float cos_gamma0 = std::cos(gamma0);
              float cos_theta = std::cos(theta);
              vector[0] = cos_theta - 2.0 * cos_theta * cos_gamma0 * cos_gamma0;
              vector[1] = std::sin(theta);
              vector[2] =  2.0 * cos_theta * sin_gamma0 * cos_gamma0;

              // 法向量求解
              float normal[VECTOR_SIZE] = {0};
              float angle = currentBlock.units[laserID].GetAzimuth() * ASENSING_AZIMUTH_UNIT;
              angle = (angle > 120) ? (angle - 360) : angle;
              if(type == 0x03)
              {
                  if(laserID / 2 == 0)
                  {
                      angle += 50;
                  }
                  else if(laserID / 2 == 1)
                  {
                      angle += 25;
                  }
                  else if(laserID / 2 == 3)
                  {
                      angle -= 25;
                  }
                  else if(laserID / 2 == 4)
                  {
                      angle -= 50;
                  }
              }
              float gamma = degreeToRadian(-angle);
              angle = static_cast<float>(currentBlock.units[laserID].GetElevation()) * ASENSING_ELEVATION_UNIT;
              angle = (angle > 120) ? (angle - 360) : angle;
              float beta = - 1 * degreeToRadian(angle);
              float sin_gamma = std::sin(gamma);
              float cos_gamma = std::cos(gamma);
              float sin_beta = std::sin(beta);
              float cos_beta = std::cos(beta);
              normal[0] = cos_beta * cos_gamma * cos_gamma0 - sin_beta * sin_gamma0;
              normal[1] = sin_gamma * cos_gamma0;
              normal[2] = -cos_gamma0 * sin_beta * cos_gamma - cos_beta * sin_gamma0;

              // 最终向量求解
              float out[VECTOR_SIZE] = {0};
              float k = vector[0] * normal[0] + vector[1] * normal[1] + vector[2] * normal[2];
              for(int i = 0; i < VECTOR_SIZE; i++)
              {
                  out[i] = vector[i] - 2 * k * normal[i];
              }
              x = distance * out[0];
              y = distance * out[1];
              z = distance * out[2];
          }

          this->Points->SetPoint(current_pt_id, x, y, z);

          if (azimuth < 0)
          {
              azimuth += 360.0f;
          }
          else if (azimuth >= 270.0f)
          {
              azimuth -= 360.0f;
          }

          if (pitch < 0)
          {
              pitch += 360.0f;
          }
          else if (pitch >= 270.0f)
          {
              pitch -= 360.0f;
          }

          TrySetValue(this->PointsX, current_pt_id, x);
          TrySetValue(this->PointsY, current_pt_id, y);
          TrySetValue(this->PointsZ, current_pt_id, z);
          TrySetValue(this->Azimuth, current_pt_id, azimuth);
          TrySetValue(this->Elevation, current_pt_id, pitch);
          TrySetValue(this->PointID, current_pt_id, structured_pt_id);
          TrySetValue(this->LaserID, current_pt_id, laserID);
          TrySetValue(this->Intensities, current_pt_id, intensity);
          TrySetValue(this->Timestamps, current_pt_id, timestamp);
          TrySetValue(this->Distances, current_pt_id, distance);
          current_pt_id++;
          structured_pt_id++;
      }
      else {

          double x, y, z, azimuth, pitch;

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

            azimuth = static_cast<float>(currentBlock.units[laserID].GetAzimuth()) * ASENSING_AZIMUTH_UNIT;
            pitch = static_cast<float>(currentBlock.units[laserID].GetElevation()) * ASENSING_ELEVATION_UNIT;

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


          uint8_t intensity = currentBlock.units[laserID].GetIntensity();

          int offset = currentBlock.GettimeOffSet();

          // Compute timestamp of the point
          timestamp += offset;


          if (current_pt_id >= this->points_per_frame)
          {
    #if PACKET_STAT_DEBUG
            // SplitFrame for safety to not overflow allcoated arrays
            vtkWarningMacro(<< "Received more datapoints than expected" << " (" << current_pt_id << ", " << current_frame_id << ")");

            if (current_frame_id > 0 && this->seq_num_counter < (this->points_per_frame / ASENSING_POINT_PER_PACKET)) {

              vtkWarningMacro(<< "Incomplete frame 2 (id: " << (current_frame_id - 1)
                              << ", packets: " << seq_num_counter
                              << ", total: " << (this->points_per_frame / ASENSING_POINT_PER_PACKET)
                              << ", lsn: " << this->last_seq_num
                              << ", points: " << this->points_per_frame << ")" );
            }
    #endif
            this->SplitFrame();
          }

          this->Points->SetPoint(current_pt_id, x, y, z);

          TrySetValue(this->PointsX, current_pt_id, NAN);
          TrySetValue(this->PointsY, current_pt_id, NAN);
          TrySetValue(this->PointsZ, current_pt_id, NAN);
          TrySetValue(this->Azimuth, current_pt_id, NAN);
          TrySetValue(this->Elevation, current_pt_id, NAN);
          TrySetValue(this->PointID, current_pt_id, structured_pt_id);
          TrySetValue(this->LaserID, current_pt_id, laserID);
          TrySetValue(this->Intensities, current_pt_id, NAN);
          TrySetValue(this->Timestamps, current_pt_id, NAN);
          TrySetValue(this->Distances, current_pt_id, NAN);
          current_pt_id++;
          structured_pt_id++;
      }
    }
  }

  this->last_seq_num = current_seq_num;

  auto stop = high_resolution_clock::now();
  duration<double, std::micro> ms_double = stop - start;
  // std::cout << ms_double.count() << "micro seconds\n";
}

//-----------------------------------------------------------------------------
bool vtkAsensing5PacketInterpreter::IsLidarPacket(
  unsigned char const* data, unsigned int dataLength)
{
#if CHECK_LIDAR_PACKET
//  const AsensingPacket* dataPacket = reinterpret_cast<const AsensingPacket*>(data);
  if (dataLength != sizeof(struct AsensingPacket)) {
    vtkWarningMacro("Invaild point cloud data packet (length mismatch)");
    return false;
  }

  /* Check sob flag of packet header */
  if (data[0] == 0xAA && data[1] == 0x55 && data[2] == 0xA5 && data[3] == 0x5A )
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
vtkSmartPointer<vtkPolyData> vtkAsensing5PacketInterpreter::CreateNewEmptyFrame(
  vtkIdType numberOfPoints, vtkIdType vtkNotUsed(prereservedNumberOfPoints))
{
  const int defaultPrereservedNumberOfPointsPerFrame = this->points_per_frame;
  vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();

  // points
  structured_pt_id = 0;
  current_pt_id = 0;
  current_seq_num = 0;
  seq_num_counter = 0;
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
  this->LaserID = CreateDataArray<vtkUnsignedCharArray>(
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
bool vtkAsensing5PacketInterpreter::PreProcessPacket(unsigned char const* data,
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
    AsensingSpecificFrameInformation* frameInfo =  reinterpret_cast<AsensingSpecificFrameInformation*>(this->ParserMetaData.SpecificInformation.get());
	auto frameID = dataPacket->header.GetFrameID();
	isNewFrame = frameInfo->IsNewFrame(0, frameID);
    if (isNewFrame)
    {
		if (frameCatalog) {
			frameCatalog->push_back(this->ParserMetaData);
		}
		else {
			return true;
		}
    }
  }
  return isNewFrame;
}

//-----------------------------------------------------------------------------
std::string vtkAsensing5PacketInterpreter::GetSensorInformation(bool vtkNotUsed(shortVersion))
{
  return "Asensing A0 LiDAR Sensor (5 modules)";
}
