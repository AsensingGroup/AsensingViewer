function (superbuild_python_write_reqs path)
  get_property(packages GLOBAL
    PROPERTY _superbuild_python_packages)

  file(WRITE "${path}" "")
  foreach (package IN LISTS packages)
    file(APPEND "${path}"
      "${package}\n")
  endforeach ()
endfunction ()

function (superbuild_install_superbuild_python3)
  if (USE_SYSTEM_python3)
    return ()
  endif()

  set(options)
  set(values
    DESTINATION_SUBDIR
    BUNDLE
    LIBSUFFIX)
  set(multivalues
    MODULES)
  cmake_parse_arguments(_install_superbuild_python "${options}" "${values}" "${multivalues}" ${ARGN})

  set(modules
    __future__
    __pycache__
    _bootlocale
    _collections_abc
    _compat_pickle
    _compression
    _dummy_thread
    _lzma
    _markupbase
    _osx_support
    _py_abc
    _pydecimal
    _pyio
    _sitebuiltins
    _strptime
    _threading_local
    _weakrefset
    abc
    aifc
    antigravity
    argparse
    ast
    asynchat
    asyncio
    asyncore
    base64
    bdb
    binhex
    bisect
    bz2
    cProfile
    calendar
    cgi
    cgitb
    chunk
    cmd
    code
    codecs
    codeop
    collections
    colorsys
    compileall
    concurrent
    config-${superbuild_python_version}m-x86_64-linux-gnu
    configparser
    contextlib
    contextvars
    copy
    copyreg
    crypt
    csv
    ctypes
    curses
    dataclasses
    datetime
    dbm
    decimal
    difflib
    dis
    distutils
    doctest
    dummy_threading
    email
    encodings
    ensurepip
    enum
    filecmp
    fileinput
    fnmatch
    formatter
    fractions
    ftplib
    functools
    genericpath
    getopt
    getpass
    gettext
    glob
    gzip
    hashlib
    heapq
    hmac
    html
    http
    idlelib
    imaplib
    imghdr
    imp
    importlib
    inspect
    io
    ipaddress
    json
    keyword
    lib2to3
    linecache
    locale
    logging
    lzma
    macpath
    mailbox
    mailcap
    mimetypes
    modulefinder
    multiprocessing
    netrc
    nntplib
    ntpath
    nturl2path
    numbers
    opcode
    operator
    optparse
    os
    pathlib
    pdb
    pickle
    pickletools
    pipes
    pkgutil
    platform
    plistlib
    poplib
    posixpath
    pprint
    profile
    pstats
    pty
    py_compile
    pyclbr
    pydoc
    pydoc_data
    queue
    quopri
    random
    re
    reprlib
    rlcompleter
    runpy
    sched
    secrets
    selectors
    shelve
    shlex
    shutil
    signal
    site
    smtpd
    smtplib
    sndhdr
    socket
    socketserver
    sqlite3
    sre_compile
    sre_constants
    sre_parse
    ssl
    stat
    statistics
    string
    stringprep
    struct
    subprocess
    sunau
    symbol
    symtable
    sysconfig
    tabnanny
    tarfile
    telnetlib
    tempfile
    test
    textwrap
    this
    threading
    timeit
    tkinter
    token
    tokenize
    trace
    traceback
    tracemalloc
    tty
    turtle
    turtledemo
    types
    typing
    unittest
    urllib
    uu
    uuid
    venv
    warnings
    wave
    weakref
    webbrowser
    wsgiref
    xdrlib
    xml
    xmlrpc
    zipapp
    zipfile
    )

  if (openssl_enabled)
    list(APPEND modules
      _ssl)
  endif ()

  if (WIN32)
    superbuild_windows_install_python(
      MODULE_DESTINATION  "/"
      MODULES             ${modules} ${_install_superbuild_python_MODULES}
      MODULE_DIRECTORIES  "${superbuild_install_location}/Python/Lib"
      SEARCH_DIRECTORIES  "${superbuild_install_location}/Python"
                          "${superbuild_install_location}/Python/libs${_install_superbuild_python_LIBSUFFIX}"
      EXCLUDE_REGEXES     "vcruntime[0-9]+.dll")

    # install everyting under `DLLs`
    install(
      DIRECTORY "${superbuild_install_location}/Python/DLLs"
      DESTINATION "bin"
      COMPONENT "superbuild")

  elseif (APPLE)
    if (_install_superbuild_python_DESTINATION_SUBDIR)
      set(_install_superbuild_python_suffix
        "/${_install_superbuild_python_DESTINATION_SUBDIR}")
    endif ()

    superbuild_apple_install_python(
      "\${CMAKE_INSTALL_PREFIX}${_install_superbuild_python_suffix}"
      "${_install_superbuild_python_BUNDLE}"
      MODULES             ${modules} ${_install_superbuild_python_MODULES}
      MODULE_DIRECTORIES  "${superbuild_install_location}/lib/python${superbuild_python_version}"
                          "${superbuild_install_location}/lib/python${superbuild_python_version}/lib-dynload"
      PYTHON_DESTINATION  "Contents/Libraries/lib/python${superbuild_python_version}")

    # fixup and install everything under lib-dynload.
    # fixup is needed to ensure that any dependencies for these libraries are
    # installed. this also ensures rpaths etc are fixed up correctly.
    file(GLOB so_names
      "${superbuild_install_location}/lib/python${superbuild_python_version}/lib-dynload/*.so")
    foreach (so_name IN LISTS so_names)
      superbuild_apple_install_module(
        "\${CMAKE_INSTALL_PREFIX}${_install_superbuild_python_suffix}"
        "${_install_superbuild_python_BUNDLE}"
        "${so_name}"
        "Contents/Libraries/lib/python${superbuild_python_version}"
        LOADER_PATHS "${superbuild_install_location}/lib")
    endforeach ()
  else ()
    superbuild_unix_install_python(
      MODULE_DESTINATION  "/"
      LIBDIR              "lib${_install_superbuild_python_LIBSUFFIX}"
      MODULES             ${modules} ${_install_superbuild_python_MODULES}
      MODULE_DIRECTORIES  "${superbuild_install_location}/lib/python${superbuild_python_version}"
                          "${superbuild_install_location}/lib/python${superbuild_python_version}/lib-dynload")

    # fixup and install everything under lib-dynload.
    # fixup is needed to ensure that any dependencies for these libraries are
    # installed. this also ensures rpaths etc are fixed up correctly.
    file(GLOB so_names
      RELATIVE "${superbuild_install_location}/lib/python${superbuild_python_version}/lib-dynload"
      "${superbuild_install_location}/lib/python${superbuild_python_version}/lib-dynload/*.so")
    foreach (so_name IN LISTS so_names)
      superbuild_unix_install_module("${so_name}"
        "lib"
        "lib/python${superbuild_python_version}/lib-dynload"
        LOADER_PATHS "${superbuild_install_location}/lib")
    endforeach ()
  endif ()
endfunction ()
