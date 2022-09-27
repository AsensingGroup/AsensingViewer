
#set(PandarGeneralRaw ${CMAKE_CURRENT_SOURCE_DIR}/AsensingLidar_General_SDK/src/PandarGeneralRaw)

set(asensingplugin_xml
  ${CMAKE_CURRENT_SOURCE_DIR}/AsensingPacketInterpreter/AsensingPacketInterpreter.xml
  #${CMAKE_CURRENT_SOURCE_DIR}/AsensingGeneralPacketInterpreter/AsensingGeneralPacketInterpreter.xml
  )

set(asensingplugin_classes
  ${CMAKE_CURRENT_SOURCE_DIR}/AsensingPacketInterpreter/vtkAsensingPacketInterpreter
  #${CMAKE_CURRENT_SOURCE_DIR}/AsensingGeneralPacketInterpreter/vtkAsensingGeneralPacketInterpreter
  )

set(asensingplugin_sources
  #${PandarGeneralRaw}/src/pandarGeneral_internal.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/AsensingPacketInterpreter/laser_ts.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/AsensingPacketInterpreter/cJSON.c
  )

#Create Module
vtk_module_add_module(AsensingLidar
  FORCE_STATIC # Using FORCE_STATIC build the vtk module statically into the plugin library, to avoid confusion when loading  
  SOURCES ${asensingplugin_sources} # Will not trigger a PV wrapping
  CLASSES ${asensingplugin_classes} # Those will be PV ClientServer wrapped #shorcut for sources x.cxx x.h  
  HEADERS ${asensingplugin_headers} # Those will be PV ClientServer wrapped
)
paraview_add_server_manager_xmls(
  MODULE AsensingLidar
  XMLS  ${asensingplugin_xml}
)

target_include_directories(AsensingLidar
  PUBLIC
  AsensingPacketInterpreter/
  #AsensingGeneralPacketInterpreter
  #${PandarGeneralRaw}/include/
  #${PandarGeneralRaw}/include/pandarGeneral
  )

set(calib_files
  Neptune48.json
  RT-Matrix.json
  No-Correction.json
)

# Copy the calibration to the build folder
foreach(file ${calib_files})
  configure_file("CalibrationFiles/${file}" "${PROJECT_BINARY_DIR}/share/${file}" COPYONLY)
endforeach()

# Install step
set(installfile_dest share)
if(APPLE)
  set(installfile_dest "${LV_INSTALL_LIBRARY_DIR}/../Resources")
endif()
foreach(file ${calib_files})
  install(FILES "${PROJECT_BINARY_DIR}/share/${file}" DESTINATION ${installfile_dest})
endforeach()
