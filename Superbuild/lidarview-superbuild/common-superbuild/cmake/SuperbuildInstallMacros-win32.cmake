set_property(GLOBAL PROPERTY
  superbuild_has_cleaned FALSE)

# TODO: error on unrecognized arguments

#[==[.md
## PE-COFF (Windows)

The superbuild installs PE-COFF binaries using a core function to construct a
command to run the `fixup_bundle.windows.py` script with the correct arguments.
It tries to emulate the runtime loader to determine where to find dependent
files and it copies them to the installation directory.

Calling this function directory should not be necessary. Instead, using the
more specific functions documented later is recommended. The core function is
used as a single place to document the various common arguments available to
the other functions. If an argument is specified by a function, it should not
be passed as the remaining arguments to the function.

```
_superbuild_windows_install_binary(
  LIBDIR <libdir>
  BINARY <path>
  TYPE <module|executable>
  [CLEAN]
  [DESTINATION <destination>]
  [LOCATION <location>]
  [INCLUDE_REGEXES <include-regex>...]
  [EXCLUDE_REGEXES <exclude-regex>...]
  [IGNORE_DLLNAMES <ignore-name>...]
  [BINARY_LIBDIR <binary_libdirs>...]
  [SEARCH_DIRECTORIES <search-directory>...])
```

A manifest file is kept in the binary directory of the packaging step. This
allows for a library to be installed just once for an entire package. It is
reset when the `CLEAN` argument is present. In addition, the install directory
is removed for non-install targets (based on `superbuild_is_install_target`)
when `CLEAN` is specified. Whether this is necessary or not is maintained
internally and it should almost never need to be provided.

The `DESTINATION` is the absolute path to the installation prefix, including
`DESTDIR`. It defaults to `\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}`. It is
escaped because we want CMake to expand its value at install time, not
configure time.

The `BINARY` argument is the path to the actual executable to install. It must
be an absolute path.

The `TYPE` argument specifies whether an executable or module (e.g., plugin or
standalone library) is being installed. For a module, the `LOCATION` argument
must be given. This is the path under the installation prefix to place the
module. Executables are always installed into `bin`. The libraries that are
found to be required by the installed binary are placed in the subdirectory of
the install destination given by the `LIBDIR` argument.

The `BINARY_LIBDIR` argument is a list of paths which the binary is assumed to
already be searching when loading a module.

The `SEARCH_DIRECTORIES` argument is a list of paths to search for libraries if
the library cannot be found due to rpaths in the binary, `LOADER_PATHS`, or
other runtime-loader logic. If these paths are required to find a library, a
warning is printed at install time.

By default, Microsoft's C runtime libraries are ignored when installing. The
`INCLUDE_REGEXES` and `EXCLUDE_REGEXES` arguments are lists of Python regular
expressions to either force-include or force-exclude from installation.
Inclusion overrides exclusion. The provided regular expressions are also
expected to match the full path of the library.

