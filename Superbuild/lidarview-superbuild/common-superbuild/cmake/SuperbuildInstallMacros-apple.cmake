# TODO: error on unrecognized arguments

#[==[.md
## Mach-O (macOS)

The superbuild installs Mach-O binaries using a core function to construct an
`.app` bundle using the `fixup_bundle.apple.py` script with the correct
arguments. It tries to emulate an Mach-O runtime loader to determine where to
find dependent files and it copies them to the installation directory. It also
fixes up internal library references so that the resulting package is
self-contained and relocatable.

### Create an application bundle.

```
superbuild_apple_create_app(<DESTINATION> <NAME> <BINARY>
  [INCLUDE_REGEXES <regex>...]
  [EXCLUDE_REGEXES <regex>...]
  [IGNORE_REGEXES <regex>...]
  [SEARCH_DIRECTORIES <library-path>...]
  [PLUGINS <plugin>...]
  [ADDITIONAL_LIBRARIES <library-path>...]
  [FAKE_PLUGIN_PATHS] [CLEAN])
```

Creates a `<NAME>.app` bundle. The bundle is placed in `<DESTINATION>` with
`<BINARY>` (an absolute path) as a main executable for the bundle (under the
`MacOS/` directory). Libraries are searched for and placed into the bundle from
the `SEARCH_DIRECTORIES` specified. Library IDs and link paths are rewritten to
use `@executable_path` or `@loader_path` as necessary.

To exclude libraries from the bundle, use Python regular expressions as
arguments to the `EXCLUDE_REGEXES` keyword. To include any otherwise-excluded
libraries, use `INCLUDE_REGEXES`. System libraries and frameworks are excluded
by default. References can be ignored if they match any of the given
`IGNORE_REGEXES`.

The `CLEAN` argument starts a new bundle, otherwise the bundle is left as-is
(and is expected to have been created by this call).

Plugins may be listed under the `PLUGINS` keyword and will be installed to the
`Plugins/` directory in the bundle. These are full paths to the plugin
binaries. If `FAKE_PLUGIN_PATHS` is given, the plugin is treated as its own
`@executable_path` which is useful when packaging plugins which may be used for
multiple applications and may require additional libraries depending on the
application.

Additional libraries may be listed under the ``ADDITIONAL_LIBRARIES`` keyword
and will be installed to the ``Libraries/`` directory in the bundle. These are
full paths to the libraries.
#]==]
function (superbuild_apple_create_app destination name binary)
  set(options
    CLEAN
    FAKE_PLUGIN_PATHS)
  set(multivalues
    INCLUDE_REGEXES
    EXCLUDE_REGEXES
    IGNORE_REGEXES
    SEARCH_DIRECTORIES
    PLUGINS
    ADDITIONAL_LIBRARIES)
  cmake_parse_arguments(_create_app "${options}" "" "${multivalues}" ${ARGN})

  set(fixup_bundle_arguments)

  if (_create_app_CLEAN)
    if (superbuild_is_install_target)
      string(APPEND fixup_bundle_arguments
        " --new")
    else ()
      string(APPEND fixup_bundle_arguments
        " --clean --new")
    endif ()
  endif ()

  if (_create_app_FAKE_PLUGIN_PATHS)
    string(APPEND fixup_bundle_arguments
      " --fake-plugin-paths")
  endif ()

  foreach (include_regex IN LISTS _create_app_INCLUDE_REGEXES)
    string(APPEND fixup_bundle_arguments
      " --include \"${include_regex}\"")
  endforeach ()

  foreach (exclude_regex IN LISTS _create_app_EXCLUDE_REGEXES)
    string(APPEND fixup_bundle_arguments
      " --exclude \"${exclude_regex}\"")
  endforeach ()

  foreach (ignore_regex IN LISTS _create_app_IGNORE_REGEXES)
    string(APPEND fixup_bundle_arguments
      " --ignore \"${ignore_regex}\"")
  endforeach ()

  foreach (search_directory IN LISTS _create_app_SEARCH_DIRECTORIES)
    string(APPEND fixup_bundle_arguments
      " --search \"${search_directory}\"")
  endforeach ()

  foreach (plugin IN LISTS _create_app_PLUGINS)
    string(APPEND fixup_bundle_arguments
      " --plugin \"${plugin}\"")
  endforeach ()

  foreach (library IN LISTS _create_app_ADDITIONAL_LIBRARIES)
    string(APPEND fixup_bundle_arguments
      " --library \"${library}\"")
  endforeach ()

  install(CODE
    "execute_process(
      COMMAND \"${_superbuild_install_cmake_dir}/scripts/fixup_bundle.apple.py\"
              --bundle      \"${name}\"
              --destination \"${destination}\"
              ${fixup_bundle_arguments}
              --manifest    \"${CMAKE_BINARY_DIR}/${name}.manifest\"
              --type        executable
              \"${binary}\"
      RESULT_VARIABLE res
      ERROR_VARIABLE  err)

    if (res)
      message(FATAL_ERROR \"Failed to install ${name}:\n\${err}\")
    endif ()"
    COMPONENT superbuild)
