superbuild_add_project(tbb
  CAN_USE_SYSTEM
  CMAKE_ARGS
    -DCMAKE_INSTALL_NAME_DIR:PATH=<INSTALL_DIR>/lib
    -DCMAKE_INSTALL_LIBDIR:STRING=lib
    -DTBB_TEST:BOOL=OFF)

superbuild_add_extra_cmake_args(
  -DTBB_ROOT:PATH=<INSTALL_DIR>)
