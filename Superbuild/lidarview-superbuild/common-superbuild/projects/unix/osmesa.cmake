set(mesa_type_args -Dplatforms= -Dglx=disabled -Dosmesa=true)
set(mesa_type_deps)
include(mesa.common)

if (BUILD_SHARED_LIBS)
  set(osmesa_library libOSMesa.so)
else ()
  set(osmesa_library libOSMesa.a)
endif ()
superbuild_add_extra_cmake_args(
  -DOPENGL_INCLUDE_DIR:PATH=IGNORE
  -DOPENGL_xmesa_INCLUDE_DIR:PATH=IGNORE
  -DOPENGL_gl_LIBRARY:FILEPATH=IGNORE
  -DOSMESA_INCLUDE_DIR:PATH=<INSTALL_DIR>/include
  -DOSMESA_LIBRARY:FILEPATH=<INSTALL_DIR>/lib/${osmesa_library})
