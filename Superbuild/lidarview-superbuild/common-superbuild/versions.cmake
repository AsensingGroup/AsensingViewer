# This maintains the links for all sources used by this superbuild.
# Simply update this file to change the revision.
# One can use different revision on different platforms.
# e.g.
# if (UNIX)
#   ..
# else (APPLE)
#   ..
# endif()

include(CMakeDependentOption)

# NOTE: if updating bzip2 version, fix patch in bzip2.cmake
superbuild_set_revision(bzip2
  URL     "https://www.paraview.org/files/dependencies/bzip2-1.0.8.tar.gz"
  URL_MD5 67e051268d0c475ea773822f7500d0e5)

superbuild_set_revision(zlib
  URL     "https://www.paraview.org/files/dependencies/zlib-1.2.11.tar.xz"
  URL_MD5 85adef240c5f370b308da8c938951a68)

superbuild_set_revision(ffmpeg
  URL     "https://www.paraview.org/files/dependencies/ffmpeg-4.4.tar.xz"
  URL_MD5 7b9d5b652d20e8c5405304ad72636d4a)

superbuild_set_revision(szip
  URL     "https://www.paraview.org/files/dependencies/szip-2.1.1.tar.gz"
  URL_MD5 dd579cf0f26d44afd10a0ad7291fc282)

superbuild_set_revision(hdf5
  URL     "https://www.paraview.org/files/dependencies/hdf5-1.12.1.tar.bz2"
  URL_MD5  442469fbf43626006346e679c22cf10a)

superbuild_set_revision(boost
  URL     "https://www.paraview.org/files/dependencies/boost_1_76_0.tar.bz2"
  URL_MD5 33334dd7f862e8ac9fe1cc7c6584fb6d)

superbuild_set_revision(png
  URL     "https://www.paraview.org/files/dependencies/libpng-1.6.37.tar.xz"
  URL_MD5 015e8e15db1eecde5f2eb9eb5b6e59e9)

if (WIN32)
  superbuild_set_revision(python3
    URL     "https://www.paraview.org/files/dependencies/python-3.9.5-windows-x86_64.zip"
    URL_MD5 c41556099961c9e0d4d6afd419045bac)
else()
  superbuild_set_revision(python3
    URL     "https://www.paraview.org/files/dependencies/Python-3.9.5.tar.xz"
    URL_MD5 71f7ada6bec9cdbf4538adc326120cfd)
endif()

superbuild_set_revision(freetype
  URL     "https://www.paraview.org/files/dependencies/freetype-2.12.0.tar.xz"
  URL_MD5 9a07649ce73ba0d80d211092b3d1c2fa)

superbuild_set_revision(gperf
  URL     "https://www.paraview.org/files/dependencies/gperf-3.1.tar.gz"
  URL_MD5 9e251c0a618ad0824b51117d5d9db87e)

superbuild_set_revision(fontconfig
  URL     "https://www.paraview.org/files/dependencies/fontconfig-2.13.1.tar.bz2"
  URL_MD5 36cdea1058ef13cbbfdabe6cb019dc1c)

superbuild_set_revision(libxml2
  URL     "https://www.paraview.org/files/dependencies/libxml2-2.9.12.tar.gz"
  URL_MD5 f433a39be087a9f0b197eb2307ad9f75)

superbuild_set_revision(nlohmannjson
  URL     "https://www.paraview.org/files/dependencies/nlohmannjson-v3.9.1.tar.gz"
  URL_MD5 e386222fb57dd2fcb8a7879fc016d037)

superbuild_set_selectable_source(qt5
  SELECT 5.8
    URL     "https://www.paraview.org/files/dependencies/qt-everywhere-opensource-src-5.8.0.tar.xz"
    URL_MD5 "66660cd3d9e1a6fed36e88adcb72e9fe"
  SELECT 5.9
    URL     "https://www.paraview.org/files/dependencies/qt-everywhere-opensource-src-5.9.2.tar.xz"
    URL_MD5 "738d1b98106e1bd39f00cc228beb522a"
  SELECT 5.10
    URL     "https://www.paraview.org/files/dependencies/qt-everywhere-src-5.10.1.tar.xz"
    URL_MD5 "7e167b9617e7bd64012daaacb85477af"
  SELECT 5.12
    URL     "https://www.paraview.org/files/dependencies/qt-everywhere-src-5.12.9.tar.xz"
    URL_MD5 "fa2646280cf38180689c29c393cddd05"
  SELECT 5.15 DEFAULT
    URL     "https://www.paraview.org/files/dependencies/qt-everywhere-src-5.15.2.tar.xz"
    URL_MD5 "e1447db4f06c841d8947f0a6ce83a7b5")

