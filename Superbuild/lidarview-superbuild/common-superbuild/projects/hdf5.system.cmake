find_package(HDF5 REQUIRED)

list(JOIN HDF5_DEFINITIONS "${_superbuild_list_separator}" hdf5_definitions_escaped)

superbuild_add_extra_cmake_args(
  -DHDF5_INCLUDE_DIRS=${HDF5_INCLUDE_DIRS}
  -DHDF5_DEFINITIONS=${hdf5_definitions_escaped}
  -DHDF5_LIBRARIES=${HDF5_LIBRARIES}
  -DHDF5_HL_LIBRARIES=${HDF5_HL_LIBRARIES})
