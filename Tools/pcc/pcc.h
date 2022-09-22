/*
 * Copyright (c) 2014-2022, Asensing Group
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-06-10     luhuadong    The first version.
 */

#ifndef __PCC_H__
#define __PCC_H__

#ifdef __cplusplus
extern "C" {
#endif

#define VER_MAJOR                      0
#define VER_MINOR                      0
#define VER_PATCH                      1

#define MAX_POINT_NUM_IN_PACKET        (96)
#define LASER_MODULE_NUM               (4)
#define CHANNEL_NUM_PER_MODULE         (2)
#define LASER_CHANNEL_NUM              (LASER_MODULE_NUM * CHANNEL_NUM_PER_MODULE)
#define BLOCK_NUM                      (MAX_POINT_NUM_IN_PACKET / LASER_CHANNEL_NUM)

#define POINT_NUM_PER_LASER_HORIZON    (200)
#define POINT_NUM_PER_LASER_VERTICAL   (48)
#define POINT_NUM_PER_LASER            (POINT_NUM_PER_LASER_HORIZON * POINT_NUM_PER_LASER_VERTICAL)
#define POINT_NUM_PER_FRAME            (POINT_NUM_PER_LASER * LASER_MODULE_NUM)

#define DEFAULT_MSOP_PORT              (51180)
#define DEFAULT_DIFOP_PORT             (51080)

#define MAXSIZE                        (1500)
#define WHILE_NUM                      (1)
#define MAX_POINT_NUM_IN_BLOCK         LASER_CHANNEL_NUM
#define MAX_BLOCK_NUM                  (4)
#define ROLL_NUM                       (3)         /* 配置的回波次数，最大3回波 */

#define ASENSING_DISTANCE_UNIT         (0.01f)
#define ASENSING_AZIMUTH_UNIT          (0.01f)
#define ASENSING_ELEVATION_UNIT        (0.01f)

#define FPS                            (5)         /* Unit: Hz */
#define FOV_HORIZON                    (80)        /* Classic: 360 */
#define FOV_VERTICAL                   (20)        /* Classic: 40 */
#define RESOLUTION_HORIZON             (0.1f)      /* 120 / 32 */
#define RESOLUTION_VERTICAL            (0.4f)

#ifndef FALSE
#define FALSE (0)
#endif
#ifndef TRUE
#define TRUE  (1)
#endif
typedef int StatusCode; 


/* Custom */
#pragma pack(push, 1)
typedef struct
{
    uint16_t distance;      /* 球坐标系径向距离 radius（单位 mm） */
    uint16_t azimuth;       /* 球坐标系水平夹角，方位角（分辨率 0.01°） */
    uint16_t elevation;     /* 球坐标系垂直夹角，俯仰角/极角（分辨率 0.01°） */
    uint8_t  intensity;     /* 反射强度 intensity */
    uint16_t reserved;      /* 保留 */
} PointT;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct
{
    uint8_t channelNum;
    uint8_t timeOffSet;
    uint8_t returnSn;
    uint8_t reserved;
    PointT  pointT[LASER_CHANNEL_NUM];
} AsensingBlock;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct
{
    uint32_t Sob;
    uint32_t FrameID;
    uint16_t SeqNum;
    uint16_t PkgLen;
    uint16_t LidarType;
    uint8_t  VersionMajor;
    uint8_t  VersionMinor;

    uint8_t  UTCTime0;
    uint8_t  UTCTime1;
    uint8_t  UTCTime2;
    uint8_t  UTCTime3;
    uint8_t  UTCTime4;
    uint8_t  UTCTime5;
    uint32_t Timestamp;

    uint8_t  MeasureMode;
    uint8_t  LaserNum;
    uint8_t  BlockNum;
    uint8_t  EchoCount;
    uint8_t  TimeSyncMode;
    uint8_t  TimeSyncStat;
    uint8_t  MemsTemp;
    uint8_t  SlotNum;

    uint16_t PointNum;
    uint16_t Reserved1;
    uint16_t Reserved2;

} AsensingHeader;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct
{
    uint8_t Reserved1;
    uint8_t Reserved2;
    uint8_t Reserved3;
    uint8_t Reserved4;

} AsensingTail;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct
{
    AsensingHeader header;
    AsensingBlock blocks[BLOCK_NUM];
    //uint32_t crc;
    //uint8_t functionSafety[17];
    AsensingTail tail;

} AsensingPacket;
#pragma pack(pop)

#ifdef __cplusplus
}
#endif

#endif /* __PCC_H__ */