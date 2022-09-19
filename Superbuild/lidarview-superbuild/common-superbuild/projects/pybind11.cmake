if (MSVC AND (MSVC_VERSION LESS 1900) AND pybind11_enabled)
  message(FATAL_ERROR
    "Visual Studio 2015 or later is required to use pybind11.")
endif ()

superbuild_add_project(pybind11
  DEPENDS python3 cxx11 pythonsetuptools
  CMAKE_ARGS
    -DPYBIND11_TEST:BOOL=OFF)

if (WIN32)
  set(pybind11_python_args
    "--prefix=Python")
else ()
  set(pybind11_python_args
    "--single-version-externally-managed"
    "--install-lib=lib/python${superbuild_python_version}/site-packages"
    "--prefix=")
endif ()

superbuild_project_add_step(pybind11-pip-install
  COMMAND   "${superbuild_python_executable}"
            setup.py
            install
            --root=<INSTALL_DIR>
            ${pybind11_python_args}
  DEPENDEES install
  COMMENT   "Install pybind11 for pip"
  WORKING_DIRECTORY <SOURCE_DIR>)
