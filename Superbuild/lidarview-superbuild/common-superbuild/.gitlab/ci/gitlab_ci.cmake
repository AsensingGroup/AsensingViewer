# Set up the source and build paths.
set(CTEST_SOURCE_DIRECTORY "$ENV{CI_PROJECT_DIR}/selftest")
set(CTEST_SITE "gitlab-ci")

set(CTEST_BINARY_DIRECTORY "$ENV{CI_PROJECT_DIR}/build")

if ("$ENV{CMAKE_CONFIGURATION}" STREQUAL "")
  message(FATAL_ERROR
    "The CMAKE_CONFIGURATION environment variable is required to know what "
    "cache initialization file to use.")
endif ()

# Set the build metadata.

set(build_name_prefix)
if (DEFINED "ENV{CI_MERGE_REQUEST_IID}")
  set(build_name_prefix "mr-!$ENV{CI_MERGE_REQUEST_IID}-")
endif()

set(CTEST_BUILD_NAME "$ENV{CI_PROJECT_NAME}-${build_name_prefix}[$ENV{CMAKE_CONFIGURATION}]")

# Default to Release builds.
if (NOT "$ENV{CMAKE_BUILD_TYPE}" STREQUAL "")
  set(CTEST_BUILD_CONFIGURATION "$ENV{CMAKE_BUILD_TYPE}")
endif ()
if (NOT CTEST_BUILD_CONFIGURATION)
  set(CTEST_BUILD_CONFIGURATION "Release")
endif ()

# Default to using Ninja.
if (NOT "$ENV{CMAKE_GENERATOR}" STREQUAL "")
  set(CTEST_CMAKE_GENERATOR "$ENV{CMAKE_GENERATOR}")
endif ()
if (NOT CTEST_CMAKE_GENERATOR)
  set(CTEST_CMAKE_GENERATOR "Ninja")
endif ()

# Determine the track to submit to.
set(ctest_track "Experimental")
if (NOT "$ENV{CI_MERGE_REQUEST_ID}" STREQUAL "")
  if ("$ENV{CI_MERGE_REQUEST_TARGET_BRANCH_NAME}" STREQUAL "master")
    set(ctest_track "merge-requests")
  endif ()
elseif ("$ENV{CI_PROJECT_PATH}" STREQUAL "paraview/common-superbuild")
  if ("$ENV{CI_COMMIT_REF_NAME}" STREQUAL "master")
    set(ctest_track "master")
  endif ()
endif ()
