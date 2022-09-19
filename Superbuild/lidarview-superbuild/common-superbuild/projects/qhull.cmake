set(qhull_build_static_libs ON)
if (BUILD_SHARED_LIBS)
  set(qhull_build_static_libs OFF)
endif ()

superbuild_add_project(qhull
  CMAKE_ARGS
    -DBUILD_SHARED_LIBS:BOOL=${BUILD_SHARED_LIBS}
    -DBUILD_STATIC_LIBS:BOOL=${qhull_build_static_libs}
    -DCMAKE_INSTALL_LIBDIR:STRING=lib
    -DCMAKE_INSTALL_NAME_DIR:PATH=<INSTALL_DIR>/lib)

superbuild_apply_patch(qhull apple-install-name-dir
  "fix bogus install name directory settings")
