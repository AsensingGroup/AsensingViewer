if (BUILD_SHARED_LIBS)
  set(libxml2_shared_args --enable-shared --disable-static)
else ()
  set(libxml2_shared_args --disable-shared --enable-static)
endif ()

superbuild_add_project(libxml2
  CAN_USE_SYSTEM
  CONFIGURE_COMMAND
    <SOURCE_DIR>/configure
      --prefix=<INSTALL_DIR>
      --without-python
      ${libxml2_shared_args}
  BUILD_COMMAND
    $(MAKE)
  INSTALL_COMMAND
    $(MAKE) install
  BUILD_IN_SOURCE 1)

if (APPLE AND CMAKE_OSX_DEPLOYMENT_TARGET)
  superbuild_append_flags(c_flags
    "-mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET}"
    PROJECT_ONLY)
endif ()
