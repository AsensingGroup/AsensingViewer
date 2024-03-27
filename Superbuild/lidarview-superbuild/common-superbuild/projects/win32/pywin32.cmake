if (SUPERBUILD_SKIP_PYTHON_PROJECTS)
  superbuild_add_project_python(pywin32
    PACKAGE pywin32
    DEPENDS pythonsetuptools)
else ()
  superbuild_add_project_python_wheel(pywin32)
endif()
