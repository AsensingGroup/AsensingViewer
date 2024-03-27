#[==[.md
# Installation support

When creating packages, these functions may be used in order to ensure that the
resulting package includes the required executables as well as all runtime
dependencies. These are discovered using platform-specific utilties to find out
what libraries are required and then emulating the platform's library searching
logic in order to find dependent libraries. For more details, see each
platform's section.

Due to platform differences, each platform has its own set of functions for
use.
#]==]

set(_superbuild_install_cmake_dir "${CMAKE_CURRENT_LIST_DIR}")
include(CMakeParseArguments)

# Find a Python executable to run the `fixup_bundle` scripts.
if (NOT superbuild_python_executable)
  find_package(Python${python_version} COMPONENTS Interpreter)
  if (Python${python_version}_EXECUTABLE)
    set(superbuild_python_executable
      "${Python${python_version}_EXECUTABLE}")
  else ()
    message(FATAL_ERROR
      "Could not find a Python executable newer than "
      "${superbuild_python_version}; one is required to create packages.")
  endif ()
endif ()

# TODO: The functions in these files should be grouped and made OS-agnostic.
#       Keyword arguments should be used more and be uniform across all
#       platforms.

if (WIN32)
  include("${CMAKE_CURRENT_LIST_DIR}/SuperbuildInstallMacros-win32.cmake")
elseif (APPLE)
  include("${CMAKE_CURRENT_LIST_DIR}/SuperbuildInstallMacros-apple.cmake")
elseif (UNIX)
  include("${CMAKE_CURRENT_LIST_DIR}/SuperbuildInstallMacros-unix.cmake")
else ()
  # Unsupported platform.
endif ()
