# Matplotlib expects these libraries to be named differently on Windows...
if (WIN32)
  configure_file(
    "${install_location}/lib/zlib.lib"
    "${install_location}/lib/z.lib"
    COPYONLY)
  configure_file(
    "${install_location}/lib/libpng16.lib"
    "${install_location}/lib/png.lib"
    COPYONLY)
endif ()

configure_file(
  "${patches_location}/matplotlib.setup.cfg.in"
  "${source_location}/setup.cfg")
