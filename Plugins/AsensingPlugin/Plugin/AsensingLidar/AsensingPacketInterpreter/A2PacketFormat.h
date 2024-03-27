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
#define ASENSING_DISTANCE_UNIT       (0.01f)
#define ASENSING_AZIMUTH_UNIT        (0.01f)
#define ASENSING_ELEVATION_UNIT      (0.01f)

#define A2_CHANNEL_NUM               (192)
#define A2_BLOCK_NUM                 (1)
#define A2_POINT_PER_PACKET          (A2_BLOCK_NUM * A2_CHANNEL_NUM)    /* eg. 12 * 8 = 96, 12 * 10 = 120 */

#define A2_POINT_NUM                 (1200 * A2_CHANNEL_NUM)      /* 忽略，使用动态计算 */
#define A2_POINT_NUM_MAX             (A2_POINT_NUM * 4)     /* Dual echo & High precision for max ROI */

#define ASENSING_SOB_SIZE            (2)
#define ASENSING_LIDAR_TYPE_SIZE     (1)
#define ASENSING_LIDAR_INFO_SIZE     (1)
#define ASENSING_VERSION_SIZE        (1)
#define ASENSING_BLOCK_NUM_SIZE      (1)
#define ASENSING_CHANNEL_NUM_SIZE    (2)
#define ASENSING_LIDAR_FLAG_SIZE     (2)
#define ASENSING_POINT_NUM_SIZE      (4)
#define ASENSING_PACKET_LEN_SIZE     (2)
#define ASENSING_FRAME_ID_SIZE       (2)
#define ASENSING_SEQ_NUM_SIZE        (2)
#define ASENSING_TIMESTAMP_SIZE      (10)
#define ASENSING_HEAD_RESERVED1_SIZE (2)
#define ASENSING_HEAD_SIZE \
  (ASENSING_SOB_SIZE + ASENSING_LIDAR_TYPE_SIZE + ASENSING_LIDAR_INFO_SIZE + ASENSING_VERSION_SIZE + \
   ASENSING_BLOCK_NUM_SIZE + ASENSING_CHANNEL_NUM_SIZE + ASENSING_LIDAR_FLAG_SIZE + \
   ASENSING_POINT_NUM_SIZE + ASENSING_PACKET_LEN_SIZE + ASENSING_FRAME_ID_SIZE + \
   ASENSING_SEQ_NUM_SIZE + ASENSING_TIMESTAMP_SIZE + ASENSING_HEAD_RESERVED1_SIZE)

#define ASENSING_BLOCK_TIME_OFFSET_SIZE  (2)
#define ASENSING_BLOCK_AZIMUTH_SIZE      (2)

#define ASENSING_UNIT_DISTANCE_SIZE      (2)
#define ASENSING_UNIT_INTENSITY_SIZE     (1)
#define ASENSING_UNIT_CONFIDENCE_SIZE    (1)

#define ASENSING_UNIT_SIZE \
  (ASENSING_UNIT_DISTANCE_SIZE + ASENSING_UNIT_INTENSITY_SIZE + ASENSING_UNIT_CONFIDENCE_SIZE)
#define ASENSING_BLOCK_SIZE \
  (ASENSING_UNIT_SIZE * A2_CHANNEL_NUM + ASENSING_BLOCK_TIME_OFFSET_SIZE + ASENSING_BLOCK_AZIMUTH_SIZE)

#define ASENSING_TAIL_CHECKSUM_SIZE      (4)
#define ASENSING_TAIL_RESERVED_SIZE      (4)
#define ASENSING_TAIL_SIZE               (ASENSING_TAIL_CHECKSUM_SIZE + ASENSING_TAIL_RESERVED_SIZE)

#define ASENSING_PACKET_SIZE \
  (ASENSING_HEAD_SIZE + ASENSING_BLOCK_SIZE * A2_BLOCK_NUM + ASENSING_TAIL_SIZE)


/* Custom */
#pragma pack(push, 1)
class A2Unit
{
private :
  boost::endian::little_uint16_t Distance;    /* 球坐标系径向距离 radius（单位 cm） */
  boost::endian::little_uint8_t  Intensity;   /* 反射强度 intensity */
  boost::endian::little_uint8_t  Confidence;  /* 置信度 */

public :
  GET_NATIVE_UINT(16, Distance)
  SET_NATIVE_UINT(16, Distance)
  GET_NATIVE_UINT(8, Intensity)
  SET_NATIVE_UINT(8, Intensity)
  GET_NATIVE_UINT(8, Confidence)
  SET_NATIVE_UINT(8, Confidence)
};
#pragma pack(pop)


