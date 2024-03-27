find_program(SUPERBUILD_NINJA_COMMAND
  NAMES ninja-build ninja
  DOC "Path to `ninja` for ninja-using projects")
mark_as_advanced(SUPERBUILD_NINJA_COMMAND)

if (ninja_enabled AND NOT SUPERBUILD_NINJA_COMMAND)
  message(FATAL_ERROR
    "This build is using `meson` which requires a `ninja` binary to be "
    "present on the system when used. It has not been found (see "
    "`SUPERBUILD_NINJA_COMMAND`). It is usually packaged as `ninja` or "
    "`ninja-build` in most package managers. It can also be downloaded from "
    "upstream directly: https://github.com/ninja-build/ninja/releases")
endif ()

if (ninja_enabled)
  set(superbuild_ninja_command "${SUPERBUILD_NINJA_COMMAND}")
endif ()
