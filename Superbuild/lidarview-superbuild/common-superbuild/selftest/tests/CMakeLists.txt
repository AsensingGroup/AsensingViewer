set(install_dir "${CMAKE_BINARY_DIR}/install")

if (APPLE)
  add_test(
    NAME apple-check-binaries
    COMMAND
      "${CMAKE_COMMAND}"
      "-Dinstall_dir=${install_dir}"
      "-Ddeployment_target=${CMAKE_OSX_DEPLOYMENT_TARGET}"
      "-Darch=${CMAKE_HOST_SYSTEM_PROCESSOR}"
      -P "${CMAKE_CURRENT_SOURCE_DIR}/apple-check.cmake")
endif ()

if (APPLE AND CMAKE_OSX_DEPLOYMENT_TARGET VERSION_LESS "11.0"
    AND (TARGET numpy OR TARGET scipy))
  add_test(
    NAME no-accelerate-framework
    COMMAND
      "${CMAKE_COMMAND}"
      "-Dinstall_dir=${install_dir}"
      "-Dnumpy_enabled=$<TARGET_EXISTS:numpy>"
      "-Dscipy_enabled=$<TARGET_EXISTS:scipy>"
      -P "${CMAKE_CURRENT_SOURCE_DIR}/accelerate-check.cmake")
endif ()

if (UNIX)
  add_test(
    NAME unix-check-lib64
    COMMAND
      "${CMAKE_COMMAND}"
      "-Dinstall_dir=${install_dir}"
      -P "${CMAKE_CURRENT_SOURCE_DIR}/unix-check-lib64.cmake")
endif ()

function (add_python_import_test package module)
  if (NOT TARGET "${package}")
    return ()
  endif ()

  add_test(
    NAME "python3-${package}-import-${module}"
    COMMAND
      "${superbuild_python_executable}"
      -c "import os; [os.add_dll_directory(p) for p in filter(lambda x: x, os.environ.get('PYTHON_WINDOWS_DLL_DIRECTORIES', '').split(';'))]; import ${module}; print(${module}.__file__.replace(os.sep, '/'))")
  if (NOT USE_SYSTEM_${package})
    set_property(TEST "python3-${package}-import-${module}"
      PROPERTY
        PASS_REGULAR_EXPRESSION "${install_dir}")
  endif ()
endfunction ()

function (add_python_script_test package script)
  if (NOT TARGET "${package}")
    return ()
  endif ()

  add_test(
    NAME "python3-${package}-script-${script}"
    COMMAND
      "${superbuild_python_executable}"
      "${CMAKE_CURRENT_SOURCE_DIR}/${script}.py")
endfunction ()

if (TARGET mpi)
  add_test(
    NAME mpi-check
    COMMAND
      "${CMAKE_CTEST_COMMAND}"
        --build-generator "${CMAKE_GENERATOR}"
        --build-and-test "${CMAKE_CURRENT_SOURCE_DIR}/mpi-check" "${CMAKE_CURRENT_BINARY_DIR}/mpi-check"
        --build-options "-DCMAKE_PREFIX_PATH=${install_dir}"
        --test-command "${CMAKE_CTEST_COMMAND}")
endif ()

if (TARGET tbb)
  add_test(
    NAME tbb-check
    COMMAND
      "${CMAKE_CTEST_COMMAND}"
        --build-generator "${CMAKE_GENERATOR}"
        --build-and-test "${CMAKE_CURRENT_SOURCE_DIR}/tbb-check" "${CMAKE_CURRENT_BINARY_DIR}/tbb-check"
        --build-options "-DCMAKE_PREFIX_PATH=${install_dir}")
endif ()

add_python_import_test(python3 this)
add_python_import_test(matplotlib matplotlib)
add_python_script_test(matplotlib matplotlib-render)
add_python_import_test(numpy numpy)
add_python_import_test(pythonaiohttp aiohttp)
add_python_import_test(pythonasynctimeout async_timeout)
add_python_import_test(pythonattrs attr)
add_python_import_test(pythonautobahn autobahn)
add_python_import_test(pythonbeniget beniget)
add_python_import_test(pythoncffi cffi)
add_python_import_test(pythonchardet chardet)
add_python_import_test(pythonconstantly constantly)
add_python_import_test(pythoncppy cppy)
add_python_import_test(pythoncryptography cryptography)
add_python_import_test(pythoncycler cycler)
add_python_import_test(pythoncython cython)
add_python_import_test(pythondateutil dateutil)
add_python_import_test(pythongast gast)
add_python_import_test(pythonhyperlink hyperlink)
add_python_import_test(pythonidna idna)
add_python_import_test(pythonincremental incremental)
add_python_import_test(pythonkiwisolver kiwisolver)
add_python_import_test(pythonmultidict multidict)
add_python_import_test(pythonpillow PIL)
add_python_import_test(pythonpillow PIL.Image)
add_python_import_test(pythonply ply)
add_python_import_test(pythonpycparser pycparser)
add_python_import_test(pythonpygments pygments)
add_python_import_test(pythonpyparsing pyparsing)
add_python_import_test(pythonpythran pythran)
add_python_import_test(pythonsemanticversion semantic_version)
add_python_import_test(pythonsetuptools setuptools)
add_python_import_test(pythonsetuptoolsscm setuptools_scm)
add_python_import_test(pythonsetuptoolsrust setuptools_rust)
add_python_import_test(pythonsix six)
add_python_import_test(pythontoml toml)
add_python_import_test(pythontwisted twisted)
add_python_import_test(pythontxaio txaio)
add_python_import_test(pythontypingextensions typing_extensions)
add_python_import_test(pythonwheel wheel)
add_python_import_test(pythonwslink wslink)
add_python_import_test(pythonwslinkasync wslink)
add_python_import_test(pythonyarl yarl)
add_python_import_test(pythonzope App)
add_python_import_test(pythonzope OFS)
add_python_import_test(pythonzopeinterface zope.interface)
add_python_import_test(pytz pytz)
add_python_import_test(pywin32 adodbapi)
add_python_import_test(scipy scipy)

if (WIN32)
  # Python 3.9 needs help finding dependent DLLs and Pillow doesn't do this on
  # its own, so extra work is needed to make this happy.
  set(pil_using_tests
    "python3-matplotlib-import-matplotlib"
    "python3-matplotlib-script-matplotlib-render"
    "python3-pythonpillow-import-PIL.Image")
  foreach (pil_using_test IN LISTS pil_using_tests)
    if (TEST "${pil_using_test}")
      set_property(TEST "${pil_using_test}" APPEND
        PROPERTY
          ENVIRONMENT "PYTHON_WINDOWS_DLL_DIRECTORIES=${install_dir}/bin")
    endif ()
  endforeach ()
endif ()

# Verify that Python3 support for ssl is removed from a built-in Python if not
# enabled.
add_python_import_test(python3 ssl)
if (NOT TARGET "openssl" AND NOT USE_SYSTEM_python3 AND TEST "python3-python3-import-ssl")
  # Require that the `_ssl` module is not found.
  set_property(TEST "python3-python3-import-ssl"
    PROPERTY
      PASS_REGULAR_EXPRESSION "ModuleNotFoundError: No module named '_ssl'")
endif ()
