superbuild_add_project(flann
  CMAKE_ARGS
   -DBUILD_EXAMPLES:BOOL=OFF
   -DBUILD_TESTS:BOOL=OFF
   -DBUILD_DOC:BOOL=OFF
   -DBUILD_PYTHON_BINDINGS:BOOL=OFF
   -DBUILD_MATLAB_BINDINGS:BOOL=OFF
   -DCMAKE_INSTALL_NAME_DIR:PATH=<INSTALL_DIR>/lib
)
