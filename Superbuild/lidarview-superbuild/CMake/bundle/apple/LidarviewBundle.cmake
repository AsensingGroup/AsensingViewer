# Bundling scripts for LidarView - Apple Specific

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

# the variable lidarview_appname:
# - must be a valid dirname: will be a directory at the top of the .dmg
# - is visible in the macOS GUI when opening the .dmg
# - MUST end with .app (else its tree is not considered as an app by macOS)
set(lidarview_appname "${SOFTWARE_NAME}.app")

set(lidarview_additional_libraries)
# Not needed anymore as Slam is now a standalone plugin, kept for future reference
#if(LIDARVIEW_BUILD_SLAM)
#  LIST(APPEND lidarview_additional_libraries "${superbuild_install_location}/bin/${lidarview_appname}/Contents/Libraries/libLidarSlam.dylib")
#endif()

# MUST FIND PLUGINS WIP SEE "set(paraview_plugin_paths)" part in pv-sb #WIP NOT A .dylib ???
set(all_plugin_paths)
foreach (lidarview_plugin IN LISTS lidarview_plugins)
  list(APPEND all_plugin_paths "${superbuild_install_location}/${LV_PLUGIN_INSTALL_SUBDIRECTORY}/${lidarview_plugin}/${lidarview_plugin}.so")
endforeach ()
foreach (paraview_plugin IN LISTS paraview_plugins)
  list(APPEND all_plugin_paths "${superbuild_install_location}/${LV_INSTALL_PV_PLUGIN_SUBDIR}/${paraview_plugin}/${paraview_plugin}.so")
endforeach ()

superbuild_apple_create_app(
  "\${CMAKE_INSTALL_PREFIX}"
  "${lidarview_appname}"
  "${superbuild_install_location}/Applications/${lidarview_appname}/Contents/MacOS/${SOFTWARE_NAME}"
  CLEAN
  PLUGINS ${all_plugin_paths} #${superbuild_install_location}/${LV_PLUGIN_INSTALL_SUBDIRECTORY} #WIP TO CHECK
  SEARCH_DIRECTORIES
    "${superbuild_install_location}/bin/${lidarview_appname}/Contents/Libraries" #WIP probably not the best idea to install them here
    "${superbuild_install_location}/lib"
  ADDITIONAL_LIBRARIES ${lidarview_additional_libraries}
)

# Install Lidarview Plugins xmls
file(GLOB xml_files "${superbuild_install_location}/${LV_PLUGIN_INSTALL_SUBDIRECTORY}/*.xml")
install(
  FILES       ${xml_files}
  DESTINATION "${lidarview_appname}/Contents/Plugins"
  COMPONENT   superbuild)
# Install Paraview Plugins xmls
file(GLOB xml_files "${superbuild_install_location}/${LV_INSTALL_PV_PLUGIN_SUBDIR}/*.xml")
install(
  FILES       ${xml_files}
  DESTINATION "${lidarview_appname}/Contents/Plugins"
  COMPONENT   superbuild)

# Install Configuration file
install(
  FILES       "${superbuild_install_location}/bin/${SOFTWARE_NAME}.conf"
  DESTINATION "${lidarview_appname}/Contents/MacOS/"
  COMPONENT   superbuild)

# Bundle Icon
install(
  FILES       "${superbuild_install_location}/${LV_INSTALL_RESOURCE_DIR}/logo.icns"
  DESTINATION "${lidarview_appname}/Contents/Resources"
  COMPONENT   superbuild)

# Info.plist
install(
  FILES       "${superbuild_install_location}/Applications/${SOFTWARE_NAME}.app/Contents/Info.plist"
  DESTINATION "${lidarview_appname}/Contents"
  COMPONENT   superbuild)

# Remove "LidarView" from the list since we just installed it above.
list(REMOVE_ITEM lidarview_executables
  ${SOFTWARE_NAME})

# Install Executables
# WIP WE DISABLE THIS FOR NOW
#foreach (executable IN LISTS lidarview_executables)
#  superbuild_apple_install_utility(
#    "\${CMAKE_INSTALL_PREFIX}"  #DEST
#    "${lidarview_appname}" # Same name as application creation
#    "${superbuild_install_location}/bin/${executable}"
#    SEARCH_DIRECTORIES "${library_paths}")
#endforeach ()

