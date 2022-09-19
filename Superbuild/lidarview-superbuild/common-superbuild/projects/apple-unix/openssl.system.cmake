find_package(OpenSSL REQUIRED)

superbuild_add_extra_cmake_args(
  "-DOPENSSL_INCLUDE_DIR:PATH=${OPENSSL_INCLUDE_DIR}"
  "-DOPENSSL_CRYPTO_LIBRARY:FILEPATH=${OPENSSL_CRYPTO_LIBRARY}"
  "-DOPENSSL_SSL_LIBRARY:FILEPATH=${OPENSSL_SSL_LIBRARY}")
