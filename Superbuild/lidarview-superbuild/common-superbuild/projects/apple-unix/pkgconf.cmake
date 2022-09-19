superbuild_add_project(pkgconf
  CAN_USE_SYSTEM
  CONFIGURE_COMMAND
    <SOURCE_DIR>/configure
      --prefix=<INSTALL_DIR>
  BUILD_COMMAND
    $(MAKE)
  INSTALL_COMMAND
    make install
  BUILD_IN_SOURCE 1)

set(superbuild_pkgconf "<INSTALL_DIR>/bin/pkgconf")