endfunction ()

#[==[.md
### Utility executables

```
superbuild_apple_install_utility(<DESTINATION> <NAME> <BINARY>
  [INCLUDE_REGEXES <regex>...]
  [EXCLUDE_REGEXES <regex>...]
  [IGNORE_REGEXES <regex>...]
  [SEARCH_DIRECTORIES <library-path>...]
  [FRAMEWORK_DEST <framework-dest>]
  [LIBRARY_DEST <library-dest>])
```

Adds a binary to the `bin/` path of the bundle. Required libraries are
installed and fixed up using `@executable_path`.

A previous call must have been made with matching `DESTINATION` and `NAME`
arguments; this call will not create a new application bundle.

The `INCLUDE_REGEXES`, `EXCLUDE_REGEXES`, `IGNORE_REGEXES`, and
`SEARCH_DIRECTORIES` arguments are the same as those for
`superbuild_apple_create_app`.
#]==]
function (superbuild_apple_install_utility destination name binary)
  set(values
    FRAMEWORK_DEST
    LIBRARY_DEST)
  set(multivalues
    INCLUDE_REGEXES
    EXCLUDE_REGEXES
    IGNORE_REGEXES
    SEARCH_DIRECTORIES)
  cmake_parse_arguments(_install_utility "" "${values}" "${multivalues}" ${ARGN})

  set(fixup_bundle_arguments)

  foreach (include_regex IN LISTS _install_utility_INCLUDE_REGEXES)
    string(APPEND fixup_bundle_arguments
      " --include \"${include_regex}\"")
  endforeach ()

  foreach (exclude_regex IN LISTS _install_utility_EXCLUDE_REGEXES)
    string(APPEND fixup_bundle_arguments
      " --exclude \"${exclude_regex}\"")
  endforeach ()

  foreach (ignore_regex IN LISTS _install_utility_IGNORE_REGEXES)
    string(APPEND fixup_bundle_arguments
      " --ignore \"${ignore_regex}\"")
  endforeach ()

  foreach (search_directory IN LISTS _install_utility_SEARCH_DIRECTORIES)
    string(APPEND fixup_bundle_arguments
      " --search \"${search_directory}\"")
  endforeach ()

  if (DEFINED _install_utility_FRAMEWORK_DEST)
    string(APPEND fixup_bundle_arguments
      " --framework-dest \"${_install_utility_FRAMEWORK_DEST}\"")
  endif ()

  if (DEFINED _install_utility_LIBRARY_DEST)
    string(APPEND fixup_bundle_arguments
      " --library-dest \"${_install_utility_LIBRARY_DEST}\"")
  endif ()

  install(CODE
    "execute_process(
      COMMAND \"${_superbuild_install_cmake_dir}/scripts/fixup_bundle.apple.py\"
              --bundle      \"${name}\"
              --destination \"${destination}\"
              ${fixup_bundle_arguments}
              --manifest    \"${CMAKE_BINARY_DIR}/${name}.manifest\"
              --type        utility
              \"${binary}\"
      RESULT_VARIABLE res
      ERROR_VARIABLE  err)

    if (res)
      message(FATAL_ERROR \"Failed to install ${name}:\n\${err}\")
    endif ()"
    COMPONENT superbuild)
endfunction ()

#[==[.md
### Module libraries

```
superbuild_apple_install_module(<DESTINATION> <NAME> <BINARY> <LOCATION>
  [INCLUDE_REGEXES <regex>...]
  [EXCLUDE_REGEXES <regex>...]
  [IGNORE_REGEXES <regex>...]
  [SEARCH_DIRECTORIES <library-path>...])
```

Adds a library to the `<LOCATION>` path of the bundle. Required libraries which
have not been installed by previous executable installs are installed and fixed
up using `@loader_path`. Use this to install things such as compiled language
modules and the like.

A previous call must have been made with matching `DESTINATION` and `NAME`
arguments; this call will not create a new application bundle.

