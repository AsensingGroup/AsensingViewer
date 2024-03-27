superbuild_add_project(ceres
  DEPENDS eigen glog

  CMAKE_ARGS
  -DCMAKE_CXX_STANDARD=17
  -DCMAKE_CXX_STANDARD_REQUIRED=ON
  -DCXX11=ON
  -DBUILD_SHARED_LIBS=ON
  -DBUILD_TESTING=OFF
  -DBUILD_EXAMPLES=OFF
  -DGFLAGS=OFF
  -DCUDA=OFF
)

if (WIN32)
  superbuild_append_flags(cxx_flags "/MD" PROJECT_ONLY)
endif()
