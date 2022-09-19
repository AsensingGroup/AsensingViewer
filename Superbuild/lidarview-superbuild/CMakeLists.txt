cmake_minimum_required(VERSION 3.12)

# Set a default build type if none was specified
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to 'RelWithDebInfo' as none was specified.")
  set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release"
    "MinSizeRel" "RelWithDebInfo")
endif()

#Variables
project(LidarViewSuperBuild)
set (SuperBuild_CMAKE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/CMake")
set (SuperBuild_PROJECTS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Projects")
set (SuperBuild_BINARY_DIR ${LidarViewSuperBuild_BINARY_DIR}) # required to do FindPackage(ParaView) inside LidarView's CMakeLists
set (SuperBuild_SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR})

# Setup CMAKE_MODULE_PATH so that platform specific configurations are processed
# before the generic ones.
set (CMAKE_MODULE_PATH
  "${SuperBuild_PROJECTS_DIR}"
  "${SuperBuild_CMAKE_DIR}"
  "${CMAKE_CURRENT_SOURCE_DIR}"
  #"${CMAKE_CURRENT_SOURCE_DIR}/../CMake" #Not needed
  ${CMAKE_MODULE_PATH})

# Set LV_BUILD_PLATFORM
set(LV_DISTRO "${CMAKE_SYSTEM_NAME}")
if(UNIX AND NOT APPLE)
  # Use Distro Name+Version instead of plain "Linux"
  find_program(LSB_RELEASE_EXEC lsb_release)
  if(NOT LSB_RELEASE_EXEC)
    message(WARNING "Could not detect lsb_release executable")
  else()
    execute_process(COMMAND "${LSB_RELEASE_EXEC}" --short --id OUTPUT_VARIABLE LSB_RELEASE_ID_SHORT OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process(COMMAND "${LSB_RELEASE_EXEC}" --short --release OUTPUT_VARIABLE LSB_RELEASE_VERSION_SHORT OUTPUT_STRIP_TRAILING_WHITESPACE)
    set(LV_DISTRO "${LSB_RELEASE_ID_SHORT}${LSB_RELEASE_VERSION_SHORT}")
  endif()
endif()
set (LV_BUILD_PLATFORM "${LV_DISTRO}-${CMAKE_SYSTEM_PROCESSOR}")

#------------------------------------------------------------------------------
#Configuration

# Config SB
set(_git_clone_arguments_default --progress)
set(_superbuild_default_cxx11 ON)
#set(_superbuild_default_nlohmannjson ON)

# Config Qt
set(qt_version 5)
list(APPEND qt5_extra_options -no-icu) # ICU is not used for localisation

# skiped module as defined in paraview superbuild.
# Differences:
#   - pythonQt needs Multimedia so we don't skip it.
set(qt5_skip_modules
  qtspeech
  qtconnectivity
  qtgamepad
  qtlocation
  qtsensors
  qtserialport
  qtwayland
  qtwebchannel
  qtwebengine
  qtwebsockets)
set(_superbuild_qt5_default_selection "5.12")
set(qt5_ENABLE_SVG ON CACHE INTERNAL "ParaView requires SVG support") # Localisation lib only required for qtwebkit

# Config Boost
set(boost_libraries
  atomic
  date_time
  filesystem
  iostreams
  program_options
  thread
  chrono
  regex
  system)
 
set(boost_extra_options
  "-sNO_BZIP2=1")

# Check if some Boost environment variables could hide local Boost install
if (NOT USE_SYSTEM_boost AND (DEFINED ENV{BOOSTROOT} OR DEFINED ENV{BOOST_ROOT} OR DEFINED ENV{BOOST_LIBRARYDIR} OR DEFINED ENV{BOOST_INCLUDEDIR}))
  message(WARNING "Some Boost environment variables are set and may hide the local superbuild Boost installation. "
                  "Consider enabling USE_SYSTEM_boost flag or unsetting the following environement variables :\n"
                  " BOOSTROOT=$ENV{BOOSTROOT}\n BOOST_ROOT=$ENV{BOOST_ROOT}\n BOOST_LIBRARYDIR=$ENV{BOOST_LIBRARYDIR}\n BOOST_INCLUDEDIR=$ENV{BOOST_INCLUDEDIR}")
endif()

#------------------------------------------------------------------------------
#SuperBuild Macros

macro (superbuild_setup_variables)
    include(SuperbuildVersionMacros)
    set(paraview_default_version "5.9.0")
    superbuild_set_version_variables(paraview "${paraview_default_version}" "paraview-version.cmake" "version.txt")
    set(paraview_version "${paraview_version_major}.${paraview_version_minor}")
endmacro ()

function (superbuild_find_projects var)
  # Some of these allow using system libraries.
  set(projects
    cxx11
    paraview
    python3
    qt5
    pythonqt
    boost
    bzip2
    eigen
    liblas
    freetype
    opencv
    pcap
    png
    zlib
    ceres
    glog
    pcl
    qhull
    flann
    nanoflann
    yaml
    darknet
    g2o
    numpy
    fortran
    lapack
    pythoncython
    pythonpythran
    pythonsetuptools
    pythongast
    pythonbeniget
    pythonply
    )

  if (WIN32)
    list(APPEND projects
      pywin32
    )
  endif()

  if (UNIX)
    list(APPEND projects
      libxml2
      ffi
      sqlite
    )

    if (NOT APPLE)
      list(APPEND projects
        fontconfig
        gperf
        utillinux
        pkgconf
      )
    endif()
  endif()
    
  if (COMMAND add_project_to_superbuild)
    add_project_to_superbuild(projects)
  else (COMMAND add_project_to_superbuild)
	  message(WARNING "The function add_project_to_superbuild is not defined. Only LidarView's common dependencies will be compile.")
  endif(COMMAND add_project_to_superbuild)
  set("${var}"
    ${projects}
    PARENT_SCOPE)
endfunction ()

function (superbuild_sanity_check)
  #TODO
endfunction ()

function (superbuild_add_packaging)

  if (WIN32)
    set(generators
      ZIP
      NSIS)
  elseif (APPLE)
    set(generators
      DragNDrop)
  else ()
    set(generators
      TGZ
      TXZ)
  endif ()
  list(GET generators 0 default_generator)
  
  if (qt5_enabled AND (USE_SYSTEM_qt5 OR APPLE OR WIN32))
    list(APPEND superbuild_export_variables
      Qt5_DIR)
    find_package(Qt5 QUIET REQUIRED COMPONENTS Core)
    set(qt5_version "${Qt5Core_VERSION_MAJOR}.${Qt5Core_VERSION_MINOR}")
  else ()
    set(qt5_version "${qt5_SOURCE_SELECTION}")
  endif ()
  list(APPEND superbuild_export_variables
    qt5_version)
  
  # pass some variables that are required for packaging step:
  list(APPEND superbuild_export_variables
    paraview_version
    LV_BUILD_PLATFORM
    )
  
  set(default_package lidarview)
  foreach (generator IN LISTS generators)
      superbuild_add_extra_package_test(lidarview "${generator}"
      LABELS  "LidarView" # does not have to be ${SOFTWARE_NAME} I think, it is currently not available here
      TIMEOUT 6400)
  endforeach ()
  
  if (default_package)
    superbuild_enable_install_target("${default_package}/${default_generator}")
  endif ()

endfunction ()

function (superbuild_add_tests)
  # TODO: add existing superbuild tests
  # add_subdirectory("${CMAKE_SOURCE_DIR}/tests" "${CMAKE_BINARY_DIR}/tests")
endfunction ()

list(APPEND superbuild_version_files
  "${SuperBuild_SOURCE_DIR}/versions.cmake")
#list(APPEND superbuild_ctest_custom_files
#  "${SuperBuild_SOURCE_DIR}/cmake/CTestCustom.cmake")
list(APPEND superbuild_project_roots
  "${SuperBuild_PROJECTS_DIR}")

# Dependencies Options
option(ENABLE_all "Enable all optional dependancy like pcl, ceres, opencv, darknet, ..." OFF)
if (ENABLE_all)
  set(ENABLE_opencv ON CACHE BOOL "enable OpenCV")
  set(ENABLE_ceres ON CACHE BOOL "enable Ceres")
  set(ENABLE_pcl ON CACHE BOOL "enable PCL")
  set(ENABLE_nanoflann ON CACHE BOOL "enable nanoflann")
  #set(ENABLE_darknet ON CACHE BOOL "enable darknet")
  #set(ENABLE_g2o ON CACHE BOOL "enable G2O")
endif(ENABLE_all)

# Features Options
option(LIDARVIEW_BUILD_SLAM "Build Lidar SLAM (requires PCL, Ceres, nanoflann; optional g2o)" OFF)
if(LIDARVIEW_BUILD_SLAM)
  if(NOT ENABLE_ceres)
    message(FATAL_ERROR "SLAM requires Ceres")
  endif()
  if(NOT ENABLE_pcl)
    message(FATAL_ERROR "SLAM requires PCL")
  endif()
  if(NOT ENABLE_nanoflann)
    message(FATAL_ERROR "SLAM requires nanoflann")
  endif()
endif()

# Common-SuperBuild
add_subdirectory(common-superbuild)
