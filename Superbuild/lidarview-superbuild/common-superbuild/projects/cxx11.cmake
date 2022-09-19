superbuild_add_dummy_project(cxx11)

if (cxx11_enabled)
  set(CMAKE_CXX_STANDARD 11)
  set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

  add_library(cxx11_check
    "${CMAKE_CURRENT_LIST_DIR}/scripts/cxx11.cxx")
endif ()

superbuild_add_extra_cmake_args(
  -DCMAKE_CXX_STANDARD:STRING=11
  -DCMAKE_CXX_STANDARD_REQUIRED:STRING=TRUE)

superbuild_append_flags(cxx_flags "${CMAKE_CXX11_STANDARD_COMPILE_OPTION}")
