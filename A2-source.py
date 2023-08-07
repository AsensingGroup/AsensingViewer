import socket
import struct
import random
import time

UDP_IP = "127.0.0.1"  # 目标IP地址
UDP_PORT = 51180     # 目标UDP端口号
BUFFER_SIZE = 812    # 数据包总长度
PIXEL_COUNT = 96     # 像素数量
FIXED_DISTANCE = 100 #固定径向距离 单位0.01m

# 创建UDP套接字
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

header = 0xA255    # 识别符
frame_number = 0   # 帧号
packet_number = 0  # 包序号
fov_horizon = 120.0
start_azimuth = 30.0  # 水平方位角起始位置（单位：度）
end_azimuth = start_azimuth + fov_horizon  # 水平方位角结束位置（单位：度）
horizontal_resolution = 0.1  # 水平角度分辨率（单位：度）

packet_len = 812
Lidar_type = 0
Lidar_Info = 0
Protocol_version = 0
timestamps = 0
Laser_num = 96
Block_num = 1
Lidar_Flag1 = 0
Lidar_Flag2 = 0
Reserved1 = 0
# PointNumInFrame = Laser_num*int((end_azimuth-start_azimuth)/horizontal_resolution)
PointNumInFrame = 115200


while True:
    # 生成随机数据
    status_info = "Status Information"  # 状态信息
    time_offset = 0  # Block序号
    channel_count = 96  # 通道数
    azimuth = start_azimuth + packet_number * horizontal_resolution  # 水平方位角，0.1°为单位
    invalid_chars = b'\x00\x00'  # 无效字符
    pixels = []
    for _ in range(PIXEL_COUNT):
        # distance = random.randint(0, 10000)  # 径向距离信息
        distance = FIXED_DISTANCE * random.randint(1, 20)  # 径向距离信息
        intensity = 255   # 灰度信息
        confidence = 255  # 置信度信息
        status = 0  # 像素状态信息
        pixels.append(struct.pack('HBBI', distance, intensity, confidence, status))
    pixel_data = b''.join(pixels)

    # 构建数据包
    packet = (
        struct.pack('<B', 0x55) + # 包头识别符
        struct.pack('<B', 0xA2) +
        struct.pack('<B', Lidar_type) +
        struct.pack('<B', Lidar_Info) +
        struct.pack('<B', Protocol_version) +
        struct.pack('<B', Block_num) +
        struct.pack('<H', Laser_num) +
        struct.pack('<B', Lidar_Flag1) +
        struct.pack('<B', Lidar_Flag2) +
        struct.pack('<I', PointNumInFrame) +
        struct.pack('<H', packet_len) +
        struct.pack('<H', frame_number) +             # 帧号
        struct.pack('<H', packet_number) +            # 包序号

        # struct.pack('10s',timestamps) +
        b'\x00' * 10 + 
        struct.pack('<H', Reserved1) +

        # Block
        struct.pack('<H', time_offset) +            # 时间偏移
        struct.pack('<H', int(azimuth * 100)) +     # 水平方位角
        #invalid_chars +                             # 无效字符
        pixel_data +                                # 像素数据

        # Tail
        b'\x00' * 8                                 # 包尾字节
    )

    # 发送数据包
    sock.sendto(packet, (UDP_IP, UDP_PORT))

    # 更新帧号和包序号
    packet_number += 1
    time.sleep(0.001)
    if azimuth >= end_azimuth - horizontal_resolution:
        time.sleep(1)
        packet_number = 0
        frame_number += 1
        if frame_number > 2**32-1:
            frame_number = 0