# 更改日志

## AsensingViewer V1.1.0

- 解决切换各种模式引起点数变化导致的崩溃问题
- 解决intensity、laser id渲染颜色异常、点云闪烁和数据跳变问题
- 解决A2水平角和垂直角异常问题
- 支持A0距离分辨率动态调整：0.005 or 0.01
- 增加过滤对话框全选和全不选按钮
- 支持A2 192线协议及其过滤功能，更新对应的标定文件参数
- Tools栏增加Windows防火墙配置入口，Linux版本则没有该功能

## AsensingViewer V1.0.0

- 默认点云点大小为2

## AsensingViewer V0.49.0

- 软件更名为AsensingViewer
- 支持点云的Filter功能，入口在菜单栏Tools下
- 支持A2（elevation）原始角度文件标定，A2 spreadsheet新增置信度、包序号、面id、水平角、垂直角展示
- 新增difop设备信息监控和参数告警界面，入口在菜单栏Tools下，默认是不打开
- 覆盖安装的时候允许用户另安装路径

## AsensingView V0.48.1

- 解决颜色配置冲突导致点云闪烁的问题
- 点云大小默认为1

## AsensingView V0.48.0

- 支持高亮colormap配色
- 解决上位机长时间播放卡顿问题
- 默认编译模式为release
- 更新 A2 点云协议
- 增加 A2 雷达水平角度偏移设置
- A2 点云增加 Channel 字段信息

## AsensingView V0.47.0

- 将上位机软件名称由 NeptuneViewer 修改为 AsensingView
- 增加 A2 点云协议支持
- 修复选择帧重新导出 pcap 文件功能
- 增加导入路径对中文字符的支持
- 屏蔽深度图功能

## NeptuneViewer V0.46.0

- 支持深度图模式（加载pcap时）
- 解决csv文件导出时间戳精度丢失的问题

## NeptuneViewer V0.45.2

- 修复导入双回波 pcap 数据时崩溃的问题

## NeptuneViewer V0.45.0

- 修复不同mems类型切换崩溃的问题
- 增加角度模型算法支持
- 增加帧校验
- 增加RT_enable字段决定是否开启RT矩阵算法

## NeptuneViewer V0.44.0

- 修复解析器加载错误的问题
- 支持3D窗口全屏
- 兼容旧版pcap数据回读

## NeptuneViewer V0.43.0

- 增加对 5 模组点云协议的支持
- 增加 New Color Preset 点云渲染配色模板
- 修复 Bug #1843（点云数超出设计范围），增加无效点云过滤处理
- 修复内存申请失败时程序崩溃问题

## NeptuneViewer V0.42.0

- 增加点云序号显示功能
- 增加点云序号导出 PCD 文件
- 增加点云帧率实时计算和显示功能
- 增加日志显示按钮（默认不显示）

## NeptuneViewer V0.41.0

- 解决 Windows 版本不支持中文路径问题
- 解决 Windows 版本时间戳解析异常问题
- 解决 pcap 数据导入异常问题
- 增加告警对话框的显示配置（默认不显示）

## NeptuneViewer V0.33.1

- 修复点云帧拆帧逻辑漏洞
- 修复 UDP 数据包统计漏洞

## NeptuneViewer V0.33.0

- 修正 RT 矩阵运算逻辑
- 增加 UDP 数据包统计信息
- 使用 UTC 时间解析世界戳
- 修改点云帧 PointNum 字段数据类型
- 增加点云数据包头标志校验
- 调整 socket 缓冲区大小
- 优化点云闪烁问题

## NeptuneViewer V0.31.1

- 修复标定文件默认安装问题
- 修复点云帧计算问题
- 发布 Windows 版本

## NeptuneViewer V0.23.3

- 增加点云数据的 RT 矩阵转换
- 支持点云大小动态调整

## NeptuneViewer V0.23.2

- 修复了双回波模式下偶发的闪退现象
- Wireshark 插件支持小端字节序

## NeptuneViewer V0.23.1

- 增加激光雷达出射角度模型标定功能
- 增加对任意点测距模式的点云解析支持
- 增加双回波模式的点云解析支持
- 修改介绍界面的版本信息
- 规范版本号约束
- 更新 Wireshark 插件