superbuild_set_revision(numpy
  URL     "https://www.paraview.org/files/dependencies/numpy-1.21.1.zip"
  URL_MD5 1d016e05851a4ba85307f3246eb569aa)
superbuild_set_revision(scipy
  URL     "https://www.paraview.org/files/dependencies/scipy-1.7.1.tar.gz"
  URL_MD5 8ac74369cdcabc097f602682c951197c)

superbuild_set_revision(qhull
  URL     "https://www.paraview.org/files/dependencies/qhull-2020-src-8.0.2.tgz"
  URL_MD5 295f7332269a38279478f555cc185296)

superbuild_set_revision(libjpegturbo
  URL     "https://www.paraview.org/files/dependencies/libjpeg-turbo-2.1.1.tar.gz"
  URL_MD5 cf16866976ab31cd6fc478eac8c2c54e)

superbuild_set_revision(pythonpillow
  URL     "https://www.paraview.org/files/dependencies/Pillow-8.3.1.tar.gz"
  URL_MD5 e42fc66e41b5309436a573af49cec47c)

superbuild_set_revision(matplotlib
  URL "https://www.paraview.org/files/dependencies/matplotlib-3.4.2.tar.gz"
  URL_MD5 e34749a5f0661b8af74a1dc327fb74f6)

superbuild_set_revision(pywin32
  URL "https://www.paraview.org/files/dependencies/pywin32-301-cp39-cp39-win_amd64.whl"
  URL_MD5 3fe9793d6bee6e9b6515bc744f7585df)

superbuild_set_revision(mpi
  URL     "https://www.paraview.org/files/dependencies/mpich-3.4.2.tar.gz"
  URL_MD5 6ee1cfff98728e5160c6e78bdb1986ca)

superbuild_set_revision(lapack
  URL     "https://www.paraview.org/files/dependencies/lapack-3.10.0.tar.gz"
  URL_MD5 d70fc27a8bdebe00481c97c728184f09)

superbuild_set_revision(netcdf
  URL     "https://www.paraview.org/files/dependencies/netcdf-c-4.8.1.tar.gz"
  URL_MD5 b069f4eb1718798c2907c38189615f95)

superbuild_set_revision(tbb
  URL     "https://www.paraview.org/files/dependencies/tbb-2021.5.0.tar.gz"
  URL_MD5 5e5f2ee22a0d19c0abbe7478f1c7ccf6)

superbuild_set_revision(pytz
  URL     "https://www.paraview.org/files/dependencies/pytz-2021.1.tar.gz"
  URL_MD5 8c849bdf95414fe708a84473e42d4406)

superbuild_set_revision(pythondateutil
  URL     "https://www.paraview.org/files/dependencies/python-dateutil-2.8.2.tar.gz"
  URL_MD5 5970010bb72452344df3d76a10281b65)

superbuild_set_revision(pythonpyparsing
  URL     "https://www.paraview.org/files/dependencies/pyparsing-2.4.7.tar.gz"
  URL_MD5 f0953e47a0112f7a65aec2305ffdf7b4)

superbuild_set_revision(pythonbeniget
  URL     "https://www.paraview.org/files/dependencies/beniget-0.4.1.tar.gz"
  URL_MD5 a2bbe7f17f10f9c127d8ef00692ddc55)

superbuild_set_revision(pythongast
  URL     "https://www.paraview.org/files/dependencies/gast-0.5.2.tar.gz"
  URL_MD5 eb2489df0c85ae198e4740e5711c7299)

superbuild_set_revision(pythonply
  URL     "https://www.paraview.org/files/dependencies/ply-3.11.tar.gz"
  URL_MD5 6465f602e656455affcd7c5734c638f8)

superbuild_set_revision(pythonpythran
  URL     "https://www.paraview.org/files/dependencies/pythran-0.9.12.post1.tar.gz"
  URL_MD5 b84d70ed33554dcef423673216bc3826)

