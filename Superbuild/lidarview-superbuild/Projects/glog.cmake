superbuild_add_project(glog
  CMAKE_ARGS
    -DBUILD_SHARED_LIBS=ON
    -DBUILD_TESTING=OFF
    -DWITH_GFLAGS=OFF
)

if (WIN32)
  superbuild_append_flags(cxx_flags "/MD" PROJECT_ONLY)
endif()
