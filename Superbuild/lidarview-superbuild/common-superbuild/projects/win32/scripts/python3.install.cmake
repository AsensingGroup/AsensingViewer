file(GLOB files "*")
file(
  INSTALL ${files}
  DESTINATION "${install_location}")

if (NOT keep_openssl)
  file(REMOVE
    "${install_location}/DLLs/libcrypto-1_1.dll"
    "${install_location}/DLLs/libssl-1_1.dll"
    "${install_location}/DLLs/_ssl.pyd")
endif ()
