if (CMAKE_GENERATOR MATCHES "Ninja")
  superbuild_add_dummy_project(ninja)
  if (ninja_enabled)
    set(superbuild_ninja_command "${CMAKE_MAKE_PROGRAM}")
  endif ()
else ()
  superbuild_add_project(ninja
    CAN_USE_SYSTEM
    CMAKE_ARGS
      -DBUILD_TESTING:BOOL=OFF)
  if (ninja_enabled)
    set(superbuild_ninja_command "<INSTALL_DIR>/bin/ninja")
  endif ()
endif ()
