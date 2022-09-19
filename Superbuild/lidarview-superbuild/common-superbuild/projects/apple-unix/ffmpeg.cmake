if (BUILD_SHARED_LIBS)
  set(ffmpeg_shared_args --enable-shared --disable-static)
else ()
  set(ffmpeg_shared_args --disable-shared --enable-static)
endif ()

set(ffmpeg_c_flags "${superbuild_c_flags}")
if (APPLE AND CMAKE_OSX_SYSROOT)
  string(APPEND ffmpeg_c_flags " --sysroot=${CMAKE_OSX_SYSROOT}")
endif ()
set(ffmpeg_ld_flags "${superbuild_ld_flags}")
if (APPLE AND CMAKE_OSX_DEPLOYMENT_TARGET)
  string(APPEND ffmpeg_ld_flags " -isysroot ${CMAKE_OSX_SYSROOT} -mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET}")
endif ()

superbuild_add_project(ffmpeg
  DEPENDS zlib pkgconf
  CONFIGURE_COMMAND
    <SOURCE_DIR>/configure
      --prefix=<INSTALL_DIR>
      --disable-avdevice
      --disable-bzlib
      --disable-decoders
      --disable-doc
      --disable-ffplay
      --disable-ffprobe
      --disable-network
      --disable-vaapi
      --disable-vdpau
      --disable-x86asm
      --pkg-config=${superbuild_pkgconf}
      ${ffmpeg_shared_args}
      "--extra-cflags=${ffmpeg_c_flags}"
      "--extra-ldflags=${ffmpeg_ld_flags}"
  BUILD_COMMAND
    $(MAKE)
  INSTALL_COMMAND
    make install
  BUILD_IN_SOURCE 1)

superbuild_apply_patch(ffmpeg swscalex86-yuv2yuvX-revert-conversion-to-assembly
  "revert assembly port of yuv2yuvX function")
