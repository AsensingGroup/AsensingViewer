include(openssl.common)

set(openssl_zlib_options
  zlib)
if (zlib_built_by_superbuild)
  list(APPEND openssl_zlib_options
    --with-zlib-include=<INSTALL_DIR>/include
    --with-zlib-lib=<INSTALL_DIR>/lib)
endif ()

superbuild_add_project(openssl
  CAN_USE_SYSTEM
  DEPENDS zlib
  CONFIGURE_COMMAND
    "<SOURCE_DIR>/config"
      "--prefix=<INSTALL_DIR>"
      "--libdir=lib"
      ${openssl_zlib_options}
      no-autoload-config # our packages are relocatable
      no-tests

      # Enable a bunch of stuff that is needed by tools that the superbuild may
      # also run. Many of these tools will end up loading our OpenSSL due to
      # LD_LIBRARY_PATH settings.
      enable-camellia enable-seed enable-rfc3779
      enable-cms enable-md2 enable-rc5 enable-ssl3 enable-ssl3-method
      enable-weak-ssl-ciphers
  BUILD_COMMAND
    $(MAKE)
  INSTALL_COMMAND
    $(MAKE) install)

# Required to be compatible with OpenSSL-using tools shipped by Fedora. Small
# changes made to avoid FIPS stuff we're not patching in.
superbuild_apply_patch(openssl evp-kdf
  "Add EVP_KDF_ functions")
