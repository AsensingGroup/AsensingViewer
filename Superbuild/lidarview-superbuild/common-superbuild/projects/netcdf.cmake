set(netcdf_libdir)
if (UNIX AND NOT APPLE)
  set(netcdf_libdir
    -DCMAKE_INSTALL_LIBDIR:BOOL=lib)
endif ()

superbuild_add_project(netcdf
  CAN_USE_SYSTEM
  DEPENDS hdf5 zlib

  CMAKE_ARGS
    -DBUILD_SHARED_LIBS:BOOL=${BUILD_SHARED_LIBS}
    -DNC_FIND_SHARED_LIBS:BOOL=${BUILD_SHARED_LIBS}
    -DBUILD_TESTING:BOOL=OFF
    -DCMAKE_INSTALL_NAME_DIR:PATH=<INSTALL_DIR>/lib
    -DENABLE_TESTS:BOOL=OFF
    -DBUILD_UTILITIES:BOOL=OFF
    -DUSE_SZIP:BOOL=OFF
    -DENABLE_DAP:BOOL=OFF
    ${netcdf_libdir})

superbuild_apply_patch(netcdf fix-size-uchar
  "fix check on size of uchar: test for existence first")
