#ifndef A2PacketFormat_H
#define A2PacketFormat_H

#include <boost/endian/arithmetic.hpp>
#include <memory>

//----------------------------------------------------------------------------
//! @brief Simple getter that handles conversion to native unsigned integer types.
#define GET_NATIVE_UINT(n, attr) uint ## n ##_t Get ## attr() const { return this->attr; }
#define SET_NATIVE_UINT(n, attr) void Set ## attr(uint ## n ##_t x) { this->attr = x; }

#define BIT(n)                  ( 1<<(n) )
//! Create a bitmask of length \a len.
#define BIT_MASK(len)           ( BIT(len)-1 )

//! Create a bitfield mask of length \a starting at bit \a start.
#define BF_MASK(start, len)     ( BIT_MASK(len)<<(start) )

//! Extract a bitfield of length \a len starting at bit \a start from \a y.
#define BF_GET(y, start, len)   ( ((y)>>(static_cast<decltype(y)>(start))) & BIT_MASK(len) )

//! Prepare a bitmask for insertion or combining.
#define BF_PREP(x, start, len)  ( ((x)&BIT_MASK(len)) << (start) )

//! Insert a new bitfield value x into y.
#define BF_SET(y, x, start, len)    ( y= ((y) &~ BF_MASK(start, len)) | BF_PREP(x, start, len) )

#define CIRCLE (36000)

/* Custom */
#define LASER_MODULE_NUM             (4)
#define CHANNEL_NUM_PER_MODULE       (2) /* 1 Laser => 2 Channel */

#define ASENSING_DISTANCE_UNIT       (0.01f)
#define ASENSING_AZIMUTH_UNIT        (0.01f)
#define ASENSING_ELEVATION_UNIT      (0.01f)

#define ASENSING_LASER_NUM           (LASER_MODULE_NUM * CHANNEL_NUM_PER_MODULE)  /* eg. 4 * 2 = 8 */
#define ASENSING_BLOCK_NUM           (12)                                         /* 12 blocks per packet */
#define ASENSING_POINT_PER_PACKET    (ASENSING_BLOCK_NUM * ASENSING_LASER_NUM)    /* eg. 12 * 8 = 96, 12 * 10 = 120 */

#define ASENSING_POINT_NUM           (4800 * ASENSING_LASER_NUM)  /* 忽略，使用动态计算 (38400) */
#define ASENSING_POINT_NUM_MAX       (ASENSING_POINT_NUM * 6)     /* Dual echo & High precision for max ROI */

#define ASENSING_SOB_SIZE            (4)
#define ASENSING_FRAME_ID_SIZE       (4)
#define ASENSING_SEQ_NUM_SIZE        (2)
#define ASENSING_PACKET_LEN_SIZE     (2)
#define ASENSING_LIDAR_TYPE_SIZE     (2)
#define ASENSING_VERSION_MAJOR_SIZE  (1)
#define ASENSING_VERSION_MINOR_SIZE  (1)
#define ASENSING_TIMESTAMP_SIZE      (10)
#define ASENSING_MEASURE_MODE_SIZE   (1)
#define ASENSING_LASER_NUM_SIZE      (1)
#define ASENSING_BLOCK_NUM_SIZE      (1)
#define ASENSING_ECHO_COUNT_SIZE     (1)
#define ASENSING_TIME_SYNC_MODE_SIZE (1)
#define ASENSING_TIME_SYNC_STAT_SIZE (1)
#define ASENSING_MEMS_TEMP_SIZE      (1)
#define ASENSING_SLOT_NUM_SIZE       (1)
#define ASENSING_HEAD_RESERVED1_SIZE (2)
#define ASENSING_HEAD_RESERVED2_SIZE (2)
#define ASENSING_HEAD_RESERVED3_SIZE (2)
#define ASENSING_HEAD_SIZE \
  (ASENSING_SOB_SIZE + ASENSING_FRAME_ID_SIZE + ASENSING_SEQ_NUM_SIZE + ASENSING_PACKET_LEN_SIZE + \
   ASENSING_LIDAR_TYPE_SIZE + ASENSING_VERSION_MAJOR_SIZE + ASENSING_VERSION_MINOR_SIZE + \
   ASENSING_TIMESTAMP_SIZE + ASENSING_MEASURE_MODE_SIZE + ASENSING_LASER_NUM_SIZE + \
   ASENSING_BLOCK_NUM_SIZE + ASENSING_ECHO_COUNT_SIZE + ASENSING_TIME_SYNC_MODE_SIZE + \
   ASENSING_TIME_SYNC_STAT_SIZE + ASENSING_MEMS_TEMP_SIZE + ASENSING_SLOT_NUM_SIZE + \
   ASENSING_HEAD_RESERVED1_SIZE + ASENSING_HEAD_RESERVED2_SIZE + ASENSING_HEAD_RESERVED3_SIZE)