superbuild_set_revision(pythoncycler
  URL     "https://www.paraview.org/files/dependencies/cycler-0.10.0.tar.gz"
  URL_MD5 4cb42917ac5007d1cdff6cccfe2d016b)

superbuild_set_revision(pythoncython
  URL     "https://www.paraview.org/files/dependencies/Cython-0.29.24.tar.gz"
  URL_MD5 81aff945f5bfdfb86e7a5d24f5467668)

superbuild_set_revision(pythonsetuptools
  URL     "https://www.paraview.org/files/dependencies/setuptools-57.4.0.tar.gz"
  URL_MD5 1023c3535d6a3724b3c9a9929dcef195)

superbuild_set_revision(pythonwheel
  # PyPI source tarball with 'unicode.dist' test excised from it (CMake has
  # issues extracting non-UTF-8 names in tarballs).
  URL     "https://www.paraview.org/files/dependencies/wheel-0.37.0-nounicodedist.tar.gz"
  URL_MD5 c4223d1502f05a23a67d6f0db0b424e9)

superbuild_set_revision(pythonpycparser
  URL     "https://www.paraview.org/files/dependencies/pycparser-2.20.tar.gz"
  URL_MD5 b8f88de737db8c346ee8d31c07c7a25a)

superbuild_set_revision(pythontoml
  URL     "https://www.paraview.org/files/dependencies/toml-0.10.2.tar.gz"
  URL_MD5 59bce5d8d67e858735ec3f399ec90253)

superbuild_set_revision(pythonsetuptoolsscm
  URL     "https://www.paraview.org/files/dependencies/setuptools_scm-6.0.1.tar.gz"
  URL_MD5 aa7f0efbbf46c5576db5994dd1ce3f8d)

superbuild_set_revision(pythonsetuptoolsrust
  URL     "https://www.paraview.org/files/dependencies/setuptools-rust-0.12.1.tar.gz"
  URL_MD5 33c3fd3bcde2877483ab782353bee54c)

superbuild_set_revision(pythonaiohttp
  URL     "https://www.paraview.org/files/dependencies/aiohttp-3.7.4.post0.tar.gz"
  URL_MD5 7052a8e9877921d73da98d2b18c9a145)

superbuild_set_revision(pythonasynctimeout
  URL     "https://www.paraview.org/files/dependencies/async-timeout-3.0.1.tar.gz"
  URL_MD5 305c4fa529f2485c403d0dbe14390175)

superbuild_set_revision(pythonchardet
  URL     "https://www.paraview.org/files/dependencies/chardet-4.0.0.tar.gz"
  URL_MD5 bc9a5603d8d0994b2d4cbf255f99e654)

superbuild_set_revision(pythonmultidict
  URL     "https://www.paraview.org/files/dependencies/multidict-5.1.0.tar.gz"
  URL_MD5 df8b37f069809779214d6b80b250e45b)

superbuild_set_revision(pythontypingextensions
  URL     "https://www.paraview.org/files/dependencies/typing_extensions-3.10.0.0.tar.gz"
  URL_MD5 9b5b33ae64c94479fa0862cf8ae89d58)

superbuild_set_revision(pythonyarl
  URL     "https://www.paraview.org/files/dependencies/yarl-1.6.3.tar.gz "
  URL_MD5 3b6f2da3db8c645a9440375fd6a414eb)

superbuild_set_revision(pythonautobahn
  URL     "https://www.paraview.org/files/dependencies/autobahn-21.3.1.tar.gz"
  URL_MD5 dcba839ee61be33d05042a09c008c6bc)

superbuild_set_revision(pythoncffi
  URL     "https://www.paraview.org/files/dependencies/cffi-1.14.6.tar.gz"
  URL_MD5 5c118a18ea897df164dbff67a32876fc)

superbuild_set_revision(pythonsemanticversion
  URL     "https://www.paraview.org/files/dependencies/semantic_version-2.8.5.tar.gz"
  URL_MD5 76d7364def7ee487b6153d40b13de904)

superbuild_set_revision(pythonconstantly
  URL     "https://www.paraview.org/files/dependencies/constantly-15.1.0.tar.gz"
  URL_MD5 f0762f083d83039758e53f8cf0086eef)

superbuild_set_revision(pythonhyperlink
  URL     "https://www.paraview.org/files/dependencies/hyperlink-21.0.0.tar.gz"
  URL_MD5 6285ac13e7d6be4157698ad7960ed490)

