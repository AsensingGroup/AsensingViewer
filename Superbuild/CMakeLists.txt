cmake_minimum_required(VERSION 3.20.3)

project(LidarViewSuperBuild)

set(CMAKE_BUILD_TYPE "Release")
#set(SUPERBUILD_OFFLINE_BUILD ON)
set(WITH_STATIC_LASZIP OFF)
#set(SUPERBUILD_ALLOW_DEBUG ON)

#-----------------------------------------------------------------------------
# All the function and variable bellow are here because we rely on lidarview's
# superbuild.
# To see which function/variable cana/needs to be set, please take a look at
# the CMakeLists.txt inside "lidarview-superbuild"
#-----------------------------------------------------------------------------

option(INTERPRETER_ENABLE_Asensing "Enable Asensing interpreter " ON)

list(APPEND superbuild_project_roots
  "${CMAKE_CURRENT_SOURCE_DIR}/Projects")

function (add_project_to_superbuild var)
  # list cannot append to parent's scope so we do it in two steps
  list(APPEND "${var}" lidarview)
  set("${var}" "${${var}}" PARENT_SCOPE)
endfunction ()

list(APPEND superbuild_version_files
  "${CMAKE_CURRENT_LIST_DIR}/versions.cmake")

add_subdirectory(lidarview-superbuild/)
