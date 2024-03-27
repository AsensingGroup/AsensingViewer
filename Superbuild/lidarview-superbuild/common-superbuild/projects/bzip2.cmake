superbuild_add_project(bzip2
  CAN_USE_SYSTEM
  CMAKE_ARGS
    -DBUILD_SHARED_LIBS:BOOL=${BUILD_SHARED_LIBS}
    -DCMAKE_INSTALL_LIBDIR:PATH=lib)

superbuild_apply_patch(bzip2 add-cmake
  "Add a CMake build system to bzip2")
