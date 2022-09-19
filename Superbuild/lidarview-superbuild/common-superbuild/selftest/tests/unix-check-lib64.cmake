if (NOT EXISTS "${install_dir}/lib64")
  return ()
endif ()

message("CTEST_FULL_OUTPUT # Avoid truncation")

execute_process(
  COMMAND ls -R "${install_dir}/lib64")

message(FATAL_ERROR
  "`lib64` should not be used in the superbuild; all projects "
  "should install to `lib` regardless of architecture.")
