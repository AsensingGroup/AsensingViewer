set(fontconfig_process_environment)
if (BUILD_SHARED_LIBS)
  set(fontconfig_shared_args --enable-shared --disable-static)
else ()
  set(fontconfig_shared_args --disable-shared --enable-static)
endif ()

set(fontconfig_depends)
if (NOT APPLE)
  list(APPEND fontconfig_depends
    utillinux)
endif ()
if (APPLE OR UNIX)
  list(APPEND fontconfig_depends
    pkgconf)

  if (pkgconf_enabled)
    list(APPEND fontconfig_process_environment
      PKG_CONFIG "${superbuild_pkgconf} --keep-system-cflags --keep-system-libs")
  endif ()
endif ()


superbuild_add_project(fontconfig
  DEPENDS freetype libxml2 png gperf ${fontconfig_depends}
  BUILD_IN_SOURCE 1
  CONFIGURE_COMMAND
    <SOURCE_DIR>/configure
      --prefix=<INSTALL_DIR>
      --disable-docs
      --enable-libxml2
      ${fontconfig_shared_args}

      # Use the system configuration and cachedirs.
      --sysconfdir=/etc
      --localstatedir=/var
  BUILD_COMMAND
    $(MAKE)
  INSTALL_COMMAND
    $(MAKE) install-exec install-pkgconfigDATA installdirs
  PROCESS_ENVIRONMENT
    ${fontconfig_process_environment})

superbuild_project_add_step(install-headers
  COMMAND   make install-fontconfigincludeHEADERS
  DEPENDEES install
  COMMENT   ""
  WORKING_DIRECTORY <SOURCE_DIR>/fontconfig)
