superbuild_add_project(pythonqt
  DEPENDS qt5 python3
  CMAKE_ARGS
	-DCMAKE_BUILD_TYPE=Release
  -DBUILD_SHARED_LIBS:BOOL=OFF
  
  -DPythonQt_QT_VERSION:STRING=${qt_version}

  -DPythonQt_Wrap_Qtcore:BOOL=ON
  -DPythonQt_Wrap_Qtgui:BOOL=ON
  -DPythonQt_Wrap_Qtwidgets:BOOL=ON
  -DPythonQt_Wrap_Qtuitools:BOOL=ON

  -DCMAKE_MACOSX_RPATH:BOOL=OFF
)

superbuild_add_extra_cmake_args(
  -DPYTHONQT_INCLUDE_DIR:PATH=<INSTALL_DIR>/include/PythonQt
)
