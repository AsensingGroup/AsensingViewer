# Crossplatform Bundling scripts for LidarView
# Remember to use following variables:
#   superbuild_install_location
#   superbuild_source_directory
#   ${project}_enabled
#   USE_SYSTEM_${project}

# Sanitize checks
if(NOT paraview_version )
  message(FATAL_ERROR "paraview_version not set")
endif()
if(NOT LV_BUILD_PLATFORM )
  message(FATAL_ERROR "LV_BUILD_PLATFORM not set")
endif()
if(NOT SOFTWARE_NAME )
  message(FATAL_ERROR "SOFTWARE_NAME branding not set")
endif()
if(NOT SOFTWARE_VENDOR )
  message(FATAL_ERROR "SOFTWARE_VENDOR branding not set")
endif()

# Get Architecture Relative Path Variables from LVCore
include(${LidarViewSuperBuild_LVCORE_CMAKE_DIR}/SetupOutputDirs.cmake)

# Set Packaging Absolute Origin/Destination paths
set(library_paths   "${superbuild_install_location}/${LV_INSTALL_LIBRARY_DIR}") # List
set(executable_path "${superbuild_install_location}/${LV_INSTALL_RUNTIME_DIR}")
set(share_path      "${superbuild_install_location}/${LV_INSTALL_RESOURCE_DIR}")

# Determine Version
include("${LidarViewSuperBuild_LVCORE_CMAKE_DIR}/Git.cmake")
include("${LidarViewSuperBuild_LVCORE_CMAKE_DIR}/ParaViewDetermineVersion.cmake")
# Sets fallback LV_VERSION_{MAJOR,MINOR,PATCH} this represents the last major version only

set(LV_VERSION_FILE ${LidarViewSuperBuild_SOURCE_DIR}/../version.txt)
file(STRINGS "${LV_VERSION_FILE}" version_txt)
extract_version_components("${version_txt}" "LV")
# Try setting LV_VERSION_{MAJOR,MINOR,PATCH} using git
determine_version(${LidarViewSuperBuild_SOURCE_DIR} ${GIT_EXECUTABLE} "LV")
if( version_txt STREQUAL LV_VERSION_FULL )
  message(WARNING "Could not determine version through git, using fallback Major Version number ${LV_VERSION_FULL} from ${LV_VERSION_FILE}")
endif()

# Sets GD_YEAR, GD_MONTH, GD_DAY
include(${LidarViewSuperBuild_SOURCE_DIR}/lidarview-superbuild/Projects/getdate.cmake)
GET_DATE()
set(PACKAGE_TIMESTAMP "${GD_YEAR}${GD_MONTH}${GD_DAY}")

# Set Package Branding
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${SOFTWARE_NAME}")
set(CPACK_PACKAGE_NAME   "${SOFTWARE_NAME}")
set(CPACK_PACKAGE_VENDOR "${SOFTWARE_VENDOR}")

