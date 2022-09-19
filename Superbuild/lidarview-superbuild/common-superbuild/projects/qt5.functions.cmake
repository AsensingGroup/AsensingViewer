function (superbuild_install_qt5_plugin_paths output)
  if (USE_SYSTEM_qt5 AND UNIX)
    set(qt5_no_package_paths)
    if (APPLE)
      list(APPEND qt5_no_package_paths
        "/usr/lib/cmake/Qt5")
    elseif (UNIX)
      list(APPEND qt5_no_package_paths
        "/lib/cmake/Qt5"
        "/lib64/cmake/Qt5"
        "/usr/lib/cmake/Qt5"
        "/usr/lib64/cmake/Qt5"
        "/usr/local/lib/cmake/Qt5"
        "/usr/local/lib64/cmake/Qt5")
    endif ()

    # The package will not be installing Qt5 since it is provided by the
    # system; do not install the plugins.
    if (Qt5_DIR IN_LIST qt5_no_package_paths)
      return ()
    endif ()
  endif ()

  if (USE_SYSTEM_qt5)
    if (NOT Qt5_DIR)
      message(FATAL_ERROR
        "Installing plugins from a system Qt5 requires `Qt5_DIR` to be set.")
    endif ()

    set(qt5_base_libdir "${Qt5_DIR}/../..")
    if (EXISTS "${qt5_base_libdir}/qt5")
      # This is the layout for Linux distributions.
      set(qt5_base_libdir "${qt5_base_libdir}/qt5")
    elseif (EXISTS "${qt5_base_libdir}/../plugins")
      # This is the layout for Qt binaries.
      set(qt5_base_libdir "${qt5_base_libdir}/..")
    elseif (EXISTS "${qt5_base_libdir}/../libexec/qt5")
      # This is the layout for MacPorts.
      set(qt5_base_libdir "${qt5_base_libdir}/../libexec/qt5")
    endif ()
  else ()
    set(qt5_base_libdir "${superbuild_install_location}")
  endif ()

  set(qt5_plugin_path "${qt5_base_libdir}/plugins")
  if (WIN32)
    set(qt5_plugin_ext ".dll")
  elseif (APPLE)
    set(qt5_plugin_ext ".dylib")
  elseif (UNIX)
    set(qt5_plugin_ext ".so")
  else ()
    message(FATAL_ERROR
      "Unknown Qt5 plugin path for this platform.")
  endif ()

  set(plugin_paths)
  foreach (plugin IN LISTS ARGN)
    set(plugin_path "${qt5_plugin_path}/${plugin}${qt5_plugin_ext}")
    if (NOT EXISTS "${plugin_path}")
      message(FATAL_ERROR
        "Unable to find the ${plugin} plugin from Qt5 under ${qt5_plugin_path}.")
    endif ()

    list(APPEND plugin_paths
      "${plugin_path}")
  endforeach ()

  set("${output}" "${plugin_paths}" PARENT_SCOPE)
endfunction ()
