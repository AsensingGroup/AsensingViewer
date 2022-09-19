string(REPLACE "${list_separator}" "\;" superbuild_python_executable "${superbuild_python_executable}")

message("Running `setup.py install` the first time")
message(STATUS "Running `setup.py install` the first time")

message("Command to run:
  ${superbuild_python_executable}
  setup.py
  install
  --prefix=${install_dir}")

# First attempt.
execute_process(
  COMMAND
    ${superbuild_python_executable}
      setup.py
      install
      "--prefix=${install_dir}"
  RESULT_VARIABLE res)
if (NOT res)
  message(STATUS
    "XXXXXX SciPy install worked with one pass! Please remove this horrible hack.")
  return ()
endif ()

message("Running `setup.py install` a second time")
message(STATUS "Running `setup.py install` a second time")

# Second attempt.
execute_process(
  COMMAND
    ${superbuild_python_executable}
      setup.py
      install
      "--prefix=${install_dir}"
  RESULT_VARIABLE res)
if (res)
  message(FATAL_ERROR
    "Failed to install SciPy after two install passes")
endif ()
