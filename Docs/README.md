# AsensingViewer

AsensingViewer project for point cloud



## Dependencies

| Project                          | SHA      | URL                                                          |
| -------------------------------- | -------- | ------------------------------------------------------------ |
| lv-sb                            | c7b92a87 | <https://gitlab.kitware.com/LidarView/lidarview-superbuild>  |
| lv-sb/common-superbuild          | 69c10677 | <https://gitlab.kitware.com/paraview/common-superbuild>      |
| LVCore                           | 52553728 | <https://gitlab.kitware.com/LidarView/lidarview-core>        |
| LVCore/LidarSlamPlugin/LidarSlam | dffecb3c | <https://gitlab.kitware.com/keu-computervision/slam>         |
| Plugins/VelodynePlugin           | 31dd3fe3 | <https://gitlab.kitware.com/LidarView/velodyneplugin>        |
| Plugins/HesaiPlugin              | be9094bd | <https://gitlab.kitware.com/LidarView/HesaiPlugin>           |
| Plugins/HesaiPlugin/SDK          | f272fc05 | <https://gitlab.kitware.com/LidarView/HesaiLidar_General_SDK> |
|                                  |          |                                                              |

Notes:

- lv-sb = LidarView-Superbuild
- LVCore = LidarView-Core

## Format

```bash
+--------+--------+--------+
| Header | Blocks |  Tail  |
+--------+--------+--------+
```

数据块区间是 MSOP 包中雷达测量值部分，由多个 block 组成，关于数据包、block 数、一个 block 数中的 channel 数之间的关系，该部分设计支持变长模式，确定方法如下：

1. 一个 block 中 channel 数即为激光线束数；
2. 保证一个数据包有尽可能多的 block 数；
3. 保证整个包组成以太网帧后长度不超过 1500 字节；
4. 由 block 数、channel 数、roll 数据最终确定的 package size 固定为 864 字节。

Package 的具体说明如下：

| Mode | Roll | Channel | Block | 备注 |
| --- | --- | --- | --- | --- |
| 单回波 | 1 | 8 | 12 | |
| 单回波 | 1 | 2 | 48 | 前期调试单扇区配置 |
| 双回波 | 2 | 8 | 6 | |
| 双回波 | 2 | 2 | 24 | |
| 三回波 | 3 | 8 | 4 | |
| 三回波 | 3 | 2 | 16 | |

