#Copied from paraview-superbuild/projects/paraview.cmake with minor changes:
# - Deprecated DPARAVIEW_PYTHON_VERSION
# - Enabled VTK_MODULE_ENABLE_VTK_libproj

set(PARAVIEW_EXTRA_CMAKE_ARGUMENTS ""
  CACHE STRING "Extra arguments to be passed to ParaView when configuring.")
mark_as_advanced(PARAVIEW_EXTRA_CMAKE_ARGUMENTS)

set(PARAVIEW_BUILD_EDITION "CANONICAL"
  CACHE STRING "Build selected ParaView Edition")
set_property(CACHE PARAVIEW_BUILD_EDITION
  PROPERTY
    STRINGS "CORE;RENDERING;CATALYST;CATALYST_RENDERING;CANONICAL")

set (paraview_extra_cmake_options)
if (PV_NIGHTLY_SUFFIX)
  list(APPEND paraview_extra_cmake_options
    -DPV_NIGHTLY_SUFFIX:STRING=${PV_NIGHTLY_SUFFIX})
endif ()

# Without an offscreen rendering backend, X should be used.
set(paraview_use_x ON)
if (WIN32 OR APPLE OR osmesa_enabled OR egl_enabled)
  set(paraview_use_x OFF)
endif()

set(paraview_all_plugins)

set(paraview_smp_backend "Sequential")
if (tbb_enabled)
  set(paraview_smp_backend "TBB")
endif ()

set(paraview_enable_cuda "OFF")
if(vtkm_enabled AND cuda_enabled)
  set(paraview_enable_cuda "ON")
endif()

set(PARAVIEW_EXTERNAL_PROJECTS ""
  CACHE STRING "A list of projects for ParaView to depend on")
mark_as_advanced(PARAVIEW_EXTERNAL_PROJECTS)

set(paraviews_platform_dependencies)
if (UNIX)
  if (NOT APPLE)
    list(APPEND paraviews_platform_dependencies
      mesa osmesa egl

      # Needed for fonts to work properly.
      fontconfig)
  endif ()
  list(APPEND paraviews_platform_dependencies
    ffmpeg libxml2 freetype mili gmsh
    # For cosmotools
    genericio cosmotools)
endif ()

if (WIN32)
  list(APPEND paraviews_platform_dependencies
    openvr)
endif ()

if (USE_NONFREE_COMPONENTS AND (WIN32 OR (UNIX AND NOT APPLE)))
  list(APPEND paraviews_platform_dependencies
    visrtx)
endif ()

set(PARAVIEW_USE_PYTHON ${python3_enabled})
if (python3_enabled AND USE_SYSTEM_python3 AND NOT python3_FIND_LIBRARIES)
  message(FATAL_ERROR "Python3 libraries not found")
  set(PARAVIEW_USE_PYTHON OFF)
endif()

if (expat_enabled)
  list(APPEND paraviews_platform_dependencies expat)
endif ()

cmake_dependent_option(PARAVIEW_INITIALIZE_MPI_ON_CLIENT
  "Initialize MPI on client-processes by default. Can be overridden using command line arguments" ON
  "mpi_enabled" OFF)
mark_as_advanced(PARAVIEW_INITIALIZE_MPI_ON_CLIENT)

if (mpi_enabled)
  list(APPEND paraview_extra_cmake_options
    -DPARAVIEW_INITIALIZE_MPI_ON_CLIENT:BOOL=${PARAVIEW_INITIALIZE_MPI_ON_CLIENT})
endif()

option(PARAVIEW_ENABLE_MOTIONFX "Enable MotionFX reader, if supported on platform" ON)
mark_as_advanced(PARAVIEW_ENABLE_MOTIONFX)

option(PARAVIEW_ENABLE_VRPLUGIN "Enable VRPlugin" ON)
mark_as_advanced(PARAVIEW_ENABLE_VRPLUGIN)

# vrui support is only available on linux
if (PARAVIEW_ENABLE_VRPLUGIN AND UNIX)
  list(APPEND paraview_extra_cmake_options
    -DPARAVIEW_PLUGIN_VRPlugin_USE_VRUI:BOOL=ON
  )
endif()

set(paraview_use_raytracing OFF)
if (ospray_enabled OR visrtx_enabled)
  set(paraview_use_raytracing ON)
endif ()

# add an option to override ParaView shared-libs flag.
set(BUILD_SHARED_LIBS_paraview "<same>"
  CACHE STRING "The shared/static build flag for the paraview project.")
set_property(CACHE "BUILD_SHARED_LIBS_paraview"
  PROPERTY
    STRINGS "<same>;ON;OFF")
