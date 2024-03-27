if (CMAKE_MINIMUM_REQUIRED_VERSION VERSION_LESS "3.18")
  cmake_minimum_required(VERSION 3.18 FATAL_ERROR)
endif ()

if (POLICY CMP0114)
  cmake_policy(SET CMP0114 NEW)
endif ()

if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  message(FATAL_ERROR
    "This project is not meant to be run directly, but instead used via "
    "add_subdirectory.")
elseif (CMAKE_CONFIGURATION_TYPES)
  # These generators aren't tested at all and non-CMake subprojects do not
  # change their configuration properly without a reconfigure. This could
  # change in the future, but it is not in the plan right now.
  message(FATAL_ERROR
    "The superbuild does not currently support multi-config generators "
    "(such as Visual Studio or Xcode). The recommended generators are "
    "Ninja and Unix Makefiles, but NMake Makefiles should work as well.")
elseif (NOT COMMAND superbuild_find_projects)
  message(FATAL_ERROR
    "The superbuild_find_projects function is not implemented; no projects "
    "can be enabled.")
endif ()

list(INSERT CMAKE_MODULE_PATH 0
  "${CMAKE_CURRENT_LIST_DIR}/cmake")
include("SuperbuildUtils")
include("SuperbuildCrossMacros")

_superbuild_check_up_to_date()

set(platform)
if (APPLE)
  set(platforms "apple" "apple-unix")
elseif (UNIX)
  set(platforms "unix" "apple-unix")
elseif (WIN32)
  set(platforms "win32")
else ()
  message(FATAL_ERROR "unsupported platform")
endif ()

superbuild_detect_64bit_target()

# Setup shared library defaults.  If explicitly specified somehow, then default
# to that.  Otherwise base the default on whether or not shared libs are even
# supported.
if (NOT _superbuild_no_static_everywhere)
  if (NOT DEFINED BUILD_SHARED_LIBS)
    get_property(superbuild_shared_libs_supported
      GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS)
    cmake_dependent_option(BUILD_SHARED_LIBS
      "Build shared libraries (so/dylib/dll)." "${superbuild_shared_libs_supported}"
      "superbuild_shared_libs_supported" OFF)
    mark_as_advanced(BUILD_SHARED_LIBS)
  endif ()
else ()
  set(BUILD_SHARED_LIBS ON)
endif ()

# Setup PIC defaults.
cmake_dependent_option(CMAKE_POSITION_INDEPENDENT_CODE
  "Build with Position Independent Code" ON
  "superbuild_shared_libs_supported" OFF)
mark_as_advanced(CMAKE_POSITION_INDEPENDENT_CODE)

# Verify shared libs and PIC compatibility
if(BUILD_SHARED_LIBS AND NOT CMAKE_POSITION_INDEPENDENT_CODE)
  if (NOT _superbuild_no_static_everywhere)
    message(WARNING "Forcing CMAKE_POSITION_INDEPENDENT_CODE to be ON as required by BUILD_SHARED_LIBS.")
  endif ()
  set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()

#-----------------------------------------------------------------------------
# Setup some standard variables that control various locations and flags.
if (NOT superbuild_install_location)
  set(superbuild_install_location "${CMAKE_BINARY_DIR}/install")
endif ()

if (NOT IS_ABSOLUTE "${superbuild_install_location}")
  message(WARNING
    "The given install location `${superbuild_install_location}` is relative; "
    "replacing with an absolute path.")
  get_filename_component(superbuild_install_location
    "${superbuild_install_location}" ABSOLUTE)
endif ()

set(superbuild_prefix_path "${superbuild_install_location}")
set(superbuild_download_location "${CMAKE_BINARY_DIR}/downloads"
  CACHE PATH "Location for downloaded source tarballs")
mark_as_advanced(superbuild_download_location)

