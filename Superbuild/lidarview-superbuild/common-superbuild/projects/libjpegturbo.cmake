set(libjpegturbo_enable_static ON)
if (BUILD_SHARED_LIBS)
  set(libjpegturbo_enable_static OFF)
endif ()

superbuild_add_project(libjpegturbo
  CMAKE_ARGS
    -DENABLE_SHARED:BOOL=${BUILD_SHARED_LIBS}
    -DENABLE_STATIC:BOOL=${libjpegturbo_enable_static}
    -DBUILD_TESTING:BOOL=OFF
    -DCMAKE_INSTALL_NAME_DIR:PATH=<INSTALL_DIR>/lib
    -DWITH_SIMD:BOOL=OFF
    -DWITH_CRT_DLL:BOOL=ON
    -DCMAKE_INSTALL_LIBDIR:STRING=lib)

if (WIN32)
  superbuild_apply_patch(libjpegturbo msvc-default-lib-flags
    "remove problematic default flags for MSVC")
endif ()
