set(qt5_SKIP_PCH "ON" CACHE BOOL "")

# Linux replaces `CC` and `CXX`
if (NOT "$ENV{CI_JOB_NAME}" MATCHES "linux" AND
    # Something doesn't work when using sccache.
    NOT "$ENV{CI_JOB_NAME}" MATCHES "windows")
  set(CMAKE_C_COMPILER_LAUNCHER "sccache" CACHE STRING "")
  set(CMAKE_CXX_COMPILER_LAUNCHER "sccache" CACHE STRING "")
endif ()
