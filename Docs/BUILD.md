# 构建

## 跨平台说明

**跨平台依赖和工具**

以下工具的特定版本可能在你的操作系统包管理器中可用，也可能不可用。

 - **[必需] CMake 3.20.3+**，下载地址：<https://cmake.org/>

 - **[推荐] Ninja 1.8.2+**，下载地址：<https://ninja-build.org/>

    强烈建议在所有平台上使用 Ninja 以加快编译速度，如果你不想使用它，请确保从配置命令中删除 `-GNinja` 选项。


### Linux 特定依赖

**显卡驱动**

确保显卡驱动程序是最新的，确保安装了必要的图形软件包。

提示：如果你没有显卡，则需要安装 `mesa` 驱动程序。

**软件包/库**

 - 在 Ubuntu 18.04 及更高版本上构建，需要安装以下软件包：

    ```bash
    sudo apt-get install build-essential byacc flex freeglut3-dev libbz2-dev libffi-dev \
        libfontconfig1-dev libfreetype6-dev libnl-genl-3-dev libopengl0 libprotobuf-dev \
        libx11-dev libx11-xcb-dev libx11-xcb-dev libxcb-glx0-dev libxcb-glx0-dev \
        libxcb-icccm4-dev libxcb-image0-dev libxcb-keysyms1-dev libxcb-randr0-dev \
        libxcb-render-util0-dev libxcb-shape0-dev libxcb-shm0-dev libxcb-sync-dev \
        libxcb-util-dev libxcb-xfixes0-dev libxcb-xinerama0-dev libxcb-xkb-dev libxcb1-dev \
        libxext-dev libxext-dev libxfixes-dev libxi-dev libxkbcommon-dev libxkbcommon-dev \
        libxkbcommon-x11-dev libxkbcommon-x11-dev libxrender-dev libxt-dev pkg-config \
        protobuf-compiler zlib1g-dev
    ```

 - 如果是 Ubuntu 20，还需要额外安装：

    ```bash
    sudo apt-get install libglx-dev libibverbs1
    ```



**[可选] Qt5**

