if (NOT _superbuild_enable_openssl)
  message(FATAL_ERROR
    "Due to complications around shipping OpenSSL in binaries, the superbuild "
    "requires explicit permission to deal with OpenSSL. Please see "
    "documentation for use of OpenSSL.")
endif ()
