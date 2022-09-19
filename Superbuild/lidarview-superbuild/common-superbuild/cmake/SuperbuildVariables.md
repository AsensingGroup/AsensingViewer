# Superbuild variables

There are many variables that are used within the common superbuild
infrastructure to influence parts of the build that are project-controlled
rather than user-controlled. This documentation covers these variables.

## Projects

### `superbuild_build_phase`

This variable indicates whether the configure is in the "scan" phase where
dependencies are gathered or the "build" phase where the actual commands to be
used are created. During the "build" phase, the following variables are available:

  - `<PROJECT>_enabled` If set, the project is enabled in the superbuild.
  - `<PROJECT>_built_by_superbuild` If set, the project is also being built by
    the superbuild (versus being externally provided).

The values of these variables may only be trusted if `superbuild_build_phase`
is set. Generally, this means that they should not be used in conditionals to
determine dependencies of a project. Instead, triangle dependencies ("`A`
depends on `B` because `C` is enabled") should be handled by having an optional
dependency from `A` on `B` and `C` and if `superbuild_build_phase` is set,
erroring out if `C_enabled` is set and `B_enabled` is not.

### `_superbuild_help_string_<PROJECT>`

Instead of the default help string for the `ENABLE_<PROJECT>` option, a more
descriptive name may be provided.

### `superbuild_install_location`

This variable contains the path that will be given as the installation prefix
to all projects when building and installing. It is also where packaging steps
will look for files to install. If not specified, it defaults to
`${CMAKE_BINARY_DIR}/install`.

### `_superbuild_<PROJECT>_selectable`

If set, the project `PROJECT` is treated as if it has a `SELECTABLE` keyword.
See the documentation for [`superbuild_add_project`][superbuild_add_project].

[superbuild_add_project]: SuperbuildMacros.md#Adding-a-project-to-the-build

### `_superbuild_list_separator`

If a project needs to pass `;` as a flag to a build command, instead, this
variable should be used.

## Building

### `_superbuild_<PROJECT>_default_selection`

For projects which use a source selection, the default selection may be
overriden from the top-level by specifying the selection with this variable.

### `_superbuild_suppress_<PROJECT>_output`

If defined, this is used as the default value for the
`SUPPRESS_<PROJECT>_OUTPUT` variable for the given project.

### Language flags

The following variables contain (as a string, not a list), the flags to pass to
projects using `CFLAGS`, `CXXFLAGS`, and `LDFLAGS` as environment variables or
through CMake's cache variables.

  - `superbuild_c_flags`
  - `superbuild_cpp_flags`
  - `superbuild_cxx_flags`
  - `superbuild_ld_flags`

Superbuild projects may add flags to these by using:

  - `superbuild_extra_c_flags`
  - `superbuild_extra_cpp_flags`
  - `superbuild_extra_cxx_flags`

On Windows, these variables are not supported.

## Testing

### `superbuild_ctest_custom_files`

This is a list of paths to files which are included from the
`CTestCustom.cmake` file. It is read by CTest. Please see the
[CTest variable documentation][] for details (the `CTEST_CUSTOM_` variables).

[CTest variable documentation]: https://cmake.org/cmake/help/git-master/manual/cmake-variables.7.html#variables-for-ctest

## Packaging

These variables are used in packaging tests (mainly `.bundle.cmake` files).

### `superbuild_bundle_skip_system_libraries`

If set in a `bundle.cmake` file, it can be used to skip the
`InstallRequiredSystemLibraries` bit implicit in a packaging test.

### `superbuild_is_install_target`

If set, the package is being installed to the host system, not into a package.
Use of this is mostly internal, but it is available if necessary.

### Variables forwarded from the build

These variables are provided by the superbuild during the packaging step.

  - `superbuild_install_location`
  - `<PROJECT>_enabled`
  - `<PROJECT>_built_by_superbuild`
  - `USE_SYSTEM_<PROJECT>`

## Debugging

There are some debugging facilities provided. These variables control them.

### `SUPERBUILD_DEBUG_CONFIGURE_STEPS`

If set, projects will log their configure steps to a file. In addition, CMake
projects will be passed `--trace-expand`.
