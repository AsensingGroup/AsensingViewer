cmake_minimum_required(VERSION 3.12)
project(mpi-check C)

message(CTEST_FULL_OUTPUT)

function (check_target target)
  if (NOT TARGET "${target}")
    message(SEND_ERROR
      "Target `${target}` not defined.")
  endif ()
endfunction ()

find_package(MPI REQUIRED
  COMPONENTS C)

check_target(MPI::MPI_C)

add_executable(mpitest mpi.c)
target_link_libraries(mpitest PRIVATE MPI::MPI_C)
target_compile_options(mpitest
  PRIVATE
    # Detect bogus `-framework` flag settings eating another flag that is
    # important.
    "$<$<C_COMPILER_ID:Clang,AppleClang>:-Werror=unused-command-line-argument>")

include(CTest)
enable_testing()

add_test(NAME mpitest COMMAND mpitest)