#define ASENSING_BLOCK_CHANNEL_SIZE      (1)
#define ASENSING_BLOCK_TIME_OFFSET_SIZE  (1)
#define ASENSING_BLOCK_RETURN_SN_SIZE    (1)
#define ASENSING_BLOCK_RESERVED_SIZE     (1)

#define ASENSING_UNIT_DISTANCE_SIZE      (2)
#define ASENSING_UNIT_AZIMUTH_SIZE       (2)
#define ASENSING_UNIT_ELEVATION_SIZE     (2)
#define ASENSING_UNIT_INTENSITY_SIZE     (1)
#define ASENSING_UNIT_RESERVED_SIZE      (2)

#define ASENSING_UNIT_SIZE \
  (ASENSING_UNIT_DISTANCE_SIZE + ASENSING_UNIT_AZIMUTH_SIZE + ASENSING_UNIT_ELEVATION_SIZE + \
   ASENSING_UNIT_INTENSITY_SIZE + ASENSING_UNIT_RESERVED_SIZE)
#define ASENSING_BLOCK_SIZE \
  (ASENSING_UNIT_SIZE * ASENSING_LASER_NUM + ASENSING_BLOCK_CHANNEL_SIZE + \
   ASENSING_BLOCK_TIME_OFFSET_SIZE + ASENSING_BLOCK_RETURN_SN_SIZE + ASENSING_BLOCK_RESERVED_SIZE)

#define ASENSING_TAIL_RESERVED_SIZE      (4)
#define ASENSING_TAIL_SIZE               (ASENSING_TAIL_RESERVED_SIZE)

#define ASENSING_PACKET_SIZE                                         \
  (ASENSING_HEAD_SIZE + ASENSING_BLOCK_SIZE * ASENSING_BLOCK_NUM + \
   ASENSING_TAIL_SIZE)


/* Custom */
#pragma pack(push, 1)
class AsensingUnit
{
private :
  boost::endian::little_uint16_t Distance;  /* 球坐标系径向距离 radius（单位 cm） */
  boost::endian::little_uint16_t Azimuth;   /* 球坐标系水平夹角，方位角（分辨率 0.01°） */
  boost::endian::little_uint16_t Elevation; /* 球坐标系垂直夹角，俯仰角/极角（分辨率 0.01°） */
  boost::endian::little_uint8_t  Intensity; /* 反射强度 intensity */
  boost::endian::little_uint16_t Reserved;  /* 保留 */

public :
  GET_NATIVE_UINT(16, Distance)
  SET_NATIVE_UINT(16, Distance)
  GET_NATIVE_UINT(16, Azimuth)
  SET_NATIVE_UINT(16, Azimuth)
  GET_NATIVE_UINT(16, Elevation)
  SET_NATIVE_UINT(16, Elevation)
  GET_NATIVE_UINT(8,  Intensity)
  SET_NATIVE_UINT(8,  Intensity)
  GET_NATIVE_UINT(16, Reserved)
  SET_NATIVE_UINT(16, Reserved)
};
#pragma pack(pop)


#pragma pack(push, 1)
class AsensingBlock
{
private :
  boost::endian::little_uint8_t channelNum;
  boost::endian::little_uint8_t timeOffSet;
  boost::endian::little_uint8_t returnSn;
  boost::endian::little_uint8_t reserved;

public :
  AsensingUnit units[ASENSING_LASER_NUM];

public :
  GET_NATIVE_UINT(8, channelNum)
  SET_NATIVE_UINT(8, channelNum)
  GET_NATIVE_UINT(8, timeOffSet)
  SET_NATIVE_UINT(8, timeOffSet)
  GET_NATIVE_UINT(8, returnSn)
  SET_NATIVE_UINT(8, returnSn)
  GET_NATIVE_UINT(8, reserved)
  SET_NATIVE_UINT(8, reserved)
};
#pragma pack(pop)


#pragma pack(push, 1)
//! @brief class representing the Raw packet
/*
   0               1               2               3
   0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |               Sob              |  VersionMajor | VersionMinor |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |    DistUnit    |     Flags     |   LaserNum    |  BlockNum    |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |    EchoCount   |   EchoNum     |          Reserved            |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */
class AsensingHeader
{
private:
  boost::endian::little_uint32_t Sob;
  boost::endian::little_uint32_t FrameID;
  boost::endian::little_uint16_t SeqNum;
  boost::endian::little_uint16_t PkgLen;
  boost::endian::little_uint16_t LidarType;
  boost::endian::little_uint8_t  VersionMajor;  /* Protocol version */
  boost::endian::little_uint8_t  VersionMinor;

