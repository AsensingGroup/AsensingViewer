find_package(ZLIB REQUIRED)

superbuild_add_extra_cmake_args(
  -DZLIB_INCLUDE_DIR:PATH=${ZLIB_INCLUDE_DIR}
  -DZLIB_LIBRARY:FILEPATH=${ZLIB_LIBRARY}
)
