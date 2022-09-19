cmake_minimum_required(VERSION 3.0)

project(standalone-boost)

function (superbuild_find_projects var)
  set(projects
    boost
    cxx11
    zlib)

  set("${var}"
    ${projects}
    PARENT_SCOPE)
endfunction ()

function (superbuild_sanity_check)
  if (NOT boost_enabled)
    message(FATAL_ERROR "Boost must be enabled.")
  endif ()
endfunction ()

set(_superbuild_default_boost ON)

set(boost_libraries ""
  CACHE STRING "The list of Boost libraries to build.")

set(boost_install_location ""
  CACHE PATH "Location to install Boost to.")
if (NOT boost_install_location)
  message(FATAL_ERROR "boost_install_location is not set!")
endif ()
set(superbuild_install_location "${boost_install_location}")

get_filename_component(sb_dir "${CMAKE_CURRENT_LIST_DIR}" DIRECTORY)
add_subdirectory("${sb_dir}" "${CMAKE_CURRENT_BINARY_DIR}/superbuild")
