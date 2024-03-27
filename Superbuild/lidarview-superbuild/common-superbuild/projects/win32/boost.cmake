set(boost_platform_options)

# 8.0 and below unsupported anyways.
if (NOT MSVC_VERSION VERSION_GREATER 1400)
  message(FATAL_ERROR "At least Visual Studio 9.0 is required")
elseif (NOT MSVC_VERSION VERSION_GREATER 1500)
  set(msvc_ver 9.0)
elseif (NOT MSVC_VERSION VERSION_GREATER 1600)
  set(msvc_ver 10.0)
elseif (NOT MSVC_VERSION VERSION_GREATER 1700)
  set(msvc_ver 11.0)
elseif (NOT MSVC_VERSION VERSION_GREATER 1800)
  set(msvc_ver 12.0)
elseif (NOT MSVC_VERSION VERSION_GREATER 1900)
  set(msvc_ver 14.0)
elseif (NOT MSVC_VERSION VERSION_GREATER 1919)
  set(msvc_ver 14.1)
elseif (NOT MSVC_VERSION VERSION_GREATER 1930)
  set(msvc_ver 14.2)
else ()
  message(FATAL_ERROR "Unrecognized MSVC version: ${MSVC_VERSION}")
endif ()

list(APPEND boost_platform_options
  "--toolset=msvc-${msvc_ver}"
  address-model=64)

include(boost.common)

superbuild_project_add_step(boost-copylibs
  COMMAND   "${CMAKE_COMMAND}"
            -Dinstall_location:PATH=<INSTALL_DIR>
            -P "${CMAKE_CURRENT_LIST_DIR}/scripts/boost.copylibs.cmake"
  DEPENDEES install
  COMMENT   "Copy .dll files to the bin/ directory"
  WORKING_DIRECTORY <SOURCE_DIR>)
