if (BUILD_SHARED_LIBS)
  set(szip_shared_args --enable-shared --disable-static)
else ()
  set(szip_shared_args --disable-shared --enable-static)
endif ()

superbuild_add_project(szip
  CONFIGURE_COMMAND
    <SOURCE_DIR>/configure
      ${szip_shared_args}
      --disable-encoding
      --prefix=<INSTALL_DIR>
  BUILD_COMMAND
    $(MAKE)
  INSTALL_COMMAND
    make install)

superbuild_add_extra_cmake_args(
  -DSZIP_LIBRARY:FILEPATH=<INSTALL_DIR>/lib/${CMAKE_SHARED_LIBRARY_PREFIX}sz${CMAKE_SHARED_LIBRARY_SUFFIX}
  -DSZIP_INCLUDE_DIR:FILEPATH=<INSTALL_DIR>/include)
