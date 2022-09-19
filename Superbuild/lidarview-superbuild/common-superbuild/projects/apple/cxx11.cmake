include("${CMAKE_CURRENT_LIST_DIR}/../cxx11.cmake")

superbuild_append_flags(cxx_flags "-stdlib=libc++")
