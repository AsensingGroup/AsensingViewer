#=========================================================================
#
#  Program:   ParaView
#
#  Copyright (c) Kitware, Inc.
#  All rights reserved.
#  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.
#
#     This software is distributed WITHOUT ANY WARRANTY; without even
#     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
#     PURPOSE.  See the above copyright notice for more information.
#
#=========================================================================

find_package(Git)

#[==[.md
# Detect the version of a source tree

When creating packages, it can be useful to know the specific version of the
source that is being built. To that end, this module provides functions which
determine a version number for a source based on its source selection. It uses
the `<NAME>_SOURCE_DIR` (for a `source` selection) or the build tree location
(for a `git` selection) to query the version of the checked out code using `git
describe`.

If it turns out that Git is either not available or the source directory is not
a Git checkout, it reads a file in the source tree if available and if all else
fails, falls back to a static string.
#]==]

# Set a variable in the parent scope properly while still making it available
# in the current scope..
macro (_superbuild_set_up variable value)
  set("${variable}" "${value}"
    PARENT_SCOPE)
  set("${variable}" "${value}")
endmacro ()

function (_superbuild_parse_version var version)
  # Split the version number into fields.
  if (version MATCHES "([0-9]+)\\.([0-9]+)\\.([0-9]+)-?(.*)")
    set(full "${CMAKE_MATCH_0}")
    set(major "${CMAKE_MATCH_1}")
    set(minor "${CMAKE_MATCH_2}")
    set(patch "${CMAKE_MATCH_3}")
    set(patch_extra "${CMAKE_MATCH_4}")
  elseif (version MATCHES "([0-9]+)\\.([0-9]+)-?(.*)")
    set(full "${CMAKE_MATCH_0}")
    set(major "${CMAKE_MATCH_1}")
    set(minor "${CMAKE_MATCH_2}")
    set(patch "")
    set(patch_extra "${CMAKE_MATCH_3}")
  elseif (version MATCHES "([0-9]+)-?(.*)")
    set(full "${CMAKE_MATCH_0}")
    set(major "${CMAKE_MATCH_1}")
    set(minor "")
    set(patch "")
    set(patch_extra "${CMAKE_MATCH_2}")
  else ()
    message(FATAL_ERROR
      "Failed to determine the version for ${var}; got ${version}")
  endif ()

  # Set variables in the parent scope if they're available.
  if (full)
    set("${var}_VERSION" "${major}.${minor}" PARENT_SCOPE)
    set("${var}_VERSION_MAJOR" "${major}" PARENT_SCOPE)
    set("${var}_VERSION_MINOR" "${minor}" PARENT_SCOPE)
    set("${var}_VERSION_PATCH" "${patch}" PARENT_SCOPE)
    set("${var}_VERSION_PATCH_EXTRA" "${patch_extra}" PARENT_SCOPE)
    set("${var}_VERSION_FULL" "${full}" PARENT_SCOPE)
    if (patch_extra)
      set("${var}_VERSION_IS_RELEASE" FALSE PARENT_SCOPE)
    else ()
      set("${var}_VERSION_IS_RELEASE" TRUE PARENT_SCOPE)
    endif ()
  endif ()
endfunction()

function (_superbuild_detect_version_git_remote_tag var git_repo git_tag default)
  string(REPLACE "origin/" "" tag "${git_tag}")
  string(REPLACE ".git" "/raw/${tag}/version.txt" version_url "${git_repo}")

  file(DOWNLOAD "${version_url}"
    "${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/${var}Version.txt"
    STATUS status)
  list(GET status 0 error_code)
  if (error_code)
    if (default STREQUAL "")
      set(error_level FATAL_ERROR)
    else ()
      set(error_level WARNING)
    endif ()
    message("${error_level}" "Could not access the version file for ${var}")
    set(output "${default}")
  else()
    file(STRINGS "${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/${var}Version.txt" output)
  endif ()

  _superbuild_parse_version(parsed "${output}")
  set("${var}_VERSION"             "${parsed_VERSION}" PARENT_SCOPE)
  set("${var}_VERSION_MAJOR"       "${parsed_VERSION_MAJOR}" PARENT_SCOPE)
  set("${var}_VERSION_MINOR"       "${parsed_VERSION_MINOR}" PARENT_SCOPE)
  set("${var}_VERSION_PATCH"       "${parsed_VERSION_PATCH}" PARENT_SCOPE)
  set("${var}_VERSION_PATCH_EXTRA" "${parsed_VERSION_PATCH_EXTRA}" PARENT_SCOPE)
  set("${var}_VERSION_FULL"        "${parsed_VERSION_FULL}" PARENT_SCOPE)
  set("${var}_VERSION_IS_RELEASE"  "${parsed_VERSION_IS_RELEASE}" PARENT_SCOPE)
