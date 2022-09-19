superbuild_add_project_python(pythonpythran
  PACKAGE pythran
  DEPENDS pythonsetuptools pythongast pythonbeniget pythonply)

superbuild_apply_patch(pythonpythran no-pytest
  "Remove pytest requirement")
