set(pythonpillow_process_environment)
if (WIN32)
  list(APPEND pythonpillow_process_environment
    INCLUDE "<INSTALL_DIR>/include"
    LIB     "<INSTALL_DIR>/lib")
endif ()

superbuild_add_project_python(pythonpillow
  PACKAGE PIL
  DEPENDS pythonsetuptools libjpegturbo
  PROCESS_ENVIRONMENT
    ${pythonpillow_process_environment})