superbuild_set_revision(pythonidna
  URL     "https://www.paraview.org/files/dependencies/idna-3.2.tar.gz"
  URL_MD5 08ea8e2ce09e522424e872409c221138)

superbuild_set_revision(pythonincremental
  URL     "https://www.paraview.org/files/dependencies/incremental-21.3.0.tar.gz"
  URL_MD5 9f7ad12e0c05a12cee52a7350976c4e3)

superbuild_set_revision(pythontwisted
  URL     "https://www.paraview.org/files/dependencies/Twisted-21.7.0.tar.gz"
  URL_MD5 6b17e9a4b28b8846847e5324c2e59bf4)

superbuild_set_revision(pythontxaio
  URL     "https://www.paraview.org/files/dependencies/txaio-21.2.1.tar.gz"
  URL_MD5 7e80b80ed7797245a5eef803043bdede)

superbuild_set_revision(pythonwslinkasync
  URL     "https://www.paraview.org/files/dependencies/wslink-1.5.3.tar.gz"
  URL_MD5 ac0febcb76a293d981434940e86a3438)

superbuild_set_revision(pythonwslink
  URL     "https://www.paraview.org/files/dependencies/wslink-0.1.11.tar.gz"
  URL_MD5 35e6285c2a74304da0557f1402c400e5)

superbuild_set_revision(pythonpywebvue
  URL     "https://www.paraview.org/files/dependencies/pywebvue-2.1.4.tar.gz"
  URL_MD5 02f63a8b2ca20cb1d7328b1f7369a88f)

superbuild_set_revision(pythonzope
  URL     "https://www.paraview.org/files/dependencies/Zope-5.3.tar.gz"
  URL_MD5 3b8ddc554345279c0e0018d5f1814c13)

superbuild_set_revision(pythonzopeinterface
  URL     "https://www.paraview.org/files/dependencies/zope.interface-5.4.0.tar.gz"
  URL_MD5 c58b31da83449631efb499de13c68c6a)

superbuild_set_revision(pythonsix
  URL     "https://www.paraview.org/files/dependencies/six-1.16.0.tar.gz"
  URL_MD5 a7c927740e4964dd29b72cebfc1429bb)

superbuild_set_revision(pythonpygments
  URL     "https://www.paraview.org/files/dependencies/Pygments-2.9.0.tar.gz"
  URL_MD5 665516d1d1c0099241ab6e4c057e26be)

superbuild_set_revision(pythonmako
  URL     "https://www.paraview.org/files/dependencies/Mako-1.1.4.tar.gz"
  URL_MD5 2cd02c14d08c2180b3e10d3c2e749b9e)

superbuild_set_revision(pythoncppy
  URL     "https://www.paraview.org/files/dependencies/cppy-1.1.0.tar.gz"
  URL_MD5 2110891d75aa12551deebba1603428c6)

superbuild_set_revision(pythonkiwisolver
  URL     "https://www.paraview.org/files/dependencies/kiwisolver-1.3.1.tar.gz"
  URL_MD5 81012578317ddcfa3daed806142f8fed)

superbuild_set_revision(pythonattrs
  URL     "https://www.paraview.org/files/dependencies/attrs-21.2.0.tar.gz"
  URL_MD5 06af884070d9180694becdb106e5cd65)

superbuild_set_revision(pythonpandas
  URL     "https://www.paraview.org/files/dependencies/pandas-1.3.1.tar.gz"
  URL_MD5 407560bb24b0ec4785ecf4dba5e1a139)

superbuild_set_revision(ffi
  URL     "https://www.paraview.org/files/dependencies/libffi-3.4.2.tar.gz"
  URL_MD5 294b921e6cf9ab0fbaea4b639f8fdbe8)

superbuild_set_revision(utillinux
  URL     "https://www.paraview.org/files/dependencies/util-linux-2.37.1.tar.xz"
  URL_MD5 6d244f0f59247e9109f47d6e5dd0556b)

superbuild_set_revision(pkgconf
  URL     "https://www.paraview.org/files/dependencies/pkgconf-1.8.0.tar.xz"
  URL_MD5 823212dc241793df8ff1d097769a3473)

