# On windows, the dll must be next to the executable to run
file(GLOB opencv_dll ${install_location}/*/*/bin/opencv*.dll)
foreach (dll IN LISTS opencv_dll)
  file(
  COPY        "${dll}"
  DESTINATION "${install_location}/bin/"
  )
endforeach()