endfunction ()

function (_superbuild_detect_version_git var source_dir default version_file)
  set(major)
  set(minor)
  set(patch)
  set(full)
  set(patch_extra)
  set(result -1)

  if (GIT_FOUND AND source_dir)
    execute_process(
      COMMAND         "${GIT_EXECUTABLE}"
                      rev-parse
                      --is-inside-work-tree
      RESULT_VARIABLE result
      OUTPUT_VARIABLE output
      WORKING_DIRECTORY "${source_dir}"
      ERROR_QUIET
      OUTPUT_STRIP_TRAILING_WHITESPACE)

    if (NOT result)
      execute_process(
        COMMAND         "${GIT_EXECUTABLE}"
                        describe
                        --tags
        RESULT_VARIABLE result
        OUTPUT_VARIABLE output
        WORKING_DIRECTORY "${source_dir}"
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE)
    endif ()

    # Get branch information (if `describe` worked).
    set(branch "")
    if (NOT result)
      execute_process(
        COMMAND "${GIT_EXECUTABLE}"
                name-rev
                --name-only
                --no-undefined          # error if these names don't work
                --refs=refs/tags/*      # tags
                --refs=refs/heads/*     # branches
                --refs=refs/pipelines/* # CI
                HEAD
        WORKING_DIRECTORY ${source_dir}
        RESULT_VARIABLE name_rev_result
        OUTPUT_VARIABLE name_rev_output
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_STRIP_TRAILING_WHITESPACE)
      if (NOT name_rev_result)
        set(branch "${name_rev_output}")
      endif ()
    endif ()
  endif ()

  # If `git describe` failed, check for a version file.
  if (result)
    set(version_path "${source_dir}/${version_file}")
    if (source_dir AND version_file AND EXISTS "${version_path}")
      # Read the first line from the version file as the version number.
      file(STRINGS "${version_path}" output
        LIMIT_COUNT 1)
    else ()
      set(output "${default}")
    endif ()
  endif ()

  _superbuild_parse_version(parsed ${output})
  message(STATUS "Determined source version for ${var}: ${parsed_VERSION_FULL}")

  set("${var}_VERSION"             "${parsed_VERSION}" PARENT_SCOPE)
  set("${var}_VERSION_MAJOR"       "${parsed_VERSION_MAJOR}" PARENT_SCOPE)
  set("${var}_VERSION_MINOR"       "${parsed_VERSION_MINOR}" PARENT_SCOPE)
  set("${var}_VERSION_PATCH"       "${parsed_VERSION_PATCH}" PARENT_SCOPE)
  set("${var}_VERSION_PATCH_EXTRA" "${parsed_VERSION_PATCH_EXTRA}" PARENT_SCOPE)
  set("${var}_VERSION_FULL"        "${parsed_VERSION_FULL}" PARENT_SCOPE)
  set("${var}_VERSION_IS_RELEASE"  "${parsed_VERSION_IS_RELEASE}" PARENT_SCOPE)
  if (parsed_VERSION_PATCH_EXTRA)
    set("${var}_VERSION_BRANCH"    "${branch}" PARENT_SCOPE)
  else ()
    set("${var}_VERSION_BRANCH"    "" PARENT_SCOPE)
  endif ()
endfunction ()

#[==[.md
```
superbuild_set_version_variables(<PROJECT> <DEFAULT> <INCLUDE FILE> [<VERSION FILE>])
```

This will write out a file to `<INCLUDE FILE>` which may be `include`'d after
this function to set variables related to the versions of the given
`<PROJECT>`. If the version cannot be determined (e.g., because the project
will be cloned during the build), the given `DEFAULT` version will be used.

If there is a source directory to be used for the project, the `<VERSION FILE>`
will be used to get the version number. If it is empty or not provided, the
default will be used instead.

The variables available after inclusion are:

 `<PROJECT>_version` (as `<major>.<minor>`)
 `<PROJECT>_version_major`
 `<PROJECT>_version_minor`
 `<PROJECT>_version_patch`
 `<PROJECT>_version_patch_extra` (e.g., `rc1`)
 `<PROJECT>_version_suffix` (equivalent to `-<patch_extra>` if `patch_extra` is non-empty)
 `<PROJECT>_version_full`
 `<PROJECT>_version_is_release` (`TRUE` if the suffix is empty, `FALSE` otherwise)
 `<PROJECT>_version_branch` (the "branch" name from Git if available, empty otherwise)
#]==]
function (superbuild_set_version_variables project default include_file)
  set(source_dir "")
  if ((NOT ${project}_FROM_GIT AND ${project}_FROM_SOURCE_DIR) OR ${project}_SOURCE_SELECTION STREQUAL "source")
    set(source_dir "${${project}_SOURCE_DIR}")
  elseif (${project}_SOURCE_SELECTION STREQUAL "git")
    set(source_dir "${CMAKE_BINARY_DIR}/superbuild/${project}/src")
  endif ()
  _superbuild_detect_version_git("${project}" "${source_dir}" "${default}" "${ARGV3}")

  _superbuild_set_up("${project}_version" "${${project}_VERSION}")
  _superbuild_set_up("${project}_version_major" "${${project}_VERSION_MAJOR}")
  _superbuild_set_up("${project}_version_minor" "${${project}_VERSION_MINOR}")
  _superbuild_set_up("${project}_version_patch" "${${project}_VERSION_PATCH}")
  _superbuild_set_up("${project}_version_patch_extra" "${${project}_VERSION_PATCH_EXTRA}")
  if (${project}_version_patch_extra)
    _superbuild_set_up("${project}_version_suffix" "-${${project}_version_patch_extra}")
  else ()
    _superbuild_set_up("${project}_version_suffix" "")
  endif ()
  _superbuild_set_up("${project}_version_full" "${${project}_VERSION_FULL}")
  _superbuild_set_up("${project}_version_is_release" "${${project}_VERSION_IS_RELEASE}")
  _superbuild_set_up("${project}_version_branch" "${${project}_VERSION_BRANCH}")

  if (include_file)
    file(WRITE "${_superbuild_module_gen_dir}/${include_file}" "")
    foreach (variable IN ITEMS "" _major _minor _patch _patch_extra _suffix _full _is_release _branch)
      file(APPEND "${_superbuild_module_gen_dir}/${include_file}"
        "set(\"${project}_version${variable}\" \"${${project}_version${variable}}\")\n")
    endforeach ()
  endif ()
endfunction ()

function (superbuild_configure_project_version project)
  set(default_version)
  set(is_git_source_selection OFF)
  if (${project}_SOURCE_SELECTION STREQUAL "git")
    set(is_git_source_selection ON)
  elseif (ARGC GREATER 1)
    foreach (arg IN LISTS ARGN)
      if (${project}_SOURCE_SELECTION IN_LIST arg)
        set(is_git_source_selection ON)
      endif ()
    endforeach ()
  endif ()

  if (is_git_source_selection)
    set("${project}_VERSION_DEFAULT" "<FETCH_FROM_GIT>"
      CACHE STRING "The default version of ${project} to use if it cannot be detected")
    mark_as_advanced("${project}_VERSION_DEFAULT")
    if (${project}_VERSION_DEFAULT STREQUAL "<FETCH_FROM_GIT>")
      _superbuild_detect_version_git_remote_tag(${project} "${${project}_GIT_REPOSITORY}" "${${project}_GIT_TAG}" "")
      _superbuild_set_up("${project}_VERSION_DEFAULT" "${${project}_VERSION_MAJOR}.${${project}_VERSION_MINOR}.${${project}_VERSION_PATCH}")
    endif ()
    _superbuild_set_up(default_version "${${project}_VERSION_DEFAULT}")
  elseif (${project}_SOURCE_SELECTION STREQUAL "source")
    set(default_version "")
  else ()
    set(default_version "${${project}_SOURCE_SELECTION}")
  endif()
  superbuild_set_version_variables(${project} "${default_version}" "${project}-version.cmake" "version.txt")
  _superbuild_set_up("${project}_version" "${${project}_version_major}.${${project}_version_minor}")
  _superbuild_set_up("${project}_default_version" "${default_version}")
endfunction()
