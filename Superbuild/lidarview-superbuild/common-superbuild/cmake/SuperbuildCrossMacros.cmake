#[==[.md
# Cross compilation support

This is untested, but exists from prior support in a previous implemenation of
the superbuild. Most of them are intended to be used by the common superbuild
itself, but there are some hooks projects can set.

It seems that the module is largely ignored for now.
#]==]

#[==[.md
## Hooks

### `superbuild_cross_prepare`

If defined, this command is called in order to to do any up-front configuration
settings which may be necessary.
#]==]

# Sets up the build for a particular target platform.
#
# Currently unused.
function (superbuild_cross_determine_target)
  if (NOT CMAKE_CROSSCOMPILING)
    return ()
  endif ()

  # Ask user to say what machine they are compiling onto so we can get the
  # right environment settings.
  _superbuild_cross_target_machine()

  # Configure platform dependent settings 64bit_build, static_only.
  _superbuild_cross_platform_settings()

  if (COMMAND superbuild_cross_prepare)
    superbuild_cross_prepare()
  endif ()
endfunction ()

# Provide a cache variable to indicate the targeted machine.
function (_superbuild_cross_target_machine)
  set(SUPERBUILD_CROSS_TARGET "generic"
    CACHE STRING "Platform to cross compile for, either generic|bgp_xlc|bgq_xlc|bgq_gnu|xk7_gnu")
  # TODO: This should be managed by the project, not hard-coded.
  set_property(CACHE SUPERBUILD_CROSS_TARGET PROPERTY STRINGS
    "generic" "bgp_xlc" "bgq_xlc" "bgq_gnu" "xk7_gnu")
endfunction ()

# Includes an optional site-specific file from the cross-compiling directory.
#
# Currently unused.
function (_superbuild_cross_include_file var name)
  set(site_file
    "crosscompile/${SUPERBUILD_CROSS_TARGET}/${name}.cmake")
  include("${site_file}" OPTIONAL
    RESULT_VARIABLE res)
  if (NOT res)
    set(site_file)
  endif ()

  set("${var}"
    "${site_file}"
    PARENT_SCOPE)
endfunction ()

# Configures the cmake files that describe how to crosscompile for a given
# platform. From the ${SUPERBUILD_CROSS_TARGET} directory into the build tree.
function (_superbuild_cross_platform_settings)
  set(site_toolchain
    "${CMAKE_SOURCE_DIR}/crosscompile/${SUPERBUILD_CROSS_TARGET}/toolchain.cmake.in")
  set(superbuild_cross_toolchain
    "${CMAKE_BINARY_DIR}/crosscompile/toolchain.cmake")

  configure_file(
    "${site_toolchain}"
    "${superbuild_cross_toolchain}"
    @ONLY)
endfunction ()
