set(boost_toolset_option)
set(boost_platform_options)
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND
    NOT (CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang" AND
         CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 10.0.1.10010046))
  # Set the toolset to be clang if using it.
  list(APPEND boost_toolset_option
    toolset=clang)
elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  # Set the toolset to be GCC if using it.
  list(APPEND boost_toolset_option
    toolset=gcc)
elseif (APPLE AND CMAKE_OSX_DEPLOYMENT_TARGET)
  foreach (boost_flagset IN ITEMS cflags cxxflags mflags mmflags linkflags)
    list(APPEND boost_platform_options
      "${boost_flagset}=-mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET}")
  endforeach ()
endif ()

list(APPEND boost_platform_options
  ${boost_toolset_option})
if (cxx11_enabled)
  list(APPEND boost_platform_options
    cxxflags=${CMAKE_CXX11_STANDARD_COMPILE_OPTION})
endif ()

if (NOT APPLE)
  list(APPEND boost_platform_options
    hardcode-dll-paths=true
    "dll-path='\\$ORIGIN'")
endif ()

include(boost.common)

if (APPLE)
  superbuild_apply_patch(boost osx-rpath
    "Remove @rpath from the install name of Boost's libraries")

  superbuild_project_add_step(fix-install-names
    COMMAND   "${CMAKE_COMMAND}"
              "-Dlibdir:PATH=<INSTALL_DIR>/lib"
              -P "${CMAKE_CURRENT_LIST_DIR}/scripts/boost.fix-install-names.cmake"
    DEPENDEES install
    COMMENT   ""
    WORKING_DIRECTORY <INSTALL_DIR>/lib)

  # Patch reqiured to build boost 1.71 on macOS 10.13
  cmake_host_system_information(RESULT os_release QUERY OS_RELEASE)
  if (os_release VERSION_GREATER "10.13" AND os_release VERSION_LESS "10.14")
    superbuild_apply_patch(boost-iostreams pull103
      "Patch boost for macOS 10.13"
    )
  endif()
endif ()
