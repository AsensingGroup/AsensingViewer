#-----------------------------------------------------------------------------
# Build Asensing Plugin
#-----------------------------------------------------------------------------

file(GLOB_RECURSE AsensingInterpreter_xml ${CMAKE_CURRENT_SOURCE_DIR}/*PacketInterpreter*.xml)
file(GLOB_RECURSE AsensingInterpreter_cxx ${CMAKE_CURRENT_SOURCE_DIR}/*PacketInterpreter*.cxx)
file(GLOB_RECURSE AsensingInterpreter_h ${CMAKE_CURRENT_SOURCE_DIR}/*PacketInterpreter*.h)

message(STATUS "XML: ${AsensingInterpreter_xml}")
message(STATUS "CXX: ${AsensingInterpreter_cxx}")
message(STATUS "HXX: ${AsensingInterpreter_h}")

set(asensingplugin_xml
  #${AsensingInterpreter_xml}
  ${CMAKE_CURRENT_SOURCE_DIR}/AsensingPacketInterpreter/AsensingPacketInterpreter.xml
  ${CMAKE_CURRENT_SOURCE_DIR}/AsensingPacketInterpreter/Asensing5PacketInterpreter.xml
  #${CMAKE_CURRENT_SOURCE_DIR}/AsensingGeneralPacketInterpreter/AsensingGeneralPacketInterpreter.xml
  )

set(asensingplugin_classes
  #${AsensingInterpreter_cxx}
  ${CMAKE_CURRENT_SOURCE_DIR}/AsensingPacketInterpreter/vtkAsensingPacketInterpreter
  ${CMAKE_CURRENT_SOURCE_DIR}/AsensingPacketInterpreter/vtkAsensing5PacketInterpreter
  #${CMAKE_CURRENT_SOURCE_DIR}/AsensingGeneralPacketInterpreter/vtkAsensingGeneralPacketInterpreter
  )

set(asensingplugin_sources
  #${AsensingInterpreter_cxx}
  ${CMAKE_CURRENT_SOURCE_DIR}/AsensingPacketInterpreter/cJSON.c
  )

set(asensingplugin_headers
  ${AsensingInterpreter_h}
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
  )

set(calib_files
  A0-Correction.json
  A0-Correction-5.json
  A2-Correction.json
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
