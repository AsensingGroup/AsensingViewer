find_package(PNG REQUIRED)

superbuild_add_extra_cmake_args(
  -DPNG_PNG_INCLUDE_DIR:PATH=${PNG_PNG_INCLUDE_DIR}
  -DPNG_LIBRARY:FILEPATH=${PNG_LIBRARY})
