set(mesa_type_args -Dplatforms=x11 -Dglx=gallium-xlib -Dosmesa=false)
set(mesa_type_deps glproto)
include(mesa.common)

if (BUILD_SHARED_LIBS)
  set(mesa_library libGL.so)
else ()
  set(mesa_library libGL.a)
endif ()
superbuild_add_extra_cmake_args(
  -DOPENGL_INCLUDE_DIR:PATH=<INSTALL_DIR>/include
  -DOPENGL_xmesa_INCLUDE_DIR:PATH=<INSTALL_DIR>/include
  -DOPENGL_gl_LIBRARY:FILEPATH=<INSTALL_DIR>/lib/${mesa_library}
  -DOpenGL_GL_PREFERENCE:STRING=LEGACY
  -DOPENGL_egl_LIBRARY:FILEPATH=
  -DOPENGL_glx_LIBRARY:FILEPATH=
  -DOPENGL_opengl_LIBRARY:FILEPATH=
  -DOPENGL_EGL_INCLUDE_DIR:FILEPATH=
  -DOPENGL_GLX_INCLUDE_DIR:FILEPATH=
  -DOPENGL_xmesa_INCLUDE_DIR:FILEPATH=
  )
