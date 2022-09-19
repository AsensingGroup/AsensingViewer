superbuild_add_project_python(pythonpandas
  PACKAGE pandas
  DEPENDS
    numpy
    pythoncython
    pythondateutil
    pythonsetuptools
    pythonsix
    pytz
    zlib
  DEPENDS_OPTIONAL
    matplotlib)