Qt5 由 Superbuild 自动构建，但是为了加快构建过程，你可以选择使用内置的二进制文件，选项参数如下：

 - 如果你的系统软件包管理器提供 Qt5 5.15.2 或以上版本（例如 Ubuntu20.04），那么使用：

    ```bash
    qt5-default qtmultimedia5-dev qtbase5-private-dev libqt5x11extras5-dev libqt5svg5-dev qttools5-dev
    ```

    并将选项参数 `-DUSE_SYSTEM_qt5=ON` 添加到 CMake 配置中。

 - 自行构建：详细说明请参考 [附加说明](#qt-build)


## Linux 系统

克隆代码仓库

```bash
cd AsensingViewer
git clone git@gitlab.ag.com:fusionposition/software/lidar/asensingviewer.git --recurse-submodules src
```

切换到构建目录

```bash
mkdir AsensingViewer/build
cd AsensingViewer/build
```

编译 Release 版本

```bash
cmake ../src/Superbuild -GNinja -DCMAKE_BUILD_TYPE=Release
cmake --build . -j
```

启动上位机软件

```bash
./install/bin/NeptuneViewer
```

编译 Debug 版本

```bash
cmake ../src/Superbuild -GNinja -DCMAKE_BUILD_TYPE=Debug -DSUPERBUILD_ALLOW_DEBUG:BOOL=ON
cmake --build . -j
```

以调试模式启动

```bash
gdb ./install/bin/NeptuneViewer
```

## Windows 系统

#### Windows 特定依赖

 - **Microsoft Visual Studio 14/17/19** 为支持新旧版本做出了巨大努力。

    如果你对向后兼容性或许可有疑虑，可能更倾向于使用 MSVC 14 (2015)，请参阅 [附加说明](#msvc15-installer)

 - **Qt 5.15.2** 你必须自己构建它并在配置时指定 Qt 的目录。

    更多信息，请参阅 [附加说明](#qt-build)。

 - **[仅用于打包]** **NSIS version 3.04 及以上版本**，下载地址：<https://nsis.sourceforge.io/Download>

#### Windows 注意事项

 - **必须使用 MSVC 构建环境命令提示符（随 MSVC 一起安装）**

    打开位置 `Windows Start Menu > Visual Studio 20XX > "x64 Native Tools Command Prompt"`

    提示：如果你不熟悉 Windows 开发，建议使用该提示（Prompt）进行构建，因为与常规 cmd.exe 提示相比，它设置了适当的构建环境。

 - **工作目录、源（src）和构建（build）路径必须短且靠近根（root）驱动器**

    这些目录的路径必须很短，因为 Windows 对文件路径和命令的最大长度有限制。

    提示：这些路径在构建过程中会出现无数次，可能会不小心达到最大限制。

  - **源目录不得位于 LidarView 源代码目录中**

    建议使用如下目录结构：

| 位置      | 工作目录   |
| -------- | ---------- |
| C:\AsensingViewer\src   | 源代码目录 |
| C:\AsensingViewer\build | 构建目录   |

注意：在配置或编译后移动这些目录，将破坏所有构建环境并需要完全重建。

#### Windows 构建说明

 - 请注意，配置命令提到了源目录中的子目录“*Superbuild*”目录，而不是源目录本身。

 - 请注意，CMake 变量，如 `Qt5_DIR` 路径参数，必须在所有平台（Unix PATH 格式）上使用正斜杠（`/`），否则 MSVC 将使用 `\\Q` 作为构建选项。

 - 如果你更改了默认的 Qt 安装路径，则必须调整配置命令。

 - 从头开始构建可能需要 20 分钟到 2 小时，具体取决于你的硬件性能。

 - 默认情况下，添加 `-j` 选项将使用机器上所有的核进行编译，你也可以使用 `-j N` 指定要使用的内核数。

操作命令：

```bash
cd AsensingViewer
git clone git@gitlab.ag.com:fusionposition/software/lidar/asensingviewer.git --recurse-submodules src
mkdir AsensingViewer\build
cd AsensingViewer\build
```

使用 MSVC2019

```bash
cmake ..\src\Superbuild -GNinja -DCMAKE_BUILD_TYPE=Release -DUSE_SYSTEM_qt5=True -DQt5_DIR="C:/Qt/5.15.2/msvc2019_64/lib/cmake/Qt5"
```

使用 MSVC2015

```bash
cmake ..\src\Superbuild -GNinja -DCMAKE_BUILD_TYPE=Release -DUSE_SYSTEM_qt5=True -DQt5_DIR="C:/Qt/5.15.2/msvc2015_64/lib/cmake/Qt5"
```

开始构建

```bash
cmake --build . -j
```

启动程序

```bash
install\bin\NeptuneViewer
```

## 增量构建说明

如果你只修改了 AsensingViewer 源代码，通常会希望增量构建，以缩短构建时间。命令如下：

```bash
cd AsensingViewer\build\lidarview-superbuild\common-superbuild\lidarview\build
cmake --build . -j --target install
```


## 打包说明


### Linux

通过 CMake 配置激活 tests 的构建：

```bash
cd AsensingViewer/build
cmake . -DBUILD_TESTING=True
```

使用新配置构建：

```bash
cmake --build . -j
```

使用 cpack 打包：

```bash
ctest
```

### Windows

请使用预先配置好的 Windows 10 虚拟机进行编译、打包。

- 源码位于“桌面 -> AsensingViewer-main”，构建目录位于“C:\builds”；
- 打开 Qt Creator，可以更新工程 CMake 配置，编译依赖库；
- 打开“VS 2017 的 x64 本机工具命令提示”，切换到“C:\builds\lidarview-superbuild\common-superbuild\lidarview\build”，执行如下命令；
  ```bash
  cmake --build . -j --target install
  ```
- 编译后的可执行文件位于“C:\builds\install\bin”；
- 使用 Inno Setup Compiler 软件打开桌面的 pack.iss 文件，修改版本信息，点击工具栏中的“Compile”按钮开始打包程序。
- 等待打包完成，将在桌面生成一个 NeptuneViewer.exe 可执行文件。

