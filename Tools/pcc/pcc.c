/*
 * Copyright (c) 2014-2022, Asensing Group
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-06-10     luhuadong    The first version.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <getopt.h>
#include <errno.h>
#include <math.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <endian.h>
#include <sys/time.h>
#include <time.h>

#include "pcc.h"

#define BUFFER_SIZE            1500
static uint8_t buffer[BUFFER_SIZE] = {0};
const char *hello = "Hello from server";

static uint32_t count = 1000000;
static uint32_t gLaserNum = LASER_MODULE_NUM;

// static  uint8_t   bufOrigenAddr[MAXSIZE] = {0};
static uint16_t pkgSn = 0;
static uint16_t distance = 1;  /* for test */

typedef struct point_pair {
    uint16_t azimuth;
    uint16_t elevation;
} point_pair_t;

static point_pair_t point_map[POINT_NUM_PER_FRAME];

static int getRandomInt(int min, int max)
{
    return (random() % (max - min) + min);
}

static void point_map_init()
{
    uint16_t azimuth = 0;
    uint16_t elevation = 0;

    for (int i=0; i<POINT_NUM_PER_LASER_VERTICAL; i++) {

        elevation = elevation + (RESOLUTION_VERTICAL / ASENSING_ELEVATION_UNIT);
        azimuth = 0;

        for (int j=0; j<POINT_NUM_PER_LASER_HORIZON*LASER_MODULE_NUM; j++) {

            azimuth = azimuth + (RESOLUTION_HORIZON / ASENSING_AZIMUTH_UNIT);

            point_map[i*POINT_NUM_PER_LASER_HORIZON*LASER_MODULE_NUM + j].azimuth = azimuth;
            point_map[i*POINT_NUM_PER_LASER_HORIZON*LASER_MODULE_NUM + j].elevation = elevation;
        }
    }
}

static void fill_packet(AsensingPacket *packet, uint16_t distance, uint16_t sn, uint16_t index)
{
    /* Get time */
    struct timeval tv;
    struct tm *pt;
    int pos = index * MAX_POINT_NUM_IN_PACKET;
    DEBUG_PRINT("azimuth: %u, elevation: %u, distance: %u\n", point_map[pos].azimuth, point_map[pos].elevation, distance);

    //uint16_t azimuth = point_map[pos].azimuth;
    //uint16_t elevation = point_map[pos].elevation;

    gettimeofday(&tv, NULL);

    /* 此函数返回的时间日期未经时区转换，而是 UTC 时间 */
    //pt = gmtime(&tv.tv_sec);

    /* 此函数返回的时间日期经过时区转换，是本地时间 */
    pt = localtime(&tv.tv_sec);

#if DEBUG
    printf("start : %ld.%ld\n", tv.tv_sec, tv.tv_usec);
    printf("%d-%02d-%02d %02d:%02d:%02d\n", (1900 + pt->tm_year), (1 + pt->tm_mon), pt->tm_mday, pt->tm_hour, pt->tm_min, pt->tm_sec);
#endif

    /* Header */
    packet->header.Sob = htole32(0x5AA555AA); /* 0xAA, 0x55, 0xA5, 0x5A */
    packet->header.SeqNum = htole16(sn);
    packet->header.VersionMajor = 0x01;
    packet->header.VersionMinor = 0x04;
    //packet->header.DistUnit = 0x04; /* 4mm */
    //packet->header.Flags = 0x0;
    packet->header.LaserNum = 0x08; /* 8 */
    packet->header.BlockNum = 0x0C; /* 12 block */
    packet->header.EchoCount = 0x01;

    packet->header.UTCTime0 = pt->tm_year;
    packet->header.UTCTime1 = pt->tm_mon + 1;
    packet->header.UTCTime2 = pt->tm_mday;
    packet->header.UTCTime3 = pt->tm_hour;
    packet->header.UTCTime4 = pt->tm_min;
    packet->header.UTCTime5 = pt->tm_sec;

    packet->header.Timestamp = htole32(tv.tv_usec);

    packet->header.PointNum = htole32(POINT_NUM_PER_FRAME * packet->header.EchoCount);

    /* Block */
    for (int i = 0; i < BLOCK_NUM; i++)
    {
        packet->blocks[i].channelNum = LASER_CHANNEL_NUM;
        packet->blocks[i].timeOffSet = 0;
        packet->blocks[i].returnSn = 0;

        for (int channel = 0; channel < MAX_POINT_NUM_IN_BLOCK; channel++)
        {
            packet->blocks[i].pointT[channel].distance = getRandomInt(2, 50) * 100;
            packet->blocks[i].pointT[channel].intensity = getRandomInt(0, 256);
            packet->blocks[i].pointT[channel].azimuth = htole16(point_map[pos].azimuth);     /* 低字节在前 */
            packet->blocks[i].pointT[channel].elevation = htole16(point_map[pos].elevation);
            pos++;
        }
    }

#if 0
    /* CRC */
    packet->crc = 0x01020304;

    /* functionSafety */
    packet->functionSafety[0] = 0x00; /* FS Version */
    packet->functionSafety[1] = 0x28; /* 0x01 << 5 || 0x01 << 3 */
#endif

    /* Tail */
    packet->tail.Reserved1 = 0x12;
    packet->tail.Reserved2 = 0x34;
    packet->tail.Reserved3 = 0x56;
    packet->tail.Reserved4 = 0x78;
}

