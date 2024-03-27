include("${CMAKE_CURRENT_LIST_DIR}/../qt5.system.cmake")

get_filename_component(qt5_dllpath "${Qt5_DIR}/../../../bin" REALPATH)

set(qt_package_dep_variable "USE_SYSTEM_qt5")
include("${CMAKE_CURRENT_LIST_DIR}/qt5.package.cmake")