# Set Package Version
#set(CPACK_COMPONENT_LIDARVIEW_DISPLAY_NAME ${SOFTWARE_NAME}) # We do not use CPackComponent
set(CPACK_PACKAGE_VERSION_MAJOR ${LV_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${LV_VERSION_MINOR})
if (NOT LV_VERSION_IS_RELEASE)
  set(CPACK_PACKAGE_VERSION_PATCH ${LV_VERSION_PATCH}-${LV_VERSION_PATCH_EXTRA})
else()
  set(CPACK_PACKAGE_VERSION_PATCH ${LV_VERSION_PATCH})
endif()

# Set Package Name
if (NOT LV_VERSION_IS_RELEASE)
  set(CPACK_PACKAGE_FILE_NAME
      "${CPACK_PACKAGE_NAME}-${LV_VERSION_FULL}-${PACKAGE_TIMESTAMP}-${LV_BUILD_PLATFORM}")
else()
  set(CPACK_PACKAGE_FILE_NAME
      "${CPACK_PACKAGE_NAME}-${LV_VERSION_FULL}-${LV_BUILD_PLATFORM}")
endif()
message(STATUS "Bundled package name will be: ${CPACK_PACKAGE_FILE_NAME}" )

# Set Executables
set(lidarview_executables
    "${SOFTWARE_NAME}"
)

# Set Modules
set(lidarview_modules)

# Set Python Modules
set(lidarview_modules_python
  paraview
  vtk
  vtkmodules
  lidarview
  lidarviewcore
  LidarPlugin
  )

# Get LV & PV Plugins
function (lidarview_detect_plugins plugins_dir output)
    set(plugins_list)
    file(GLOB subpaths LIST_DIRECTORIES TRUE RELATIVE ${plugins_dir} ${plugins_dir}/*)
    foreach(subpath ${subpaths})
      if(IS_DIRECTORY ${plugins_dir}/${subpath})
        LIST(APPEND plugins_list ${subpath})
      endif()
    endforeach()
    set( ${output} ${plugins_list} PARENT_SCOPE)
endfunction ()

# Get Lidarview Plugins Name from directories 
set(lidarview_plugins)
lidarview_detect_plugins( "${superbuild_install_location}/${LV_PLUGIN_INSTALL_SUBDIRECTORY}" lidarview_plugins)
if(NOT lidarview_plugins)
  message(FATAL_ERROR "lidarview_plugins is empty")
endif()
list(REMOVE_ITEM lidarview_plugins "PythonQtPlugin") #WIP while it is still a static library, we have to discard it

# Get Paraview Plugins Name from directories 
set(paraview_plugins)
lidarview_detect_plugins( "${superbuild_install_location}/${LV_INSTALL_PV_PLUGIN_SUBDIR}" paraview_plugins)

# Set include/exclude regexes # can be used for optional packaging things or fixing system linking bugs
# Discarding anything from `/lidarview-superbuild/common-superbuild` fixes fixup-bundle mistakes
set(include_regexes)
set(exclude_regexes ".*/build/lidarview-superbuild/common-superbuild/.*")

#paraview_install_extra_data()
#see https://gitlab.kitware.com/paraview/paraview-superbuild/-/blob/master/projects/paraview.bundle.common.cmake
#getting started .pdf...

# THIRDPARTY

# Ship Qt5 plugins
if (Qt5_DIR)
  if (WIN32)
  list(APPEND library_paths
    "${Qt5_DIR}/../../../bin")
  elseif (APPLE)
    list(APPEND library_paths
    "${Qt5_DIR}/../..")
  elseif (UNIX)
    list(APPEND library_paths
    "${Qt5_DIR}/../..")
  endif ()
endif ()

if (qt5_enabled)
  include(qt5.functions)

  set(qt5_plugin_prefix)
  if (NOT WIN32)
    set(qt5_plugin_prefix "lib")
  endif ()

  # Add SVG support, so ParaView can use SVG icons
  set(qt5_plugins
    iconengines/${qt5_plugin_prefix}qsvgicon
    imageformats/${qt5_plugin_prefix}qsvg
    imageformats/${qt5_plugin_prefix}qjpeg
    )

  if (WIN32)
    list(APPEND qt5_plugins
      platforms/qwindows)

    if (NOT qt5_version VERSION_LESS "5.10")
      list(APPEND qt5_plugins
        styles/qwindowsvistastyle)
    endif ()
  elseif (APPLE)
    list(APPEND qt5_plugins
      platforms/libqcocoa
      printsupport/libcocoaprintersupport)

    if (NOT qt5_version VERSION_LESS "5.10")
      list(APPEND qt5_plugins
        styles/libqmacstyle)
    endif ()
  elseif (UNIX)
    list(APPEND qt5_plugins
      platforms/libqxcb
      platforminputcontexts/libcomposeplatforminputcontextplugin
      xcbglintegrations/libqxcb-glx-integration)
  endif ()

  superbuild_install_qt5_plugin_paths(qt5_plugin_paths ${qt5_plugins})
else ()
  set(qt5_plugin_paths)
endif ()