static void show_usage(const char *cmd)
{
    printf("Usage: %s [options] ... \n", cmd);
    printf("This is a point cloud client demo\n\n");
    printf("  -h, --help           display this help and exit\n");
    printf("  -v, --version        output version information and exit\n");
    printf("  -i, --ipaddr=ADDR    set target IP address\n");
    printf("  -p, --port=PORT      set target port\n");
    printf("  -n, --count=NUM      set loop times (number of frames)\n");
    printf("  -l, --laser=NUM      set the number of laser module\n\n");

    exit(0);
}

static void show_version(void)
{
    printf("version %d.%d.%d\n", VER_MAJOR, VER_MINOR, VER_PATCH);
    exit(0);
}

int main(int argc, char *argv[])
{
    int ret = 0, yes = 1;
    int option;
    char *ipaddr = NULL;
    char *port = NULL;
    
    struct sockaddr_in saddr, caddr;
    size_t len = sizeof(struct sockaddr);

    const char *const short_options = "hvi:p:n:l:";
    const struct option long_options[] = {

        {"help",    0, NULL, 'h'},
        {"version", 0, NULL, 'v'},
        {"ipaddr",  1, NULL, 'i'},
        {"port",    1, NULL, 'p'},
        {"count",   1, NULL, 'n'},
        {"laser",   1, NULL, 'l'},
        {NULL, 0, NULL, 0}};

    while ((option = getopt_long(argc, argv, short_options, long_options, NULL)) != -1)
    {
        switch (option)
        {
        case 'h':
            show_usage(argv[0]);
            break;
        case 'v':
            show_version();
            break;
        case 'i':
            ipaddr = strdup(optarg);
            break;
        case 'p':
            port = strdup(optarg);
            break;
        case 'n':
            count = atol(optarg);
            break;
        case 'l':
            gLaserNum = atol(optarg);
            break;
        case '?':
        default:
            printf("Error: option invalid\n");
            exit(EXIT_FAILURE);
            break;
        }
    }

    point_map_init();

    /* Server IP address and port */

    memset(&saddr, 0, sizeof(saddr));
    memset(&caddr, 0, sizeof(caddr));

    // bzero(&saddr, sizeof(struct sockaddr_in));
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = INADDR_ANY;
    saddr.sin_port = htons(DEFAULT_MSOP_PORT - 1);

    printf("Host IP: %s, Port: %d\n", inet_ntoa(saddr.sin_addr), ntohs(saddr.sin_port));

    /* Client IP address and port */
    // bzero(&caddr, sizeof(struct sockaddr_in));
    memset((void *)&caddr, 0, sizeof(struct sockaddr_in));
    caddr.sin_family = AF_INET;

    if (NULL == port)
    {
        caddr.sin_port = htons(DEFAULT_MSOP_PORT);
    }
    else
    {
        caddr.sin_port = htons(atoi(port));
    }

    if (NULL == ipaddr)
    {
        caddr.sin_addr.s_addr = htonl(INADDR_BROADCAST); //htonl(INADDR_ANY);
    }
    else
    {
        caddr.sin_addr.s_addr = inet_addr(ipaddr);
    }

    printf("Target IP: %s, Port: %d\n", inet_ntoa(caddr.sin_addr), ntohs(caddr.sin_port));

    /* Create socket (UDP default) */
    int sockfd;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    /* Enable broadcast */
    ret = setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, (char *)&yes, sizeof(yes));
    if (ret == -1)
    {
        perror("setsockopt error");
        return 0;
    }

    /* Bind the socket with the server address */
    if (bind(sockfd, (const struct sockaddr *)&saddr, sizeof(saddr)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    /* Packaging point cloud data */
    AsensingPacket packet = {0};

    printf("Size of packet = %lu bytes\n", sizeof(packet));
    printf("Size of Header = %lu bytes\n", sizeof(AsensingHeader));
    printf("Size of Block  = %lu bytes (each %lu bytes)\n", sizeof(AsensingBlock) * MAX_BLOCK_NUM * ROLL_NUM, sizeof(AsensingBlock));
    printf("Size of Tail   = %lu bytes\n", sizeof(AsensingTail));

    uint16_t sn = 0;  /* packet sequece number */
    uint32_t frameID = 0;

    uint32_t sr;

    while (count--)
    {
        DEBUG_PRINT("------ %4u\n", count);

        if (distance > 10) {
            distance = 1;
        }

        const char *msg = "Hello";

        /* This is a frame of point cloud */
        for (int i = 0, sn = 0; i < POINT_NUM_PER_FRAME / MAX_POINT_NUM_IN_PACKET; i++) {

            memset(&packet, 0, sizeof(packet));
            packet.header.FrameID = htole32(frameID);

            fill_packet(&packet, distance, sn, i);

            ret = sendto(sockfd, &packet, sizeof(packet), 0, (struct sockaddr *)&caddr, len);
            //ret = sendto(sockfd, &msg, strlen(msg), 0, (struct sockaddr *)&caddr, len);

            if (ret < 0)
            {
                printf("Send package error...\n");
                //close(sockfd);
                //return -1;
            }

            sn++;
        }

        frameID++;

        usleep(200000); // 10Hz
        //sleep(1);
        distance++;
    }

__exit:
    close(sockfd);
    printf("Exit!\n");

    return 0;
}
