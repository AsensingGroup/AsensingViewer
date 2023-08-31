#ifndef LIDAR_KPI_STATISTIC
#define LIDAR_KPI_STATISTIC

#include <stdint.h>

////////////////////////////////////////////////////////////////
//// 临时方案支撑，后续待移植到正式模块提供可靠性取值:start                         ////
////////////////////////////////////////////////////////////////
typedef enum
{
    MEMS_ONLY = 0,
    POLYGON_ONLY,
    MAX_MODE = 8
}ScanTypeE;

typedef enum
{
    OFFSET_NONE   = 0,  // 0 degree
    OFFSET_RESERV = 0b01,
    OFFSET_0_ON   = 0b10,
    OFFSET_25_ON  = 0b11, // 25 degree
    MAX_ANGLE_MOD = 4
}PointCloudMatrixE;

typedef enum
{
    LIDAR_NA = 0,
    LIDAR_A0,
    MAX_LIDAR_PRJ = 256
}LidarProjectNumE;


#pragma pack(push,1)

typedef struct
{
    uint32_t           timeStamp;
    uint16_t           temperature;
    uint16_t           accelerationUnit;
    uint16_t           angularVelocUnit;
    uint16_t           accelerationX;
    uint16_t           accelerationY;
    uint16_t           accelerationZ;
    uint16_t           angularVelocX;
    uint16_t           angularVelocY;
    uint16_t           angularVelocZ;
    uint8_t            rsv[2];
    uint32_t           CRC;
}IMUInfo;

typedef struct
{
    uint16_t           startAngleV;
    uint16_t           endAngleV;
    uint16_t           startAngleH;
    uint16_t           endAngleH;
}FOVCfgInfo;

typedef struct
{
    uint8_t            UTCTime0;              // year
    uint8_t            UTCTime1;              // month
    uint8_t            UTCTime2;              // day
    uint8_t            UTCTime3;              // hour
    uint8_t            UTCTime4;              // minute
    uint8_t            UTCTime5;              // second
    uint32_t           Timestamp;             // identify the timestamp for point cloud
}TimeSyncInfo;

typedef struct
{
    uint32_t           headCode;
    uint16_t           FPS;
    FOVCfgInfo         fovCfgInfo;
    uint8_t            firmwareVersion[6];
    uint8_t            SNSequnce[6];
    uint16_t           flags;
    TimeSyncInfo       timeSyncInfo;
    uint8_t            rsv[2];
}DifPackageHead;

typedef struct
{
    uint8_t            fuSAVer;
    uint8_t            status;
    int8_t             lidarTmp;
    int8_t             laserTmp;
    int8_t             sensorTmp;
    int8_t             MEMSTmp;
    int8_t             windowTmp;
    int8_t             lidarHumidity;
    uint8_t            curFaultNum;
    uint8_t            totalFaultNum;
    uint64_t           curAlarmWarn[2];
    // uint64_t           hisAlarmWarn[2];
    uint32_t           CRC;
}FuncSafeInfo;

typedef struct
{
    uint8_t           signature[32];
}SignalInfo;

typedef struct
{
    DifPackageHead     difPackageHead;
    FuncSafeInfo       funcSafeInfo;
    SignalInfo         signalInfo;
}DeviceInfoPackage;

#pragma pack(pop)

#endif
