#ifndef VTKAsensingPacketInterpreter_h
#define VTKAsensingPacketInterpreter_h

#include <vtkLidarPacketInterpreter.h>

#include <vtkDoubleArray.h>
#include <vtkTypeUInt64Array.h>
#include <vtkUnsignedIntArray.h>

#include "laser_ts.h"
#include "AsensingPacketFormat.h"

#include <memory>

struct point_xyz {
    double x;
    double y;
    double z;
};

static const float elev_angle[] = {
    14.436f,  13.535f,  13.08f,   12.624f,  12.163f,  11.702f,  11.237f,
    10.771f,  10.301f,  9.83f,    9.355f,   8.88f,    8.401f,   7.921f,
    7.437f,   6.954f,   6.467f,   5.98f,    5.487f,   4.997f,   4.501f,
    4.009f,   3.509f,   3.014f,   2.512f,   2.014f,   1.885f,   1.761f,
    1.637f,   1.511f,   1.386f,   1.258f,   1.13f,    1.009f,   0.88f,
    0.756f,   0.63f,    0.505f,   0.379f,   0.251f,   0.124f,   0.0f,
    -0.129f,  -0.254f,  -0.38f,   -0.506f,  -0.632f,  -0.76f,   -0.887f,
    -1.012f,  -1.141f,  -1.266f,  -1.393f,  -1.519f,  -1.646f,  -1.773f,
    -1.901f,  -2.027f,  -2.155f,  -2.282f,  -2.409f,  -2.535f,  -2.662f,
    -2.789f,  -2.916f,  -3.044f,  -3.172f,  -3.299f,  -3.425f,  -3.552f,
    -3.680f,  -3.806f,  -3.933f,  -4.062f,  -4.190f,  -4.318f,  -4.444f,
    -4.571f,  -4.698f,  -4.824f,  -4.951f,  -5.081f,  -5.209f,  -5.336f,
    -5.463f,  -5.589f,  -5.717f,  -5.843f,  -5.968f,  -6.099f,  -6.607f,
    -7.118f,  -7.624f,  -8.135f,  -8.64f,   -9.149f,  -9.652f,  -10.16f,
    -10.664f, -11.17f,  -11.67f,  -12.174f, -12.672f, -13.173f, -13.668f,
    -14.166f, -14.658f, -15.154f, -15.643f, -16.135f, -16.62f,  -17.108f,
    -17.59f,  -18.073f, -18.548f, -19.031f, -19.501f, -19.981f, -20.445f,
    -20.92f,  -21.379f, -21.85f,  -22.304f, -22.77f,  -23.219f, -23.68f,
    -24.123f, -25.016f,
};

static const float azimuth_offset[] = {
    3.257f,  3.263f, -1.083f, 3.268f, -1.086f, 3.273f,  -1.089f, 3.278f,
    -1.092f, 3.283f, -1.094f, 3.288f, -1.097f, 3.291f,  -1.1f,   1.1f,
    -1.102f, 1.1f,   -3.306f, 1.102f, -3.311f, 1.103f,  -3.318f, 1.105f,
    -3.324f, 1.106f, 7.72f,   5.535f, 3.325f,  -3.33f,  -1.114f, -5.538f,
    -7.726f, 1.108f, 7.731f,  5.543f, 3.329f,  -3.336f, -1.116f, -5.547f,
    -7.738f, 1.108f, 7.743f,  5.551f, 3.335f,  -3.342f, -1.119f, -5.555f,
    -7.75f,  1.11f,  7.757f,  5.56f,  3.34f,   -3.347f, -1.121f, -5.564f,
    -7.762f, 1.111f, 7.768f,  5.569f, 3.345f,  -3.353f, -1.123f, -5.573f,
    -7.775f, 1.113f, 7.780f,  5.578f, 3.351f,  -3.358f, -1.125f, -5.582f,
    -7.787f, 1.115f, 7.792f,  5.586f, 3.356f,  -3.363f, -1.126f, -5.591f,
    -7.799f, 1.117f, 7.804f,  5.595f, 3.36f,   -3.369f, -1.128f, -5.599f,
    -7.811f, 1.119f, -3.374f, 1.12f,  -3.379f, 1.122f,  -3.383f, 3.381f,
    -3.388f, 3.386f, -1.135f, 3.39f,  -1.137f, 3.395f,  -1.138f, 3.401f,
    -1.139f, 3.406f, -1.14f,  3.41f,  -1.141f, 3.416f,  -1.142f, 1.14f,
    -1.143f, 1.143f, -3.426f, 1.146f, -3.429f, 1.147f,  -3.433f, 1.15f,
    -3.436f, 1.152f, -3.44f,  1.154f, -3.443f, 1.157f,  -3.446f, -3.449f,
};