DLL names listed in `IGNORE_DLLNAMES` will be ignored if found in a dependency
list.
#]==]
function (_superbuild_windows_install_binary)
  set(options
    CLEAN)
  set(values
    DESTINATION
    LIBDIR
    LOCATION
    BINARY
    TYPE)
  set(multivalues
    INCLUDE_REGEXES
    EXCLUDE_REGEXES
    IGNORE_DLLNAMES
    BINARY_LIBDIR
    SEARCH_DIRECTORIES)
  cmake_parse_arguments(_install_binary "${options}" "${values}" "${multivalues}" ${ARGN})

  if (NOT _install_binary_BINARY)
    message(FATAL_ERROR "Cannot install a binary without a path.")
  endif ()

  if (NOT IS_ABSOLUTE "${_install_binary_BINARY}")
    message(FATAL_ERROR "Cannot install a binary without an absolute path (${_install_binary_BINARY}).")
  endif ()

  if (NOT _install_binary_DESTINATION)
    set(_install_binary_DESTINATION
      "\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}")
  endif ()

  if (NOT _install_binary_LIBDIR)
    message(FATAL_ERROR "Cannot install ${_install_binary_BINARY} without knowing where to put dependent libraries.")
  endif ()

  if (NOT _install_binary_TYPE)
    message(FATAL_ERROR "Cannot install ${_install_binary_BINARY} without knowing its type.")
  endif ()

  if (_install_binary_TYPE STREQUAL "module" AND NOT _install_binary_LOCATION)
    message(FATAL_ERROR "Cannot install ${_install_binary_BINARY} as a module without knowing where to place it.")
  endif ()

  if (NOT _install_binary_BINARY_LIBDIR)
    list(APPEND _install_binary_BINARY_LIBDIR "bin")
  endif()

  set(fixup_bundle_arguments)
  string(APPEND fixup_bundle_arguments
    " --destination ${_install_binary_DESTINATION}")
  string(APPEND fixup_bundle_arguments
    " --type ${_install_binary_TYPE}")
  string(APPEND fixup_bundle_arguments
    " --libdir ${_install_binary_LIBDIR}")

  get_property(superbuild_has_cleaned GLOBAL PROPERTY
    superbuild_has_cleaned)
  if (_install_binary_CLEAN OR NOT superbuild_has_cleaned)
    set_property(GLOBAL PROPERTY
      superbuild_has_cleaned TRUE)
    if (superbuild_is_install_target)
      string(APPEND fixup_bundle_arguments
        " --new")
    else ()
      string(APPEND fixup_bundle_arguments
        " --clean --new")
    endif ()
  endif ()

  if (_install_binary_LOCATION)
    string(APPEND fixup_bundle_arguments
      " --location \"${_install_binary_LOCATION}\"")
  endif ()

  foreach (include_regex IN LISTS _install_binary_INCLUDE_REGEXES)
    string(APPEND fixup_bundle_arguments
      " --include \"${include_regex}\"")
  endforeach ()

  foreach (exclude_regex IN LISTS _install_binary_EXCLUDE_REGEXES)
    string(APPEND fixup_bundle_arguments
      " --exclude \"${exclude_regex}\"")
  endforeach ()

  foreach (ignore_dllname IN LISTS _install_binary_IGNORE_DLLNAMES)
    string(APPEND fixup_bundle_arguments
      " --ignore \"${ignore_dllname}\"")
  endforeach ()

  foreach(binary_libdir IN LISTS _install_binary_BINARY_LIBDIR)
    string(APPEND fixup_bundle_arguments
      " --binary-libdir \"${binary_libdir}\"")
  endforeach()

  foreach (search_directory IN LISTS _install_binary_SEARCH_DIRECTORIES)
    string(APPEND fixup_bundle_arguments
      " --search \"${search_directory}\"")
  endforeach ()

  install(CODE
    "execute_process(
      COMMAND \"${superbuild_python_executable}\"
              \"${_superbuild_install_cmake_dir}/scripts/fixup_bundle.windows.py\"
              ${fixup_bundle_arguments}
              --manifest    \"${CMAKE_BINARY_DIR}/install.manifest\"
              \"${_install_binary_BINARY}\"
      RESULT_VARIABLE res
      ERROR_VARIABLE  err)

    if (res)
      message(FATAL_ERROR \"Failed to install ${name}:\n\${err}\")
    endif ()"
    COMPONENT superbuild)
endfunction ()

# A convenience function for installing an executable.
function (_superbuild_windows_install_executable path libdir)
  _superbuild_windows_install_binary(
    BINARY      "${path}"
    LIBDIR      "${libdir}"
    TYPE        executable
    ${ARGN})
endfunction ()

# A convenience function for installing a module.
function (_superbuild_windows_install_module path subdir libdir)
  _superbuild_windows_install_binary(
    BINARY      "${path}"
    LOCATION    "${subdir}"
    LIBDIR      "${libdir}"
    TYPE        module
    ${ARGN})
endfunction ()

#[==[.md
### Executables

Non-forwarding executables are binaries that may not work in the package
without the proper rpaths or `LD_LIBRARY_PATH` set when running the executable.

```
superbuild_windows_install_program(<NAME> <LIBDIR>)
```

Installs a program at `NAME` into `bin/` and its dependent libraries into
`LIBDIR` under the install destination. The program is assumed to be in the
installation prefix as `bin/<NAME>.exe`.

The following arguments are set by calling this function:

  - `BINARY`
  - `LIBDIR`
  - `TYPE` (`executable`)
#]==]
function (superbuild_windows_install_program name libdir)
  _superbuild_windows_install_executable("${superbuild_install_location}/bin/${name}.exe" "${libdir}" ${ARGN})
endfunction ()

#[==[.md
### Plugins

Plugins include libraries that are meant to be loaded at runtime. It also
includes libraries that are linked to, but need to be installed separately.

```
superbuild_windows_install_plugin(<FILENAME> <LIBDIR> <SEARCH PATHS> [<ARG>...])
```

Installs a library named `FILENAME` into `bin/` and its dependent libraries
into `LIBDIR` under the install destination. The given filename is searched for
under each path in the `SEARCH PATHS` argument.

Note that `SEARCH PATHS` is a CMake list passed as a single argument.

