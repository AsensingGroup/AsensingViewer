# Bundling scripts for LidarView - Unix Specific

# Sanitize check
if(NOT superbuild_python_version)
  message(FATAL_ERROR "superbuild_python_version not set")
endif()

if(NOT SOFTWARE_NAME )
  message(FATAL_ERROR "SOFTWARE_NAME branding not set")
endif()

if(NOT LV_INSTALL_RESOURCE_DIR )
  message(FATAL_ERROR "LV_INSTALL_RESOURCE_DIR branding not set")
endif()

# Install Executables # WIP WHAT IS `executable_path`
foreach (executable IN LISTS lidarview_executables)
  superbuild_unix_install_program(
    "${executable_path}/${executable}"
    "lib"
    SEARCH_DIRECTORIES  "${library_paths}"
    INCLUDE_REGEXES     ${include_regexes} ".*libz\.so\.1\.2\.11.*" # LIBZ dependency patch
    EXCLUDE_REGEXES     ${exclude_regexes} ".*libz.*" # LIBZ dependency patch
  )
endforeach ()

# Install Lidarview Plugins
foreach (lidarview_plugin IN LISTS lidarview_plugins)
  superbuild_unix_install_plugin(
    "${superbuild_install_location}/${LV_PLUGIN_INSTALL_SUBDIRECTORY}/${lidarview_plugin}/${lidarview_plugin}.so"
    "lib"
    "${LV_PLUGIN_INSTALL_SUBDIRECTORY}/${lidarview_plugin}"
    LOADER_PATHS  "${library_paths}" # Means "Like SEARCH_DIRECTORIES, but also used to replace RPATHS in _resolve_rpath()"
    INCLUDE_REGEXES ${include_regexes}
    EXCLUDE_REGEXES ${exclude_regexes}
    LOCATION  "${LV_PLUGIN_INSTALL_SUBDIRECTORY}/${lidarview_plugin}/")
endforeach ()

# Install Lidarview Plugins xmls
file(GLOB xml_files "${superbuild_install_location}/${LV_PLUGIN_INSTALL_SUBDIRECTORY}/*.xml")
install(
  FILES       ${xml_files}
  DESTINATION "${LV_PLUGIN_INSTALL_SUBDIRECTORY}"
)

# Install Paraview Plugins
foreach (paraview_plugin IN LISTS paraview_plugins)
  superbuild_unix_install_plugin(
    "${superbuild_install_location}/${LV_INSTALL_PV_PLUGIN_SUBDIR}/${paraview_plugin}/${paraview_plugin}.so"
    "lib"
    "${LV_INSTALL_PV_PLUGIN_SUBDIR}/${paraview_plugin}"
    LOADER_PATHS  "${library_paths}"
    INCLUDE_REGEXES ${include_regexes}
    EXCLUDE_REGEXES ${exclude_regexes}
    LOCATION  "${LV_INSTALL_PV_PLUGIN_SUBDIR}/${paraview_plugin}/")
endforeach ()

# Install Paraview Plugins xmls
file(GLOB xml_files "${superbuild_install_location}/${LV_INSTALL_PV_PLUGIN_SUBDIR}/*.xml")
install(
  FILES       ${xml_files}
  DESTINATION "${LV_INSTALL_PV_PLUGIN_SUBDIR}"
)

# Install Configuration file
install(
  FILES       "${executable_path}/${SOFTWARE_NAME}.conf"
  DESTINATION "bin"
)

# Install Python
if (python3_enabled)
  file(GLOB egg_dirs
    "${superbuild_install_location}/lib/python${superbuild_python_version}/site-packages/*.egg/")
  if (python3_built_by_superbuild)
    include(python3.functions)
    superbuild_install_superbuild_python3(
      LIBSUFFIX "/python${superbuild_python_version}")
  endif ()

  # Add extra paths to MODULE_DIRECTORIES here (.../local/lib/python${superbuild_python_version}/dist-packages)
  # is a workaround to an issue when building against system python.  When we move to
  # Python3, we should make sure all the python modules get installed to the same
  # location to begin with.
  #
  # Related issue: https://gitlab.kitware.com/paraview/paraview-superbuild/-/issues/120
  superbuild_unix_install_python(
    LIBDIR              "lib"
    MODULES             ${lidarview_modules_python}
    INCLUDE_REGEXES     ${include_regexes}
    EXCLUDE_REGEXES     ${exclude_regexes}
    MODULE_DIRECTORIES  "${superbuild_install_location}/lib/python${superbuild_python_version}/site-packages"
                        ${egg_dirs}
    LOADER_PATHS        "${library_paths}")
endif ()

# THIRDPARTY

# Install Qt Plugins if any
foreach (qt5_plugin_path IN LISTS qt5_plugin_paths)
  get_filename_component(qt5_plugin_group "${qt5_plugin_path}" DIRECTORY)
  get_filename_component(qt5_plugin_group "${qt5_plugin_group}" NAME)

  # Qt expects its libraries to be in `lib/`, not beside, so install them as
  # modules.
  superbuild_unix_install_module("${qt5_plugin_path}"
    "lib"
    "plugins/${qt5_plugin_group}/"
    LOADER_PATHS    "${library_paths}"
    INCLUDE_REGEXES ${include_regexes}
    EXCLUDE_REGEXES ${exclude_regexes})
endforeach ()

# Install Empty Qt Conf #WIP paraview has a paraview.conf ?
if (qt5_enabled AND qt5_plugin_paths)
  file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/qt.conf" "[Paths]\nPrefix = ..\n")
  install(
    FILES       "${CMAKE_CURRENT_BINARY_DIR}/qt.conf"
    DESTINATION "bin")
endif ()
