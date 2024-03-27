# This is a copy from paraview superbuild project :
# https://gitlab.kitware.com/paraview/paraview-superbuild/blob/c8006d9a96eaa06228a011bc8d646541106643ec/projects/las.cmake
# We could fetch it in the paraview download dir directly, switching to PDAL is also considered

cmake_dependent_option(Boost_NO_BOOST_CMAKE "Boost_NO_BOOST_CMAKE" ON "NOT USE_SYSTEM_boost" OFF)

superbuild_add_project(liblas
  DEPENDS boost
  CMAKE_ARGS
    -DWITH_GDAL:BOOL=OFF
    -DBUILD_OSGEO4W:BOOL=OFF
    -DWITH_GEOTIFF:BOOL=OFF
    -DWITH_LASZIP:BOOL=OFF
    -DWITH_TESTS:BOOL=OFF
    -DWITH_UTILITIES:BOOL=OFF
    -DBoost_NO_BOOST_CMAKE:BOOL=${Boost_NO_BOOST_CMAKE}
    -DBoost_USE_STATIC_LIBS:BOOL=OFF
    -DCMAKE_INSTALL_NAME_DIR:PATH=<INSTALL_DIR>/lib # This is not a perfect solution but in par with pv-sb
)

if (WIN32)
  superbuild_append_flags(cxx_flags "-DBOOST_ALL_NO_LIB" PROJECT_ONLY)
endif()

if (APPLE)
  superbuild_append_flags(cxx_flags "-stdlib=libc++" PROJECT_ONLY)
  superbuild_append_flags(ld_flags "-stdlib=libc++" PROJECT_ONLY)
endif ()

superbuild_add_extra_cmake_args(
  -DlibLAS_DIR:PATH=<INSTALL_DIR>
)