The following arguments are set by calling this function:

  - `BINARY`
  - `LIBDIR`
  - `LOCATION`
  - `TYPE` (`module`)
#]==]
function (superbuild_windows_install_plugin name libdir paths)
  if (IS_ABSOLUTE "${name}")
    _superbuild_windows_install_module("${name}" "${paths}" "${libdir}" ${ARGN})
    return ()
  endif ()

  set(found FALSE)
  foreach (path IN LISTS paths)
    if (EXISTS "${superbuild_install_location}/${path}/${name}")
      _superbuild_windows_install_module("${superbuild_install_location}/${path}/${name}" "${path}" "${libdir}" ${ARGN})
      set(found TRUE)
      break ()
    endif ()
  endforeach ()

  if (NOT found)
    string(REPLACE ";" ", " paths_list "${paths}")
    message(FATAL_ERROR "Unable to find the ${name} plugin in ${paths_list}")
  endif ()
endfunction ()

#[==[.md
### Python packages

The superbuild also provides functions to install Python modules and packages.

```
superbuild_windows_install_python(
  MODULES <module>...
  MODULE_DIRECTORIES <module-path>...
  [MODULE_DESTINATION <destination>]
  [INCLUDE_REGEXES <include-regex>...]
  [EXCLUDE_REGEXES <exclude-regex>...]
  [IGNORE_DLLNAMES <ignore-name>...]
  [SEARCH_DIRECTORIES <library-path>...])
```

The list of modules to installed is given to the `MODULES` argument. These
modules (or packages) are searched for at install time in the paths given to
the `MODULE_DIRECTORIES` argument.

Modules are placed in the `MODULE_DESTINATION` under the expected Python module
paths in the package (`bin/Lib`). By default, `/site-packages` is used.

The `INCLUDE_REGEXES`, `EXCLUDE_REGEXES`, `IGNORE_DLLNAMES`, and
`SEARCH_DIRECTORIES` used when installing compiled Python modules through
`superbuild_windows_install_plugin`.

Note that modules in the list which cannot be found are ignored.
#]==]
function (superbuild_windows_install_python)
  set(values
    MODULE_DESTINATION)
  set(multivalues
    INCLUDE_REGEXES
    EXCLUDE_REGEXES
    IGNORE_DLLNAMES
    SEARCH_DIRECTORIES
    MODULE_DIRECTORIES
    MODULES)
  cmake_parse_arguments(_install_python "${options}" "${values}" "${multivalues}" ${ARGN})

  if (NOT _install_python_MODULES)
    message(FATAL_ERROR "No modules specified.")
  endif ()

  if (NOT _install_python_MODULE_DIRECTORIES)
    message(FATAL_ERROR "No modules search paths specified.")
  endif ()

  set(fixup_bundle_arguments)

  if (NOT _install_python_MODULE_DESTINATION)
    set(_install_python_MODULE_DESTINATION "/site-packages")
  endif ()

  foreach (include_regex IN LISTS _install_python_INCLUDE_REGEXES)
    list(APPEND fixup_bundle_arguments
      --include "${include_regex}")
  endforeach ()

  foreach (exclude_regex IN LISTS _install_python_EXCLUDE_REGEXES)
    list(APPEND fixup_bundle_arguments
      --exclude "${exclude_regex}")
  endforeach ()

  foreach (ignore_dllname IN LISTS _install_python_IGNORE_DLLNAMES)
    list(APPEND fixup_bundle_arguments
      --ignore "${ignore_dllname}")
  endforeach ()

  foreach (search_directory IN LISTS _install_python_SEARCH_DIRECTORIES)
    list(APPEND fixup_bundle_arguments
      --search "${search_directory}")
  endforeach ()

  install(CODE
    "set(superbuild_python_executable \"${superbuild_python_executable}\")
    include(\"${_superbuild_install_cmake_dir}/scripts/fixup_python.windows.cmake\")
    set(python_modules \"${_install_python_MODULES}\")
    set(module_directories \"${_install_python_MODULE_DIRECTORIES}\")

    set(fixup_bundle_arguments \"${fixup_bundle_arguments}\")
    set(bundle_destination \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}\")
    set(bundle_manifest \"${CMAKE_BINARY_DIR}/install.manifest\")

    foreach (python_module IN LISTS python_modules)
      superbuild_windows_install_python_module(\"\${CMAKE_INSTALL_PREFIX}\"
        \"\${python_module}\" \"\${module_directories}\" \"bin/Lib${_install_python_MODULE_DESTINATION}\")
    endforeach ()"
    COMPONENT superbuild)
endfunction ()
