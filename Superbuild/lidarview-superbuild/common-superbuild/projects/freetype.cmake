set(freetype_install_rpath)
if (UNIX AND NOT APPLE)
  list(APPEND freetype_install_rpath
    -DCMAKE_INSTALL_RPATH:STRING=<INSTALL_DIR>/lib)
endif ()

if (zlib_enabled)
  set(freetype_zlib_disable OFF)
else ()
  set(freetype_zlib_disable ON)
endif ()

if (png_enabled)
  set(freetype_png_disable OFF)
else ()
  set(freetype_png_disable ON)
endif ()

superbuild_add_project(freetype
  CAN_USE_SYSTEM
  DEPENDS zlib png
  CMAKE_ARGS
    -DBUILD_SHARED_LIBS:BOOL=${BUILD_SHARED_LIBS}
    -DCMAKE_INSTALL_LIBDIR:STRING=lib
    -DCMAKE_INSTALL_NAME_DIR:PATH=<INSTALL_DIR>/lib
    ${freetype_install_rpath}
    -DFT_DISABLE_ZLIB:BOOL=${freetype_zlib_disable}
    -DFT_REQUIRE_ZLIB:BOOL=${zlib_enabled}
    -DFT_DISABLE_BZIP2:BOOL=ON
    -DFT_DISABLE_PNG:BOOL=${freetype_png_disable}
    -DFT_REQUIRE_PNG:BOOL=${png_enabled}
    -DFT_DISABLE_HARFBUZZ:BOOL=ON
    -DFT_DISABLE_BROTLI:BOOL=ON)

# Upstream commit 8a33164dad3e081dea219de6f19d8ba697fba1c2
# https://gitlab.freedesktop.org/freetype/freetype/-/commit/8a33164dad3e081dea219de6f19d8ba697fba1c2
superbuild_apply_patch(freetype pkgconfig-cmake-fix-prep
  "fix pkgconfig generation in CMake (prep)")

# Upstream commit 385345037e04f9ee6ffc8b14318f1a079520c41d
# https://gitlab.freedesktop.org/freetype/freetype/-/commit/385345037e04f9ee6ffc8b14318f1a079520c41d
superbuild_apply_patch(freetype pkgconfig-cmake-fix
  "fix pkgconfig generation in CMake")
