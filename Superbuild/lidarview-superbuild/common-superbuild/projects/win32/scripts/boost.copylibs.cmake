file(
  COPY        "${install_location}/lib/"
  DESTINATION "${install_location}/bin"
  FILES_MATCHING
    PATTERN "boost*.dll")