get_property(build_shared_options
  CACHE     "BUILD_SHARED_LIBS_paraview"
  PROPERTY  STRINGS)
if (NOT BUILD_SHARED_LIBS_paraview IN_LIST build_shared_options)
  string(REPLACE ";" ", " build_shared_options "${build_shared_options}")
  message(FATAL_ERROR "BUILD_SHARED_LIBS_paraview must be one of: ${build_shared_options}.")
endif ()

if (BUILD_SHARED_LIBS_paraview STREQUAL "<same>")
  set(paraview_build_shared_libs "${BUILD_SHARED_LIBS}")
else ()
  set(paraview_build_shared_libs "${BUILD_SHARED_LIBS_paraview}")
endif ()

get_property(paraview_tbb_supported GLOBAL
  PROPERTY superbuild_tbb_supported)
set(paraview_tbb_dependencies)
if (paraview_tbb_supported)
  list(APPEND paraview_tbb_dependencies
    tbb ospray)
endif ()

superbuild_add_project(paraview
  DEBUGGABLE
  DEFAULT_ON
  DEPENDS cxx11
  DEPENDS_OPTIONAL
    adios2 cuda boost fortran gdal hdf5 matplotlib mpi numpy png protobuf
    python3 qt5 visitbridge zlib silo liblas lookingglass fides #We renamed project las to liblas
    xdmf3 vrpn vtkm netcdf
    openpmd
    nlohmannjson
    paraviewgettingstartedguide
    paraviewtutorialdata paraviewweb
    ${paraview_all_plugins}
    ${paraviews_platform_dependencies}
    ${paraview_tbb_dependencies}
    ${PARAVIEW_EXTERNAL_PROJECTS}

  CMAKE_ARGS
    -DCMAKE_INSTALL_LIBDIR:PATH=lib
    -DCMAKE_INSTALL_NAME_DIR:PATH=<INSTALL_DIR>/lib
    -DCMAKE_MACOSX_RPATH:BOOL=OFF
    -DHDF5_NO_FIND_PACKAGE_CONFIG_FILE:BOOL=ON
    -DPARAVIEW_BUILD_LEGACY_REMOVE:BOOL=ON
    -DPARAVIEW_BUILD_SHARED_LIBS:BOOL=${paraview_build_shared_libs}
    -DPARAVIEW_BUILD_TESTING:BOOL=OFF
    -DPARAVIEW_BUILD_EDITION:STRING=${PARAVIEW_BUILD_EDITION}
    -DPARAVIEW_ENABLE_ADIOS2:BOOL=${adios2_enabled}
    -DPARAVIEW_ENABLE_COSMOTOOLS:BOOL=${cosmotools_enabled}
    -DPARAVIEW_ENABLE_FFMPEG:BOOL=${ffmpeg_enabled}
    -DPARAVIEW_ENABLE_FIDES:BOOL=${fides_enabled}
    -DPARAVIEW_ENABLE_GDAL:BOOL=${gdal_enabled}
    -DPARAVIEW_ENABLE_LAS:BOOL=${liblas_enabled} #We renamed project las to liblas
    -DPARAVIEW_ENABLE_LOOKINGGLASS:BOOL=${lookingglass_enabled}
    -DPARAVIEW_ENABLE_MOTIONFX:BOOL=${PARAVIEW_ENABLE_MOTIONFX}
    -DPARAVIEW_ENABLE_VISITBRIDGE:BOOL=${visitbridge_enabled}
    -DPARAVIEW_ENABLE_XDMF3:BOOL=${xdmf3_enabled}
    -DPARAVIEW_INSTALL_DEVELOPMENT_FILES:BOOL=ON
    -DPARAVIEW_PLUGIN_ENABLE_GmshIO:BOOL=${gmsh_enabled}
    -DPARAVIEW_PLUGIN_ENABLE_LookingGlass:BOOL=${lookingglass_enabled}
    -DPARAVIEW_PLUGIN_ENABLE_OpenVR:BOOL=${openvr_enabled}
    # No netcdftime module in the package.
    -DPARAVIEW_PLUGIN_ENABLE_NetCDFTimeAnnotationPlugin:BOOL=OFF
    #-DPARAVIEW_PYTHON_VERSION:STRING=${python_version} #Deprecated
    -DPARAVIEW_USE_MPI:BOOL=${mpi_enabled}
    -DPARAVIEW_USE_FORTRAN:BOOL=${fortran_enabled}
    -DPARAVIEW_USE_PYTHON:BOOL=${PARAVIEW_USE_PYTHON}
    -DPARAVIEW_USE_QT:BOOL=${qt5_enabled}
    -DVISIT_BUILD_READER_Mili:BOOL=${mili_enabled}
    -DVISIT_BUILD_READER_Silo:BOOL=${silo_enabled}
    -DVTK_DEFAULT_RENDER_WINDOW_OFFSCREEN:BOOL=${osmesa_enabled}
    -DVTK_MODULE_USE_EXTERNAL_ParaView_protobuf:BOOL=${protobuf_enabled}
    -DVTK_MODULE_USE_EXTERNAL_VTK_expat:BOOL=${expat_enabled}
    -DVTK_MODULE_USE_EXTERNAL_VTK_freetype:BOOL=${freetype_enabled}
    -DVTK_MODULE_USE_EXTERNAL_VTK_hdf5:BOOL=${hdf5_enabled}
    -DVTK_MODULE_USE_EXTERNAL_VTK_libxml2:BOOL=${libxml2_enabled}
    -DVTK_MODULE_USE_EXTERNAL_VTK_netcdf:BOOL=${netcdf_enabled}
    -DVTK_MODULE_USE_EXTERNAL_VTK_png:BOOL=${png_enabled}
    -DVTK_MODULE_USE_EXTERNAL_VTK_zlib:BOOL=${zlib_enabled}
    -DVTK_OPENGL_HAS_EGL:BOOL=${egl_enabled}
    -DVTK_OPENGL_HAS_OSMESA:BOOL=${osmesa_enabled}
    -DVTK_SMP_IMPLEMENTATION_TYPE:STRING=${paraview_smp_backend}
    -DVTK_USE_X:BOOL=${paraview_use_x}

    # raytracing flags
    -DPARAVIEW_ENABLE_RAYTRACING:BOOL=${paraview_use_raytracing}
    -DVTKOSPRAY_ENABLE_DENOISER:BOOL=${openimagedenoise_enabled}
    -DVTK_ENABLE_OSPRAY:BOOL=${ospray_enabled}
    -DVTK_ENABLE_VISRTX:BOOL=${visrtx_enabled}

    # IndeX
    -DPARAVIEW_PLUGIN_ENABLE_pvNVIDIAIndeX:BOOL=${nvidiaindex_enabled}

    # vrpn
    -DPARAVIEW_PLUGIN_ENABLE_VRPlugin:BOOL=${PARAVIEW_ENABLE_VRPLUGIN}
    -DPARAVIEW_PLUGIN_VRPlugin_USE_VRPN:BOOL=${vrpn_enabled}

    # vtkm
    -DPARAVIEW_PLUGIN_ENABLE_VTKmFilters:BOOL=${vtkm_enabled}
    -DPARAVIEW_USE_VTKM:BOOL=${vtkm_enabled}
    -DVTK_VTKM_ENABLE_CUDA:BOOL=${paraview_enable_cuda}

    # Web
    -DPARAVIEW_ENABLE_WEB:BOOL=${paraviewweb_enabled}

    # Readers
    -DVTK_MODULE_ENABLE_VTK_IOSegY:STRING=YES
    
    # LidarView requirements
    -DVTK_MODULE_ENABLE_VTK_libproj:STRING=YES

    # ParFlow
    -DPARAVIEW_PLUGIN_ENABLE_ParFlow:BOOL=${nlohmannjson_enabled}

    ${paraview_extra_cmake_options}

    ${PARAVIEW_EXTRA_CMAKE_ARGUMENTS})

if (paraview_SOURCE_SELECTION STREQUAL "5.3.0")
  superbuild_apply_patch(paraview fix-benchmarks
    "Fix various issues with the shipped benchmark scripts")
  superbuild_apply_patch(paraview fix-vtkconfig-part1
    "Fix various issues with the VTKConfig.cmake (Part 1)")
endif ()

if (paraview_SOURCE_SELECTION STREQUAL "5.5.0")
  superbuild_apply_patch(paraview fix-mpi4py
    "Fix issue with building VTK's mpi4py")
endif ()

if (paraview_SOURCE_SELECTION STREQUAL "5.6.0")
  superbuild_apply_patch(paraview fix-catalyst-adapter-deps
    "Fix issue with catalyst adapters and dependency search")
  superbuild_apply_patch(paraview fix-eye-dome-lighting
    "Fix eye dome lighting in parallel")
endif ()

if (WIN32 AND liblas_enabled)
  superbuild_append_flags(cxx_flags "-DBOOST_ALL_NO_LIB" PROJECT_ONLY)
endif()

if (ospray_enabled AND tbb_enabled)
  superbuild_add_extra_cmake_args(
    -DTBB_ROOT:PATH=<INSTALL_DIR>)
endif()