  boost::endian::little_uint8_t  UTCTime0;      /* Year   */
  boost::endian::little_uint8_t  UTCTime1;      /* Month  */
  boost::endian::little_uint8_t  UTCTime2;      /* Mday   */
  boost::endian::little_uint8_t  UTCTime3;      /* Hour   */
  boost::endian::little_uint8_t  UTCTime4;      /* Minute */
  boost::endian::little_uint8_t  UTCTime5;      /* Second */

  boost::endian::little_uint32_t Timestamp;     /* Microseconds */

  boost::endian::little_uint8_t  MeasureMode;
  boost::endian::little_uint8_t  LaserNum;
  boost::endian::little_uint8_t  BlockNum;
  boost::endian::little_uint8_t  EchoCount;     /* Wave mode: 第一回波、最后回波、最强回波、双回波 */
  boost::endian::little_uint8_t  TimeSyncMode;  /* 1: System time, 2: 1PPS, 3: PTP */
  boost::endian::little_uint8_t  TimeSyncStat;
  boost::endian::little_uint8_t  MemsTemp;      /* Temperature of MEMS */
  boost::endian::little_uint8_t  SlotNum;

  boost::endian::little_uint32_t PointNum;      /* The number of points in a frame */
  boost::endian::little_uint16_t Reserved1;     /* e.g DistUnit, Flags ... */

public:
  GET_NATIVE_UINT(32, Sob)
  SET_NATIVE_UINT(32, Sob)
  GET_NATIVE_UINT(32, FrameID)
  SET_NATIVE_UINT(32, FrameID)
  GET_NATIVE_UINT(16, SeqNum)
  SET_NATIVE_UINT(16, SeqNum)
  GET_NATIVE_UINT(16, PkgLen)
  SET_NATIVE_UINT(16, PkgLen)
  GET_NATIVE_UINT(16, LidarType)
  SET_NATIVE_UINT(16, LidarType)
  GET_NATIVE_UINT(8, VersionMajor)
  SET_NATIVE_UINT(8, VersionMajor)
  GET_NATIVE_UINT(8, VersionMinor)
  SET_NATIVE_UINT(8, VersionMinor)

  GET_NATIVE_UINT(32, Timestamp)
  SET_NATIVE_UINT(32, Timestamp)

  GET_NATIVE_UINT(8, UTCTime0)
  SET_NATIVE_UINT(8, UTCTime0)
  GET_NATIVE_UINT(8, UTCTime1)
  SET_NATIVE_UINT(8, UTCTime1)
  GET_NATIVE_UINT(8, UTCTime2)
  SET_NATIVE_UINT(8, UTCTime2)
  GET_NATIVE_UINT(8, UTCTime3)
  SET_NATIVE_UINT(8, UTCTime3)
  GET_NATIVE_UINT(8, UTCTime4)
  SET_NATIVE_UINT(8, UTCTime4)
  GET_NATIVE_UINT(8, UTCTime5)
  SET_NATIVE_UINT(8, UTCTime5)
  
  GET_NATIVE_UINT(8, MeasureMode)
  SET_NATIVE_UINT(8, MeasureMode)
  GET_NATIVE_UINT(8, LaserNum)
  SET_NATIVE_UINT(8, LaserNum)
  GET_NATIVE_UINT(8, BlockNum)
  SET_NATIVE_UINT(8, BlockNum)
  GET_NATIVE_UINT(8, EchoCount)
  SET_NATIVE_UINT(8, EchoCount)
  GET_NATIVE_UINT(8, TimeSyncMode)
  SET_NATIVE_UINT(8, TimeSyncMode)
  GET_NATIVE_UINT(8, TimeSyncStat)
  SET_NATIVE_UINT(8, TimeSyncStat)
  GET_NATIVE_UINT(8, MemsTemp)
  SET_NATIVE_UINT(8, MemsTemp)
  GET_NATIVE_UINT(8, SlotNum)
  SET_NATIVE_UINT(8, SlotNum)

  GET_NATIVE_UINT(32, PointNum)
  SET_NATIVE_UINT(32, PointNum)
  GET_NATIVE_UINT(16, Reserved1)
  SET_NATIVE_UINT(16, Reserved1)
};
#pragma pack(pop)


#pragma pack(push, 1)
//! @brief class representing the Raw packet
class AsensingTail
{
private:
  boost::endian::little_uint32_t Reserved;
public:
  GET_NATIVE_UINT(32, Reserved)
  SET_NATIVE_UINT(32, Reserved)
};
#pragma pack(pop)


#pragma pack(push, 1)
//! @brief class representing the Asensing Packet
struct AsensingPacket {
  AsensingHeader header;
  AsensingBlock blocks[ASENSING_BLOCK_NUM];

  // Reserved CRC_SIZE is equal to 4
  //boost::endian::little_uint32_t crc;

  // Reserved FUNCTION_SAFETY_SIZE is equal to 17
  //boost::endian::little_uint8_t functionSafety[17];

  AsensingTail tail;
};
#pragma pack(pop)

#endif // A2PacketFormat_H
