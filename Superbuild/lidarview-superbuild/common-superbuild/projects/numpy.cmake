# Check to see if the build path is too short for the packages.
if (numpy_enabled AND UNIX AND NOT APPLE)
  string(LENGTH "${CMAKE_BINARY_DIR}" numpy_bindir_len)
  # Emperically determined. If longer paths still have the issue, raise this limit.
  if (numpy_bindir_len LESS 24)
    message(WARNING
      "Note that your build tree (${CMAKE_BINARY_DIR}) is too short for "
      "packaging (due to limited RPATH space in the header). Please use a "
      "longer build directory to avoid this problem. You may ignore it if you "
      "are not building packages.")
  endif ()
endif ()

set(numpy_process_environment)
if (lapack_enabled)
  list(APPEND numpy_process_environment
    BLAS    "<INSTALL_DIR>"
    LAPACK  "<INSTALL_DIR>"
    NPY_BLAS_ORDER blas
    NPY_LAPACK_ORDER lapack)
else()
  list(APPEND numpy_process_environment
    BLAS    "None"
    LAPACK  "None")
endif ()

if (fortran_enabled)
  list(APPEND numpy_process_environment
    FC "${CMAKE_Fortran_COMPILER}")
endif ()

set(numpy_fortran_compiler "no")
if (fortran_enabled)
  set(numpy_fortran_compiler "${CMAKE_Fortran_COMPILER}")
endif ()

set(numpy_python_build_args
  "--fcompiler=${numpy_fortran_compiler}")

set(numpy_depends_optional)
if (NOT WIN32)
  set(numpy_depends_optional fortran lapack)
endif()

superbuild_add_project_python(numpy
  PACKAGE numpy
  CAN_USE_SYSTEM
  DEPENDS
    pythoncython
  DEPENDS_OPTIONAL ${numpy_depends_optional}
  PROCESS_ENVIRONMENT
    MKL         "None"
    ATLAS       "None"
    ${numpy_process_environment})