#pragma pack(push, 1)
class A2Block
{
private :
  boost::endian::little_uint16_t TimeOffset;
  boost::endian::little_uint16_t Azimuth;   /* 球坐标系水平夹角，方位角（分辨率 0.01°） */

public :
  A2Unit units[A2_CHANNEL_NUM];

public :
  GET_NATIVE_UINT(16, TimeOffset)
  SET_NATIVE_UINT(16, TimeOffset)
  GET_NATIVE_UINT(16, Azimuth)
  SET_NATIVE_UINT(16, Azimuth)
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
class A2Header
{
private:
  boost::endian::little_uint16_t Sob;
  boost::endian::little_uint8_t  LidarType;
  boost::endian::little_uint8_t  LidarInfo;
  boost::endian::little_uint8_t  Version;  /* Protocol version */
  boost::endian::little_uint8_t  BlockNum;
  boost::endian::little_uint16_t ChannelNum;
  boost::endian::little_uint16_t LidarFlag;
  boost::endian::little_uint32_t PointNum;      /* The number of points in a frame */
  boost::endian::little_uint16_t PkgLen;
  boost::endian::little_uint16_t FrameID;
  boost::endian::little_uint16_t SeqNum;

  boost::endian::little_uint8_t  UTCTime0;      /* Year   */
  boost::endian::little_uint8_t  UTCTime1;      /* Month  */
  boost::endian::little_uint8_t  UTCTime2;      /* Mday   */
  boost::endian::little_uint8_t  UTCTime3;      /* Hour   */
  boost::endian::little_uint8_t  UTCTime4;      /* Minute */
  boost::endian::little_uint8_t  UTCTime5;      /* Second */

  boost::endian::little_uint32_t Timestamp;     /* Microseconds */

  boost::endian::little_uint16_t Reserved1;     /* e.g DistUnit, Flags ... */

public:
  GET_NATIVE_UINT(16, Sob)
  SET_NATIVE_UINT(16, Sob)
  GET_NATIVE_UINT(8,  LidarType)
  SET_NATIVE_UINT(8,  LidarType)
  GET_NATIVE_UINT(8,  LidarInfo)
  SET_NATIVE_UINT(8,  LidarInfo)
  GET_NATIVE_UINT(8,  Version)
  SET_NATIVE_UINT(8,  Version)
  GET_NATIVE_UINT(8,  BlockNum)
  SET_NATIVE_UINT(8,  BlockNum)
  GET_NATIVE_UINT(16, ChannelNum)
  SET_NATIVE_UINT(16, ChannelNum)
  GET_NATIVE_UINT(16, LidarFlag)
  SET_NATIVE_UINT(16, LidarFlag)
  GET_NATIVE_UINT(32, PointNum)
  SET_NATIVE_UINT(32, PointNum)
  GET_NATIVE_UINT(16, PkgLen)
  SET_NATIVE_UINT(16, PkgLen)
  GET_NATIVE_UINT(16, FrameID)
  SET_NATIVE_UINT(16, FrameID)
  GET_NATIVE_UINT(16, SeqNum)
  SET_NATIVE_UINT(16, SeqNum)
  
  GET_NATIVE_UINT(32, Timestamp)
  SET_NATIVE_UINT(32, Timestamp)

  GET_NATIVE_UINT(8,  UTCTime0)
  SET_NATIVE_UINT(8,  UTCTime0)
  GET_NATIVE_UINT(8,  UTCTime1)
  SET_NATIVE_UINT(8,  UTCTime1)
  GET_NATIVE_UINT(8,  UTCTime2)
  SET_NATIVE_UINT(8,  UTCTime2)
  GET_NATIVE_UINT(8,  UTCTime3)
  SET_NATIVE_UINT(8,  UTCTime3)
  GET_NATIVE_UINT(8,  UTCTime4)
  SET_NATIVE_UINT(8,  UTCTime4)
  GET_NATIVE_UINT(8,  UTCTime5)
  SET_NATIVE_UINT(8,  UTCTime5)
  
  GET_NATIVE_UINT(16, Reserved1)
  SET_NATIVE_UINT(16, Reserved1)
};
#pragma pack(pop)


#pragma pack(push, 1)
//! @brief class representing the Raw packet
class A2Tail
{
private:
  boost::endian::little_uint32_t CRC;
  boost::endian::little_uint32_t Reserved;
public:
  GET_NATIVE_UINT(32, CRC)
  SET_NATIVE_UINT(32, CRC)
  GET_NATIVE_UINT(32, Reserved)
  SET_NATIVE_UINT(32, Reserved)
};
#pragma pack(pop)


#pragma pack(push, 1)
//! @brief class representing the Asensing Packet
struct A2Packet {
  A2Header header;
  A2Block blocks[A2_BLOCK_NUM];

  // Reserved CRC_SIZE is equal to 4
  //boost::endian::little_uint32_t crc;

  // Reserved FUNCTION_SAFETY_SIZE is equal to 17
  //boost::endian::little_uint8_t functionSafety[17];

  A2Tail tail;
};
#pragma pack(pop)

#endif // A2PacketFormat_H
