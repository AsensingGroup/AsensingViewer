set(png_libtype_args)
if (BUILD_SHARED_LIBS)
  set(png_libtype_args -DPNG_SHARED:BOOL=ON -DPNG_STATIC:BOOL=OFF)
else ()
  set(png_libtype_args -DPNG_STATIC:BOOL=ON -DPNG_SHARED:BOOL=OFF)
endif ()

set(png_args)
if (APPLE AND CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "arm64")
  list(APPEND png_args
    -DPNG_ARM_NEON:STRING=off)
endif ()

superbuild_add_project(png
  CAN_USE_SYSTEM
  DEPENDS zlib

  CMAKE_ARGS
    ${png_libtype_args}
    -DCMAKE_MACOSX_RPATH:BOOL=FALSE
    -DCMAKE_INSTALL_NAME_DIR:PATH=<INSTALL_DIR>/lib
    -DCMAKE_INSTALL_LIBDIR:PATH=lib
    -DPNG_TESTS:BOOL=OFF
    ${png_args}
    # VTK uses API that gets hidden when PNG_NO_STDIO is TRUE (default).
    -DPNG_NO_STDIO:BOOL=OFF)
