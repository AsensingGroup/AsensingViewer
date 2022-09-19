superbuild_add_project(nanoflann
  DEPENDS eigen

  CMAKE_ARGS
  -DBUILD_EXAMPLES=OFF
  -DBUILD_TESTS=OFF
  -DEIGEN3_INCLUDE=<INSTALL_DIR>/include/eigen3
  )
