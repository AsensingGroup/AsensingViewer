# Bundling scripts for LidarView - Win32 Specific

# Sanitize check
if(NOT SOFTWARE_NAME)
  message(FATAL_ERROR "SOFTWARE_NAME not set")
endif()
if(NOT superbuild_python_version)
  message(FATAL_ERROR "superbuild_python_version not set")
endif()
if(NOT LV_VERSION_FULL)
  message(FATAL_ERROR "LV_VERSION_FULL not set")
endif()
if(NOT SOFTWARE_ICON_PATH)
  message(FATAL_ERROR "SOFTWARE_ICON_PATH not set")
endif()

# Set NSIS install specific stuff.
if (CMAKE_CL_64)
  # Change default installation root path for Windows x64.
  set(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES64")
endif ()

set(CPACK_NSIS_HELP_LINK "https://www.paraview.org/lidarview/")
set(${SOFTWARE_NAME}_description "${SOFTWARE_NAME} ${LV_VERSION_FULL}")
set(CPACK_NSIS_MUI_ICON "${SOFTWARE_ICON_PATH}")

# Install Executables
foreach (executable IN LISTS lidarview_executables)
  if (DEFINED "${executable}_description")
    list(APPEND CPACK_NSIS_MENU_LINKS
      "bin/${executable}.exe" "${${executable}_description}")
  endif ()

  superbuild_windows_install_program(
  "${executable}" #NOTE: This macro gives no choice in executable origin location, must be in /install/bin
  "bin"
    SEARCH_DIRECTORIES "${library_paths}"
    INCLUDE_REGEXES     ${include_regexes}
    EXCLUDE_REGEXES     ${exclude_regexes}
  )
endforeach()

# Install additional modules #if ever needed use
# foreach _superbuild_windows_install_binary( ... ) with ${lidarview_modules} with DESTINATION "bin"

# Install Lidarview Plugins
foreach (lidarview_plugin IN LISTS lidarview_plugins)
  superbuild_windows_install_plugin(
    "${superbuild_install_location}/${LV_PLUGIN_INSTALL_SUBDIRECTORY}/${lidarview_plugin}/${lidarview_plugin}.dll"
    "bin" #where to put libraries #wip make this dir generic
    "${LV_PLUGIN_INSTALL_SUBDIRECTORY}/${lidarview_plugin}"
    SEARCH_DIRECTORIES "${library_paths}" #Same as LOADER_PATHS on UNIX
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
  superbuild_windows_install_plugin(
    "${superbuild_install_location}/${LV_INSTALL_PV_PLUGIN_SUBDIR}/${paraview_plugin}/${paraview_plugin}.dll" #wip make extension generic
    "bin" #where to put libraries #wip make this dir generic
    "${LV_INSTALL_PV_PLUGIN_SUBDIR}/${paraview_plugin}" #where to put plugin
    SEARCH_DIRECTORIES "${library_paths}" #Same as LOADER_PATHS on UNIX
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
  FILES       "${superbuild_install_location}/bin/${SOFTWARE_NAME}.conf" # wip sanitaize ? #wip not crossplatform USE LV_INSTALL_RUNTIME_DIR
  DESTINATION "bin"
)

# Install Python
# WIP This may be useful, to be put early in this file
#if (python3_enabled)
#  if (NOT python3_built_by_superbuild)
#    list(APPEND exclude_regexes
#        ".*python3[0-9]+.dll")
#  endif()
#endif ()

if (python3_enabled)
  if (python3_built_by_superbuild)
    include(python3.functions)
    superbuild_install_superbuild_python3()
  endif ()

  superbuild_windows_install_python(
    MODULES ${lidarview_modules_python}
    MODULE_DIRECTORIES  "${superbuild_install_location}/Python/Lib/site-packages"
                        "${superbuild_install_location}/bin/Lib/site-packages"
                        "${superbuild_install_location}/lib/site-packages"
                        "${superbuild_install_location}/lib/python${superbuild_python_version}/site-packages"
                        "${superbuild_install_location}/lib/paraview-${paraview_version_major}.${paraview_version_minor}/site-packages"
    SEARCH_DIRECTORIES  "${superbuild_install_location}/lib"
                        "${superbuild_install_location}/bin"
                        "${superbuild_install_location}/Python"
                        "${superbuild_install_location}/Python/Lib/site-packages/pywin32_system32"
                        ${library_paths}
    EXCLUDE_REGEXES     ${exclude_regexes})

  if (pywin32_built_by_superbuild)
      install(
        DIRECTORY   "${superbuild_install_location}/Python/Lib/site-packages/win32"
        DESTINATION "bin/Lib/site-packages"
        COMPONENT   "superbuild")
      install(
        DIRECTORY   "${superbuild_install_location}/Python/Lib/site-packages/pywin32_system32"
        DESTINATION "bin/Lib/site-packages"
        COMPONENT   "superbuild")
      install(
        FILES       "${superbuild_install_location}/Python/Lib/site-packages/pywin32.pth"
                    "${superbuild_install_location}/Python/Lib/site-packages/pywin32.version.txt"
        DESTINATION "bin/Lib/site-packages"
        COMPONENT   "superbuild")
  endif ()
endif ()

# THIRDPARTY

# Install Qt Plugins if any
foreach (qt5_plugin_path IN LISTS qt5_plugin_paths)
  get_filename_component(qt5_plugin_group "${qt5_plugin_path}" DIRECTORY)
  get_filename_component(qt5_plugin_group "${qt5_plugin_group}" NAME)

  superbuild_windows_install_plugin(
    "${qt5_plugin_path}"
    "bin"
    "bin/${qt5_plugin_group}"
    SEARCH_DIRECTORIES "${library_paths}")
endforeach ()

if (qt5_enabled)
  foreach (qt5_opengl_lib IN ITEMS opengl32sw libEGL libGLESv2)
    superbuild_windows_install_plugin(
      "${Qt5_DIR}/../../../bin/${qt5_opengl_lib}.dll"
      "bin"
      "bin"
      SEARCH_DIRECTORIES "${library_paths}")
  endforeach ()
endif ()


