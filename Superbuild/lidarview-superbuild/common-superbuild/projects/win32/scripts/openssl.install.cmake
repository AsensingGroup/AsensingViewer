file(GLOB files "x64/")
file(INSTALL ${files}
     DESTINATION "${install_location}")