class VTK_EXPORT vtkAsensingPacketInterpreter : public vtkLidarPacketInterpreter
{
public:
  static vtkAsensingPacketInterpreter* New();
  vtkTypeMacro(vtkAsensingPacketInterpreter, vtkLidarPacketInterpreter)
  //void PrintSelf(ostream& vtkNotUsed(os), vtkIndent vtkNotUsed(indent)) override;

  void LoadCalibration(const std::string& filename) override;

  bool PreProcessPacket(unsigned char const * data, unsigned int dataLength,
                        fpos_t filePosition = fpos_t(), double packetNetworkTime = 0,
                        std::vector<FrameInformation>* frameCatalog = nullptr) override;

  bool IsLidarPacket(unsigned char const * data, unsigned int dataLength) override;

  void ProcessPacket(unsigned char const * data, unsigned int dataLength) override;

  std::string GetSensorInformation(bool shortVersion = false) override;

protected:
  template<typename T>
  vtkSmartPointer<T> CreateDataArray(bool isAdvanced, const char* name, vtkIdType np, vtkIdType prereserved_np, vtkPolyData* pd);

  vtkSmartPointer<vtkPolyData> CreateNewEmptyFrame(vtkIdType numberOfPoints, vtkIdType prereservedNumberOfPoints = 60000) override;

  vtkSmartPointer<vtkPoints> Points;
  vtkSmartPointer<vtkDoubleArray> PointsX;
  vtkSmartPointer<vtkDoubleArray> PointsY;
  vtkSmartPointer<vtkDoubleArray> PointsZ;

  vtkSmartPointer<vtkUnsignedIntArray> LaserID;
  vtkSmartPointer<vtkUnsignedCharArray> Intensities;
  vtkSmartPointer<vtkDoubleArray> Timestamps;
  vtkSmartPointer<vtkDoubleArray> Distances;


  vtkAsensingPacketInterpreter();
  ~vtkAsensingPacketInterpreter();

private:
  vtkAsensingPacketInterpreter(const vtkAsensingPacketInterpreter&) = delete;
  void operator=(const vtkAsensingPacketInterpreter&) = delete;

  std::vector<double> Cos_all_angle;
  std::vector<double> Sin_all_angle;
  LasersTSOffset LaserOffset;

  //! @brief Calibration of each laser get from the calibration file
  std::vector<double> ElevationCorrection;
  std::vector<double> AzimuthCorrection;

#if 0
  std::vector<struct point_xyz> Correction;
#else
  std::vector<double> XCorrection;  // 9600 / 2 * 8 = 38400
  std::vector<double> YCorrection;
  std::vector<double> ZCorrection;

  bool CalibEnabled = false;
#endif

  uint8_t echo_count = 1;
  int current_pt_id = 0;
  uint32_t points_per_frame = ASENSING_POINT_NUM_MAX;
  uint32_t current_frame_id = 0;
};

struct AsensingSpecificFrameInformation : public SpecificFrameInformation
{
  // Declare 2 LastAzimuth values, one is used for Preprocess Packet
  // The other is used for ProcessPacket
  double LastAzimuth[2] = {-1, -1};
  uint32_t LastFrameID[2] = {0, 0};

#if 1
  bool IsNewFrame(int index, uint32_t newFrameID)
  {

    if(index > 1)
    {
      std::cerr << "Last FrameID not updated, wrong index" << std::endl;
      return false;
    }

    // We always update the LastAzimuth
    uint32_t prev_LastFrameID = this->LastFrameID[index];
    this->LastFrameID[index] = newFrameID;

    // We always update
    if(prev_LastFrameID == 0)
    {
      return false;
    }

    // We have to check if new Azimuth is 5 because
    if(newFrameID != prev_LastFrameID)
    //if((newFrameID != prev_LastFrameID) && (newFrameID % 50 == 0))
    {
      return true;
    }
    return false;
  }
#else
  bool IsNewFrame(int index, double newAzimuth)
  {
    if(index > 1)
    {
      std::cerr << "Last Azimuth not updated, wrong index" << std::endl;
      return false;
    }

    // We always update the LastAzimuth
    double prev_LastAzimuth = this->LastAzimuth[index];
    this->LastAzimuth[index] = newAzimuth;

    // We always update
    if(prev_LastAzimuth == -1)
    {
      return false;
    }

    // We have to check if new Azimuth is 5 because
    if(newAzimuth < prev_LastAzimuth && newAzimuth == 5)
    {
      return true;
    }
    return false;
  }
#endif

  void reset() override { *this = AsensingSpecificFrameInformation(); }
  std::unique_ptr<SpecificFrameInformation> clone() override { return std::make_unique<AsensingSpecificFrameInformation>(*this); }
};



#endif // VTKAsensingPacketInterpreter_h