if (qt5_enabled)
  file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/qt.conf" "[Paths]\nPlugins = Plugins\n")
  install(
    FILES       "${CMAKE_CURRENT_BINARY_DIR}/qt.conf"
    DESTINATION "${lidarview_appname}/Contents/Resources" #WIP PV DOES THIS BUT LV HAS Application/ ? WIP
    COMPONENT   superbuild)
endif ()

if (python3_enabled)
  # install python modules
  if (python3_built_by_superbuild)
    include(python3.functions)
    superbuild_install_superbuild_python3(BUNDLE "${lidarview_appname}")
  endif()

 file(GLOB egg_dirs
    "${superbuild_install_location}/lib/python${superbuild_python_version}/site-packages/*.egg/")
  superbuild_apple_install_python(
  "\${CMAKE_INSTALL_PREFIX}"
  "${lidarview_appname}"
  MODULES ${lidarview_modules_python}
  MODULE_DIRECTORIES
          "${superbuild_install_location}/Applications/paraview.app/Contents/Python"
          "${superbuild_install_location}/lib/python${superbuild_python_version}/site-packages"
          ${egg_dirs}
          "${superbuild_install_location}/Applications/${lidarview_appname}/Contents/Python"
  SEARCH_DIRECTORIES
          "${superbuild_install_location}/Applications/paraview.app/Contents/Libraries"
          "${superbuild_install_location}/lib"
          "${superbuild_install_location}/Applications/${lidarview_appname}/Contents/Libraries"
  )
endif ()

# Configure CMakeDMGSetup.scpt to replace the app name in the script.
# THIS REQUIRES `lidarview_appname`
configure_file(
  "${CMAKE_CURRENT_LIST_DIR}/files/CMakeDMGSetup.scpt.in"
  "${CMAKE_CURRENT_BINARY_DIR}/CMakeDMGSetup.scpt"
  @ONLY)

#WIP TO CHECK
#[[
# For some reason these .so files are not processed by the command
# superbuild_apple_install_python above, so we have to specify them manually
# it could be that I failed to find the correct name(s) to add in parameter
# "MODULE" but I do not think so because there are 86 such files, and because
# they seem to be part of vtk which is already specified like that in ParaView
file(GLOB missing_python_so "${superbuild_install_location}/bin/${lidarview_appname}/Contents/Libraries/vtk*Python.so")
foreach (python_so ${missing_python_so})
  superbuild_apple_install_module(
    "\${CMAKE_INSTALL_PREFIX}"
    "${lidarview_appname}"
    "${python_so}"
    "Contents/Libraries") # destination path inside bundle
endforeach()

# My understanding is that these module are not processed automatically
# by superbuild_apple_create_app because there is no path leading to
# them in binary LidarView or in any of its .dylib dependencies
foreach (module ${lidarview_modules})
  superbuild_apple_install_module(
    "\${CMAKE_INSTALL_PREFIX}"
    "${lidarview_appname}"
    "${superbuild_install_location}/bin/${lidarview_appname}/Contents/Libraries/${module}"
    "Contents/Libraries") # destination path inside bundle
endforeach()

]]

set(CPACK_DMG_BACKGROUND_IMAGE      "${CMAKE_CURRENT_LIST_DIR}/files/CMakeDMGBackground.tif")
set(CPACK_DMG_DS_STORE_SETUP_SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/CMakeDMGSetup.scpt")

# WIP Prevent Symlink Issue
set(CPACK_DMG_DISABLE_APPLICATIONS_SYMLINK ON)

message(STATUS "qt5_plugin_paths is ${qt5_plugin_paths}")
foreach (qt5_plugin_path IN LISTS qt5_plugin_paths)
  get_filename_component(qt5_plugin_group "${qt5_plugin_path}" DIRECTORY)
  get_filename_component(qt5_plugin_group "${qt5_plugin_group}" NAME)

  superbuild_apple_install_module(
    "\${CMAKE_INSTALL_PREFIX}"
    "${lidarview_appname}"
    "${qt5_plugin_path}"
    "Contents/Plugins/${qt5_plugin_group}"
    SEARCH_DIRECTORIES  "${library_paths}") #WIP ??"${superbuild_install_location}/lib"
endforeach ()

#LV already copies its calib, files but this is done to get VV specific ones too
install(DIRECTORY "${superbuild_install_location}/${LV_INSTALL_RESOURCE_DIR}"
  DESTINATION "${LV_INSTALL_RESOURCE_DIR}/.." #WIP PV DOES THIS BUT LV HAS Application/ ? WIP
  COMPONENT superbuild)
