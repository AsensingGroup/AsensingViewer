set(pythonsetuptools_depends)
if (WIN32)
  list(APPEND pythonsetuptools_depends
    pywin32)
endif ()

superbuild_add_project_python(pythonsetuptools
  PACKAGE setuptools ${pythonsetuptools_depends})