superbuild_set_revision(pybind11
  URL     "https://www.paraview.org/files/dependencies/pybind11-2.9.1.tar.gz"
  URL_MD5 7609dcb4e6e18eee9dc1a5f26572ded1)

superbuild_set_revision(sqlite
  URL     "https://www.paraview.org/files/dependencies/sqlite-autoconf-3360000.tar.gz"
  URL_MD5 f5752052fc5b8e1b539af86a3671eac7)

superbuild_set_revision(expat
  URL     "https://www.paraview.org/files/dependencies/expat-2.4.1.tar.xz"
  URL_MD5 a4fb91a9441bcaec576d4c4a56fa3aa6)

superbuild_set_revision(llvm
  URL     "https://www.paraview.org/files/dependencies/llvm-7.0.0.src.tar.xz"
  URL_MD5 e0140354db83cdeb8668531b431398f0)

superbuild_set_revision(glproto
  URL     "https://www.paraview.org/files/dependencies/glproto-1.4.17.tar.bz2"
  URL_MD5 5565f1b0facf4a59c2778229c1f70d10)

superbuild_set_revision(meson
  URL     "https://www.paraview.org/files/dependencies/meson-0.62.0.tar.gz"
  URL_MD5 2b8c86273f9f94aada9adcce895861d8)

superbuild_set_revision(mesa
  URL     "https://www.paraview.org/files/dependencies/mesa-21.2.1.tar.xz"
  URL_MD5 5d8beb41eccad604296d1e2a6688dd6a)
get_property(mesa_revision GLOBAL PROPERTY mesa_revision)
superbuild_set_revision(osmesa ${mesa_revision})

superbuild_set_revision(ninja
  URL     "https://www.paraview.org/files/dependencies/ninja-1.10.2.tar.gz"
  URL_MD5 639f75bc2e3b19ab893eaf2c810d4eb4)

set(pythoncryptography_version "3.4.7")
if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
  if (CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "64")
    set(pythoncryptography_file
      "cryptography-${pythoncryptography_version}-cp36-abi3-win_amd64.whl")
    set(pythoncryptography_md5
      "4ac946949ecb278b028c2fcf5d1cbc2b")
  endif ()
elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  if (CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "x86_64")
    set(pythoncryptography_file
      "cryptography-${pythoncryptography_version}-cp36-abi3-macosx_10_10_x86_64.whl")
    set(pythoncryptography_md5
      "f5e574ea0e46b25157a29d09fc6e76b3")
  elseif (CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "arm64")
    set(pythoncryptography_file
      "cryptography-${pythoncryptography_version}-cp36-abi3-macosx_11_0_arm64.whl")
    set(pythoncryptography_md5
      "cd55873ce4a9aa985a0573efdc4244e1")
  endif ()
elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux")
  if (CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "x86_64")
    set(pythoncryptography_file
      "cryptography-${pythoncryptography_version}-cp36-abi3-manylinux2010_x86_64.whl")
    set(pythoncryptography_md5
      "37e6016ff0dd06e168d908ee800a071b")
  endif ()
endif ()
if (NOT pythoncryptography_file)
  message(WARNING
    "The Python cryptography package is being built from source due to the "
    "lack of a suitable wheel file. This needs a Rust compiler. Please see "
    "https://rustup.rs/ for instructions on obtaining a toolchain.")
  set(pythoncryptography_file
    "cryptography-${pythoncryptography_version}.tar.gz")
  set(pythoncryptography_md5
    "f24fb11c6d5beb18cbfe216b9e58c27e")
  set_property(GLOBAL
    PROPERTY
      pythoncryptography_source 1)
endif ()
superbuild_set_revision(pythoncryptography
  URL     "https://www.paraview.org/files/dependencies/${pythoncryptography_file}"
  URL_MD5 "${pythoncryptography_md5}")

set(openssl_version 1.1.1k)
if (WIN32)
  # Obtained from https://kb.firedaemon.com/support/solutions/articles/4000121705
  superbuild_set_revision(openssl
    URL     "https://www.paraview.org/files/dependencies/openssl-${openssl_version}.zip"
    URL_MD5 0c08d59e229e2ac3cb941158b4d35915)
else ()
  superbuild_set_revision(openssl
    URL     "https://www.paraview.org/files/dependencies/openssl-${openssl_version}.tar.gz"
    URL_MD5 c4e7d95f782b08116afa27b30393dd27)
endif ()