if(BUILD_SHARED_LIBS AND CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
  string(APPEND superbuild_ld_flags " -shared-intel")
endif()
superbuild_setup_flags()

# merge in default/user-specified CMake flags.
string(PREPEND superbuild_ld_flags "${CMAKE_SHARED_LINKER_FLAGS} ")
string(PREPEND superbuild_cpp_flags "${CMAKE_CXX_FLAGS} ")
string(PREPEND superbuild_cxx_flags "${CMAKE_CXX_FLAGS} ")
string(PREPEND superbuild_c_flags "${CMAKE_C_FLAGS} ")
string(PREPEND superbuild_f_flags "${CMAKE_Fortran_FLAGS} ")

foreach (var IN ITEMS ld_flags cpp_flags cxx_flags c_flags f_flags)
  string(STRIP "${superbuild_${var}}" "superbuild_${var}")
endforeach ()

superbuild_prepare_build_tree()

#-----------------------------------------------------------------------------
# Setup CMAKE_MODULE_PATH so that platform specific configurations are
# processed before the generic ones.
set(project_path)
list(APPEND superbuild_project_roots
  "${CMAKE_CURRENT_LIST_DIR}/projects")
foreach (root IN LISTS superbuild_project_roots)
  foreach (platform IN LISTS platforms)
    list(APPEND project_path
      "${root}/${platform}")
  endforeach ()
  list(APPEND project_path
    "${root}/common"
    "${root}")
endforeach ()
list(INSERT CMAKE_MODULE_PATH 0
  ${project_path})

if (CMAKE_CROSSCOMPILING AND
    COMMAND superbuild_cross_prepare_target)
  superbuild_cross_prepare_target()
endif ()

include(SuperbuildRevisionMacros)

# Gather version information.
list(APPEND superbuild_version_files
  "${CMAKE_CURRENT_LIST_DIR}/versions.cmake")
foreach (version_file IN LISTS superbuild_version_files)
  include("${version_file}")
endforeach ()

if (superbuild_package_roots)
  include(SuperbuildPackageMacros)

  _superbuild_package_discover()
  _superbuild_package_select()
endif ()

if (COMMAND superbuild_setup_variables)
  superbuild_setup_variables()
endif ()

include(SuperbuildMacros)
include(SuperbuildPluginMacros)

superbuild_find_projects(superbuild_projects)
list(APPEND superbuild_projects ${superbuild_extra_package_projects})

if (NOT superbuild_projects)
  message(FATAL_ERROR "No projects are configured!")
endif ()

# Discover the projects.
_superbuild_discover_projects(${superbuild_projects})
# Enable projects, etc.
superbuild_process_dependencies()

# Do any sanity checking necessary here.
if (COMMAND superbuild_sanity_check)
  superbuild_sanity_check()
endif ()

# Add packaging.
if (COMMAND superbuild_add_packaging)
  include(SuperbuildPackageMacros)

  set(superbuild_export_variables)
  if (superbuild_python_executable)
    list(APPEND superbuild_export_variables
      superbuild_python_executable)
  endif ()

  if (python_version)
    list(APPEND superbuild_export_variables
      python_version)
  endif ()

  if (superbuild_python_version)
    list(APPEND superbuild_export_variables
      superbuild_python_version)
  endif ()

  superbuild_add_packaging()
endif ()

# Add any tests.
cmake_dependent_option(BUILD_TESTING "Build testing" ON
  "COMMAND superbuild_add_tests" OFF)
if (BUILD_TESTING)
  if (COMMAND superbuild_setup_tests)
    superbuild_setup_tests()
  endif ()

  # Enable testing support.
  include(CTest)
  configure_file(
    "${CMAKE_CURRENT_LIST_DIR}/cmake/CTestCustom.cmake.in"
    "${CMAKE_BINARY_DIR}/CTestCustom.cmake"
    @ONLY)

  file(RELATIVE_PATH superbuild_relative_dir
    "${CMAKE_BINARY_DIR}"
    "${CMAKE_CURRENT_BINARY_DIR}")
  configure_file(
    "${CMAKE_CURRENT_LIST_DIR}/cmake/CTestTestfile.cmake.in"
    "${CMAKE_BINARY_DIR}/CTestTestfile.cmake"
    @ONLY)

  include(SuperbuildTestingMacros)

  superbuild_add_tests()

  get_property(superbuild_test_projects GLOBAL
    PROPERTY superbuild_test_projects)
  foreach (project IN LISTS superbuild_test_projects)
    cmake_dependent_option("TEST_${project}" "Run ${project} test from the superbuild" ON
      "${project}_enabled;TARGET ${project};NOT DEVELOPER_MODE_${project}" OFF)
    if (TEST_${project})
      set(binary_dir "<BINARY_DIR>")
      _ep_replace_location_tags("${project}" binary_dir)
      list(APPEND superbuild_extra_ctest_dirs
        "${binary_dir}")
    endif ()
  endforeach ()

  configure_file(
    "${CMAKE_CURRENT_LIST_DIR}/cmake/superbuild_testing_trampoline.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/superbuild_testing_trampoline.cmake"
    @ONLY)
  set_property(
    DIRECTORY "${CMAKE_CURRENT_LIST_DIR}"
    PROPERTY
      TEST_INCLUDE_FILE "${CMAKE_CURRENT_BINARY_DIR}/superbuild_testing_trampoline.cmake")
endif ()

if (python3_enabled)
  include(python3.functions)
  superbuild_python_write_reqs("${CMAKE_BINARY_DIR}/requirements.txt")
endif ()
