find_package(NetCDF REQUIRED)

superbuild_add_extra_cmake_args(
  -DNetCDF_LIBRARY:FILEPATH=${NetCDF_LIBRARY}
  -DNetCDF_INCLUDE_DIR:FILEPATH=${NetCDF_INCLUDE_DIR})
