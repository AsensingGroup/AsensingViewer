cmake_dependent_option(PACKAGE_SYSTEM_QT "Packages needed Qt files" ON
  "${qt_package_dep_variable}" OFF)
if (NOT PACKAGE_SYSTEM_QT)
  return ()
endif ()

# For Windows, add rules to pacakge system Qt.
function (_qt_query_qmake qt_var result)
  execute_process(
    COMMAND         "${QT_QMAKE_EXECUTABLE}"
                    -query "${qt_var}"
    RESULT_VARIABLE return_code
    OUTPUT_VARIABLE output
    ERROR_VARIABLE  output
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_STRIP_TRAILING_WHITESPACE)

  if (NOT return_code)
    file(TO_CMAKE_PATH "${output}" output)
    set("${result}"
      "${output}"
      PARENT_SCOPE)
  endif ()
endfunction ()

# locate the bin dir and the plugins dir.
if (EXISTS "${QT_QMAKE_EXECUTABLE}")
  _qt_query_qmake(QT_INSTALL_PLUGINS qt_plugins_dir)
  _qt_query_qmake(QT_INSTALL_BINS qt_bin_dir)

  install(
    DIRECTORY "${qt_plugins_dir}/"
    DESTINATION "bin"
    USE_SOURCE_PERMISSIONS
    COMPONENT qt5-runtime

    # skip debug dlls
    FILES_MATCHING REGEX "^.*d4.dll$" EXCLUDE
    PATTERN "*.dll")

  install(
    DIRECTORY "${qt_bin_dir}/"
    DESTINATION "bin"
    USE_SOURCE_PERMISSIONS
    COMPONENT qt5-runtime

    # skip debug dlls
    FILES_MATCHING REGEX "^.*d4.dll$" EXCLUDE
    PATTERN "*.dll")
endif()
