#[==[.md INTERNAL
# Apple utility functions

On Apple platforms (mainly macOS), some wrangling with version variables and
the link is required. These functions handle that. They are used internally to
the superbuild and do not need to be called manually.
#]==]

#[==[.md INTERNAL
## SDK support

CMake uses various `CMAKE_OSX_` variables to control the SDK in use. The
`superbuild_osx_add_version_flags` function extracts information from these
flags and adds them to the `superbuild_c_flags` and `superbuild_cxx_flags`
variables in the calling scope.

```
superbuild_osx_add_version_flags()
```
#]==]
function (superbuild_osx_add_version_flags)
  if (NOT APPLE)
    return ()
  endif ()

  set(osx_flags)
  if (CMAKE_OSX_ARCHITECTURES)
    list(APPEND osx_flags
      -arch "${CMAKE_OSX_ARCHITECTURES}")
  endif ()
  if (CMAKE_OSX_DEPLOYMENT_TARGET)
    list(APPEND osx_flags
      "-mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET}")
  endif ()
  string(REPLACE ";" " " osx_flags "${osx_flags}")

  foreach (var IN ITEMS cxx_flags c_flags f_flags ld_flags)
    set("superbuild_${var}"
      "${superbuild_${var}} ${osx_flags}"
      PARENT_SCOPE)
  endforeach ()
endfunction ()

#[==[.md INTERNAL
## Version flags

This `superbuild_osx_pass_version_flags` function sets the given variable to
contain a list of CMake `-D` command line flags to use the same Apple SDK,
architecture, and deployment target as the superbuild.

```
superbuild_osx_pass_version_flags(<VARIABLE>)
```
#]==]
function (superbuild_osx_pass_version_flags var)
  if (NOT APPLE)
    return ()
  endif ()

  set("${var}"
    -DCMAKE_OSX_ARCHITECTURES:STRING=${CMAKE_OSX_ARCHITECTURES}
    -DCMAKE_OSX_DEPLOYMENT_TARGET:STRING=${CMAKE_OSX_DEPLOYMENT_TARGET}
    -DCMAKE_OSX_SYSROOT:PATH=${CMAKE_OSX_SYSROOT}
    -DCMAKE_OSX_SDK:STRING=${CMAKE_OSX_SDK}
    PARENT_SCOPE)
endfunction ()
