superbuild_add_project(yaml
  CMAKE_ARGS
  -DCMAKE_CXX_STANDARD=11
  -DCMAKE_CXX_STANDARD_REQUIRED=ON
  -DBUILD_TESTING=OFF
  -DYAML_CPP_BUILD_TESTS=OFF
)

superbuild_add_extra_cmake_args(
  -DYAML_DIR:PATH=<INSTALL_DIR>/include/yaml-cpp
)

if (WIN32)
  superbuild_append_flags(cxx_flags "/MD" PROJECT_ONLY)
endif()
