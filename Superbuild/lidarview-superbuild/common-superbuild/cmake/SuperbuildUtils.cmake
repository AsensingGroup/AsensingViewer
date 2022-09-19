#[==[.md INTERNAL
# Utility functions

This module contains utility functions mostly used internally to the
superbuild. Some may be of use to corner-case project builds.
#]==]

include("SuperbuildUtils-apple")
include("SuperbuildUtils-unix")

get_property(multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
# TODO: In 3.9.0, use the GENERATOR_IS_MULTI_CONFIG global property.
if (NOT multi_config)
  set(_superbuild_build_type_force)
  if (NOT CMAKE_BUILD_TYPE)
    set(_superbuild_build_type_force FORCE)
  endif ()
  # Default to a Release build.
  set(CMAKE_BUILD_TYPE "Release"
    CACHE STRING "The build mode" ${_superbuild_build_type_force})
  mark_as_advanced(CMAKE_BUILD_TYPE)
  set_property(CACHE CMAKE_BUILD_TYPE
    PROPERTY
      STRINGS "Release;RelWithDebInfo")
  if (NOT WIN32)
    # Windows debug builds is not really supported at the moment. Getting all
    # projects to agree on the C++ runtime library is not easy. Also, many
    # projects hard-code library paths and on Windows, many projects change
    # their names for debug builds.
    set_property(CACHE CMAKE_BUILD_TYPE APPEND
      PROPERTY
        STRINGS "Debug")
  endif ()

  if (NOT CMAKE_BUILD_TYPE)
    message(FATAL_ERROR "A build type (CMAKE_BUILD_TYPE) must be set.")
  endif ()

  # Ensure that the chosen build type is valid.
  get_property(build_type_options
    CACHE     CMAKE_BUILD_TYPE
    PROPERTY  STRINGS)
  if (NOT CMAKE_BUILD_TYPE IN_LIST build_type_options)
    string(REPLACE ";" ", " build_type_options "${build_type_options}")
    message(FATAL_ERROR "CMAKE_BUILD_TYPE must be one of: ${build_type_options} (found ${CMAKE_BUILD_TYPE}).")
  endif ()

  if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    if (SUPERBUILD_ALLOW_DEBUG)
      message(WARNING
        "Debug builds are probably not what you want. This is an unsupported "
        "configuration. Please consider using the CMAKE_BUILD_TYPE_<project> "
        "options if you are debugging specific projects.")
    else ()
      message(FATAL_ERROR
        "Debug builds are probably not what you want. Set the "
        "SUPERBUILD_ALLOW_DEBUG variable using either the GUI ('Add Entry') "
        "or pass -DSUPERBUILD_ALLOW_DEBUG:BOOL=ON on the command to indicate "
        "this is what you intended.")
    endif ()
  endif ()
endif ()

#[==[.md INTERNAL
# 64-bit support

This functiond detects 32-bit targets and errors if it occurs.
#]==]
function (superbuild_detect_64bit_target)
  if (CMAKE_CROSSCOMPILING)
    return ()
  endif ()

  # Collect information about the build platform.
  include(CheckTypeSize)

  check_type_size(void* void_ptr_size
    BUILTIN_TYPES_ONLY)
  if (void_ptr_size EQUAL 8)
    # OK
  elseif (void_ptr_size EQUAL 4)
    message(FATAL_ERROR
      "32-bit targets are not supported.")
  else ()
    if (WIN32)
      set(extra_message "Are you in a Visual Studio command prompt?")
    else ()
      set(extra_message "Do you have working compilers?")
    endif ()
    message(FATAL_ERROR
      "Failed to determine whether the target architecture is 32bit or 64bit. "
      "${extra_message}")
  endif ()
endfunction ()

# A utility function to create a PATH-link environment variable value.
macro (_superbuild_make_path_var var)
  set(${var} ${ARGN})
  list(REMOVE_ITEM ${var} "")
  if (UNIX)
    string(REPLACE ";" ":" ${var} "${${var}}")
  endif ()
endmacro ()

#[==[.md INTERNAL
# Compiler flag setup

Superbuild projects may set add additional language flags to be made available
to projects during their builds via the standard environment variables (more
detail is available in [their documentation](SuperbuildVariables.md#Language
flags)). The function handles the setup of the flags made available to projects
by integrating the environment at configure time, the superbuild's installation
directory for projects.
#]==]
function (superbuild_setup_flags)
  if (WIN32)
    return ()
  endif ()

  _superbuild_make_path_var(superbuild_ld_library_path
    "${superbuild_install_location}/lib"
    "$ENV{LD_LIBRARY_PATH}")
  set(superbuild_ld_library_path "${superbuild_ld_library_path}" PARENT_SCOPE)

  _superbuild_make_path_var(superbuild_pkg_config_path
    "${superbuild_install_location}/lib/pkgconfig"
    "${superbuild_install_location}/share/pkgconfig"
    "$ENV{PKG_CONFIG_PATH}")
  set(superbuild_pkg_config_path "${superbuild_pkg_config_path}" PARENT_SCOPE)

  if (CMAKE_CROSSCOMPILING)
    return ()
  endif ()

  set(superbuild_cpp_flags "$ENV{CPPFLAGS} ${superbuild_extra_cpp_flags}")
  set(superbuild_cxx_flags "$ENV{CXXFLAGS} -fPIC ${superbuild_extra_cxx_flags}")
  set(superbuild_c_flags "$ENV{CFLAGS} -fPIC ${superbuild_extra_c_flags}")
  set(superbuild_f_flags "$ENV{FFLAGS} -fPIC ${superbuild_extra_f_flags}")
  set(superbuild_ld_flags "$ENV{LDFLAGS} ${superbuild_extra_ld_flags}")

  superbuild_osx_add_version_flags()

  foreach (var IN ITEMS cpp_flags cxx_flags c_flags f_flags ld_flags)
    set("superbuild_${var}"
      "${superbuild_${var}}"
      PARENT_SCOPE)
  endforeach ()
endfunction ()

#[==[.md INTERNAL
# Preparing the build tree

The build tree needs some preparation for some features. These steps are done
here.
#]==]
macro (superbuild_prepare_build_tree)
  if (WIN32)
    # Windows doesn't like it if that directory does not exist even if it is
    # empty.
    file(MAKE_DIRECTORY "${superbuild_install_location}/lib")
  endif ()

  set(_superbuild_module_gen_dir "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/cmake")
  file(MAKE_DIRECTORY "${_superbuild_module_gen_dir}")
  list(APPEND CMAKE_MODULE_PATH
    "${_superbuild_module_gen_dir}")
endmacro ()

#[==[.md
# Handling deprecated variables

Use this function when a user-visible flag is being renamed or otherwise
replaced. If the old value is set, it will be given as the default value,
otherwise the given default value will be used. This returned value should then
be used in the `set(CACHE)` or `option()` call for the new value.

If the old value is set, it will warn that it is deprecated for the new name.

If replacing the setting `OLD_SETTING` with `NEW_SETTING`, its usage would look
like:

```cmake
superbuild_deprecated_setting(default_setting NEW_SETTING OLD_SETTING "default value")
set(NEW_SETTING "${default_setting}"
  CACHE STRING "Documentation for the setting.")
```
#]==]
function (superbuild_deprecated_setting output_default new old intended_default)
  set(default "${intended_default}")
  if (DEFINED "${old}")
    message(DEPRECATION
      "The `${old}` variable is deprecated for `${new}`.")
    set(default "${${old}}")
  endif ()

  set("${output_default}" "${default}" PARENT_SCOPE)
endfunction ()

#[==[.md
# Catching a stale superbuild submodule

The common superbuild is intended to be used as a submodule to projects.
However, updating the submodule when updating the main superbuild is an easy
step to forget. This function tries to determine whether the common superbuild
is up-to-date or not and error out if it is not.
#]==]
function (_superbuild_check_up_to_date)
  file(RELATIVE_PATH common_superbuild_path
    "${CMAKE_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}")
  if (common_superbuild_path MATCHES "^\\.\\./")
    # We're in a project shipped by the common-superbuild.
    return ()
  endif ()

  find_package(Git QUIET)
  if (NOT Git_FOUND)
    # No Git; can't perform the check.
    return ()
  endif ()

  execute_process(
    COMMAND "${GIT_EXECUTABLE}"
            rev-parse
            --is-inside-work-tree
    RESULT_VARIABLE res
    OUTPUT_VARIABLE out
    ERROR_VARIABLE  err
    WORKING_DIRECTORY
            "${CMAKE_SOURCE_DIR}"
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_STRIP_TRAILING_WHITESPACE)
  if (res)
    # We're not in a git repository; assume all is well.
    return ()
  endif ()

  set(out_of_date
    "The common superbuild may be out of date, but cannot be verified.")

  execute_process(
    COMMAND "${GIT_EXECUTABLE}"
            ls-tree
            HEAD
            --
            "${common_superbuild_path}"
    RESULT_VARIABLE res
    OUTPUT_VARIABLE out
    ERROR_VARIABLE  err
    WORKING_DIRECTORY
            "${CMAKE_SOURCE_DIR}"
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_STRIP_TRAILING_WHITESPACE)
  if (res)
    message(WARNING
      "Failed to get information about the common superbuild from the "
      "containing repository. ${out_of_date}: ${err}")
    return ()
  endif ()
  if (NOT out MATCHES "^160000 ")
    # The superbuild is not a submodule; assume all is well.
    return ()
  endif ()
  string(REGEX REPLACE "^160000 commit \([a-f0-9]+\)\t.*$" "\\1"
    expected_commit "${out}")

  execute_process(
    COMMAND "${GIT_EXECUTABLE}"
            rev-parse
            HEAD
    RESULT_VARIABLE res
    OUTPUT_VARIABLE actual_commit
    ERROR_VARIABLE  err
    WORKING_DIRECTORY
            "${CMAKE_CURRENT_SOURCE_DIR}"
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_STRIP_TRAILING_WHITESPACE)
  if (res)
    message(WARNING
      "Failed to get the current commit of the common superbuild. "
      "${out_of_date}: ${err}")
    return ()
  endif ()

  execute_process(
    COMMAND "${GIT_EXECUTABLE}"
            merge-base
            --is-ancestor
            "${expected_commit}"
            "${actual_commit}"
    RESULT_VARIABLE res
    OUTPUT_VARIABLE out
    ERROR_VARIABLE  err
    WORKING_DIRECTORY
            "${CMAKE_CURRENT_SOURCE_DIR}"
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_STRIP_TRAILING_WHITESPACE)
  if (res EQUAL 1)
    set(severity FATAL_ERROR)
    if (superbuild_can_be_out_of_date)
      set(severity WARNING)
    endif ()
    message("${severity}"
      "The common superbuild is out of date and should be updated. Please run "
      "`git submodule update` in the source directory to update.")
  elseif (res)
    message(WARNING
      "Failed to determine if the common superbuild is an old checkout. "
      "${out_of_date}: ${err}")
  endif ()
endfunction ()
