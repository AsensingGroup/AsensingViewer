superbuild_add_dummy_project(fortran)

if (fortran_enabled)
  enable_language(Fortran)
endif ()

superbuild_add_extra_cmake_args(
  -DCMAKE_Fortran_COMPILER:FILEPATH=${CMAKE_Fortran_COMPILER})

if (APPLE AND
    NOT EXISTS "/usr/lib/libSystem.dylib" AND
    NOT EXISTS "/usr/lib/libSystem.tbd")
  # macOS 11.5+ no longer ships with `libSystem.dylib` in `/usr/lib`, so
  # `gfortran` cannot find it when linking. Help it out as best we can.
  set(fortran_libsystem_path)
  set(fortran_framework_path)
  if (EXISTS "${CMAKE_OSX_SYSROOT}/usr/lib/libSystem.tbd")
    set(fortran_libsystem_path
      "${CMAKE_OSX_SYSROOT}/usr/lib")
    set(fortran_framework_path
      "/System/Library/Frameworks")
  endif ()

  if (fortran_libsystem_path)
    # Make sure that the superbuild's library path is searched before the
    # SDK's library path (causes confusion since the SDK has some of the
    # libraries we want to ship).
    string(APPEND _superbuild_fortran_ld_flags
      " -L<INSTALL_DIR>/lib")
    string(APPEND _superbuild_fortran_ld_flags
      " -L${fortran_libsystem_path}")
  endif ()
  if (fortran_framework_path)
    string(APPEND _superbuild_fortran_ld_flags
      " -F${fortran_framework_path}")
  endif ()
endif ()

# Support adding Fortran-specific linker flags.
if (_superbuild_fortran_ld_flags)
  superbuild_append_flags(ld_flags "${_superbuild_fortran_ld_flags}")
endif ()
