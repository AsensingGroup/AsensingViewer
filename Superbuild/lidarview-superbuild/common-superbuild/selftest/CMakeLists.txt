cmake_minimum_required(VERSION 3.19)

project(superbuild-selftest)

option(SUPERBUILD_ENABLE_OPENSSL "Enable support for OpenSSL" OFF)
set(_superbuild_enable_openssl "${SUPERBUILD_ENABLE_OPENSSL}")

function (superbuild_find_projects var)
  set(projects
    boost
    bzip2
    cxx11
    fortran
    freetype
    hdf5
    hdf5cpp
    lapack
    libjpegturbo
    matplotlib
    mpi
    netcdf
    ninja
    nlohmannjson
    numpy
    png
    python3
    pythonaiohttp
    pythonasynctimeout
    pythonattrs
    pythonautobahn
    pythonbeniget
    pythoncffi
    pythonchardet
    pythonconstantly
    pythoncppy
    pythoncryptography
    pythoncycler
    pythoncython
    pythondateutil
    pythongast
    pythonhyperlink
    pythonidna
    pythonincremental
    pythonkiwisolver
    pythonmako
    pythonmpi4py
    pythonmultidict
    pythonpillow
    pythonply
    pythonpycparser
    pythonpygments
    pythonpyparsing
    pythonpythran
    pythonsemanticversion
    pythonsetuptools
    pythonsetuptoolsscm
    pythonsetuptoolsrust
    pythonsix
    pythontoml
    pythontwisted
    pythontxaio
    pythontypingextensions
    pythonwheel
    pythonwslink
    pythonwslinkasync
    pythonpywebvue
    pythonyarl
    pythonzope
    pythonzopeinterface
    pytz
    qhull
    qt5
    szip
    tbb
    zlib)

  if (SUPERBUILD_ENABLE_OPENSSL)
    list(APPEND projects
      openssl)
  endif ()

  if (WIN32)
    list(APPEND projects
      pywin32)
  endif ()

  if (NOT WIN32)
    list(APPEND projects
      ffi
      ffmpeg
      gperf
      libxml2
      pkgconf
      pybind11
      scipy
      sqlite)
  endif ()

  if (UNIX AND NOT APPLE)
    list(APPEND projects
      expat
      fontconfig
      glproto
      llvm
      mesa
      meson
      osmesa
      utillinux)
  endif ()

  set("${var}"
    ${projects}
    PARENT_SCOPE)
endfunction ()

# Disable Qt modules that generally aren't necessary.
set(qt5_skip_modules
  qtconnectivity
  qtgamepad
  qtlocation
  qtmultimedia
  qtsensors
  qtserialport
  qtwayland
  qtwebchannel
  qtwebengine
  qtwebsockets)

get_filename_component(sb_dir "${CMAKE_CURRENT_LIST_DIR}" DIRECTORY)
add_subdirectory("${sb_dir}" "${CMAKE_CURRENT_BINARY_DIR}/superbuild")

include(CTest)
enable_testing()
add_subdirectory(tests)
