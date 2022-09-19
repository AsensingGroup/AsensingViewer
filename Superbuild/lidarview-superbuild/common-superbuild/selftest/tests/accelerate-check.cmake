macro (otool library)
  execute_process(
    COMMAND
      otool
      -L
      "${library}"
    RESULT_VAR res
    OUTPUT_VARIABLE out
    ERROR_VARIABLE err)
endmacro ()

set(accelerate_using_modules)

if (numpy_enabled)
  file(GLOB numpy_linalg_modules "${install_dir}/lib/python*/site-packages/numpy/linalg/*.so")
  if (NOT numpy_linalg_modules)
    message(SEND_ERROR
      "Failed to find any NumPy modules?")
  endif ()
  list(APPEND accelerate_using_modules
    ${numpy_linalg_modules})
endif ()

if (scipy_enabled)
  file(GLOB scipy_linalg_modules "${install_dir}/lib/python*/site-packages/scipy-*.egg/scipy/*/*.so")
  if (NOT scipy_linalg_modules)
    message(SEND_ERROR
      "Failed to find any SciPy modules?")
  endif ()
  list(APPEND accelerate_using_modules
    ${scipy_linalg_modules})
endif ()

foreach (module IN LISTS accelerate_using_modules)
  otool("${module}")
  if (res)
    message(SEND_ERROR
      "Failed to run `otool` on ${module}: ${err}")
  endif ()

  if (out MATCHES "Accelerate.framework")
    message(SEND_ERROR
      "The `${module} module links against `Accelerate.framework`. This ends "
      "up not working well. Investigate the build of the affected module.")
  endif ()
endforeach ()
