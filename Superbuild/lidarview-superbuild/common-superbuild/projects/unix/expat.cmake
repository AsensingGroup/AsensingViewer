if(BUILD_SHARED_LIBS)
  set(expat_args_shared --enable-shared --disable-static)
else()
  set(expat_args_shared --disable-shared --enable-static)
endif()

superbuild_add_project(expat
  CAN_USE_SYSTEM
  CONFIGURE_COMMAND
    <SOURCE_DIR>/configure
    --prefix=<INSTALL_DIR>
    ${expat_args_shared}
    --without-docbook
  BUILD_COMMAND
    $(MAKE)
  INSTALL_COMMAND
    make install
  BUILD_IN_SOURCE 1)
