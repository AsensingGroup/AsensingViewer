get_property(pythoncryptography_source GLOBAL
  PROPERTY pythoncryptography_source)

if (pythoncryptography_source)
  superbuild_add_project_python(pythoncryptography
    PACKAGE cryptography
    DEPENDS pythonsetuptools pythoncffi pythonsetuptoolsrust pythonsemanticversion)
else ()
  superbuild_add_project_python_wheel(pythoncryptography
    DEPENDS pythonsetuptools pythoncffi pythonpycparser pythontoml)
endif ()