The `INCLUDE_REGEXES`, `EXCLUDE_REGEXES`, `IGNORE_REGEXES`, and
`SEARCH_DIRECTORIES` arguments are the same as those for
`superbuild_apple_create_app`.
#]==]
function (superbuild_apple_install_module destination name binary location)
  set(multivalues
    INCLUDE_REGEXES
    EXCLUDE_REGEXES
    IGNORE_REGEXES
    SEARCH_DIRECTORIES)
  cmake_parse_arguments(_install_module "" "" "${multivalues}" ${ARGN})

  set(fixup_bundle_arguments)

  foreach (include_regex IN LISTS _install_module_INCLUDE_REGEXES)
    string(APPEND fixup_bundle_arguments
      " --include \"${include_regex}\"")
  endforeach ()

  foreach (exclude_regex IN LISTS _install_module_EXCLUDE_REGEXES)
    string(APPEND fixup_bundle_arguments
      " --exclude \"${exclude_regex}\"")
  endforeach ()

  foreach (ignore_regex IN LISTS _install_module_IGNORE_REGEXES)
    string(APPEND fixup_bundle_arguments
      " --ignore \"${ignore_regex}\"")
  endforeach ()

  foreach (search_directory IN LISTS _install_module_SEARCH_DIRECTORIES)
    string(APPEND fixup_bundle_arguments
      " --search \"${search_directory}\"")
  endforeach ()

  install(CODE
    "execute_process(
      COMMAND \"${_superbuild_install_cmake_dir}/scripts/fixup_bundle.apple.py\"
              --bundle      \"${name}\"
              --destination \"${destination}\"
              ${fixup_bundle_arguments}
              --manifest    \"${CMAKE_BINARY_DIR}/${name}.manifest\"
              --location    \"${location}\"
              --type        module
              \"${binary}\"
      RESULT_VARIABLE res
      ERROR_VARIABLE  err)

    if (res)
      message(FATAL_ERROR \"Failed to install ${name}:\n\${err}\")
    endif ()"
    COMPONENT superbuild)
endfunction ()

#[==[.md
### Python packages

The superbuild also provides functions to install Python modules and packages.

```
superbuild_apple_install_python(<DESTINATION> <NAME>
  MODULES <module>...
  MODULE_DIRECTORIES <module-path>...
  [SEARCH_DIRECTORIES <library-path>...])
```

The list of modules to installed is given to the `MODULES` argument. These
modules (or packages) are searched for at install time in the paths given to
the `MODULE_DIRECTORIES` argument.

Modules are placed in the `Python/` directory in the given application bundle.

A previous call must have been made with matching `DESTINATION` and `NAME`
arguments; this call will not create a new application bundle.

Note that modules in the list which cannot be found are ignored.
#]==]
function (superbuild_apple_install_python destination name)
  set(multivalues
    INCLUDE_REGEXES
    EXCLUDE_REGEXES
    IGNORE_REGEXES
    SEARCH_DIRECTORIES
    MODULE_DIRECTORIES
    MODULES)
  cmake_parse_arguments(_install_python "" "PYTHON_DESTINATION" "${multivalues}" ${ARGN})

  if (NOT _install_python_MODULES)
    message(FATAL_ERROR "No modules specified.")
  endif ()

  if (NOT _install_python_MODULE_DIRECTORIES)
    message(FATAL_ERROR "No modules search paths specified.")
  endif ()

  if (NOT _install_python_PYTHON_DESTINATION)
    set(_install_python_PYTHON_DESTINATION "Contents/Python")
  endif ()

  set(fixup_bundle_arguments)

  foreach (include_regex IN LISTS _install_python_INCLUDE_REGEXES)
    list(APPEND fixup_bundle_arguments
      --include "${include_regex}")
  endforeach ()

  foreach (exclude_regex IN LISTS _install_python_EXCLUDE_REGEXES)
    list(APPEND fixup_bundle_arguments
      --exclude "${exclude_regex}")
  endforeach ()

  foreach (ignore_regex IN LISTS _install_python_IGNORE_REGEXES)
    list(APPEND fixup_bundle_arguments
      --ignore "${ignore_regex}")
  endforeach ()

  foreach (search_directory IN LISTS _install_python_SEARCH_DIRECTORIES)
    list(APPEND fixup_bundle_arguments
      --search "${search_directory}")
  endforeach ()

  install(CODE
    "include(\"${_superbuild_install_cmake_dir}/scripts/fixup_python.apple.cmake\")
    set(python_modules \"${_install_python_MODULES}\")
    set(module_directories \"${_install_python_MODULE_DIRECTORIES}\")

    set(fixup_bundle_arguments \"${fixup_bundle_arguments}\")
    set(bundle_destination \"${destination}\")
    set(bundle_name \"${name}\")
    set(bundle_manifest \"${CMAKE_BINARY_DIR}/${name}.manifest\")

    foreach (python_module IN LISTS python_modules)
      superbuild_apple_install_python_module(\"\${bundle_destination}/\${bundle_name}\"
        \"\${python_module}\" \"\${module_directories}\" \"${_install_python_PYTHON_DESTINATION}\")
    endforeach ()"
    COMPONENT superbuild)
endfunction ()
