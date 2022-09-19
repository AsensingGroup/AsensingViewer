#[==[.md INTERNAL
# Unix utility functions

On Unix platforms (mainly Linux), some workarounds are sometimes necessary.
These functions implement these workarounds. They are used internally to the
superbuild and do not need to be called manually.
#]==]

include (CMakeDependentOption)

#[==[.md INTERNAL
Sometimes a build will conflict with a system library in use by an external
tool. Since the `LD_LIBRARY_PATH` environment variable is a global setting for
executables, this may cause interference. This function allows users to disable
the logic to continue a build.

The errors that show up if this needs to be disabled are usually due to
mismatches in sonames or version information.
#]==]
function (superbuild_unix_ld_library_path_hack var)
  cmake_dependent_option(PASS_LD_LIBRARY_PATH_FOR_BUILDS "Pass LD_LIBRARY_PATH to build scripts" ON
    "UNIX;NOT APPLE" OFF)
  mark_as_advanced(PASS_LD_LIBRARY_PATH_FOR_BUILDS)

  if (PASS_LD_LIBRARY_PATH_FOR_BUILDS AND superbuild_ld_library_path)
    set("${var}"
      LD_LIBRARY_PATH "${superbuild_ld_library_path}"
      PARENT_SCOPE)
  endif ()
endfunction ()
