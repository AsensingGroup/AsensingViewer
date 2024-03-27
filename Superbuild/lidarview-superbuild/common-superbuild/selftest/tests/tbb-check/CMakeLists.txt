cmake_minimum_required(VERSION 3.12)
project(tbb-check NONE)

message(CTEST_FULL_OUTPUT)

function (check_target target)
  if (NOT TARGET "${target}")
    message(SEND_ERROR
      "Target `${target}` not defined.")
  endif ()
endfunction ()

find_package(TBB CONFIG REQUIRED
  COMPONENTS tbb tbbmalloc)

check_target(TBB::tbb)
check_target(TBB::tbbmalloc)
