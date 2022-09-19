# Superbuild user variables

## Targets

  - `download-all` This target causes the download steps of all projects to be
    run. This essentially creates a cache of source files so that a build can
    be run without network access. See `SUPERBUILD_OFFLINE_BUILD` for handling
    VCS repositories in offline mode.

## Building

### `CMAKE_BUILD_TYPE_<PROJECT>`

Some projects may support having their build configuration chosen independently
of the overall superbuild. If so, this variable is exposed to change its value.
By default, the `<same>` value is used which uses the same build type as the
superbuild (`CMAKE_BUILD_TYPE`).

### `DEVELOPER_MODE_<PROJECT>`

If a project is enabled and supports developer mode, this variable controls
whether it uses the developer mode or not. When a project is in developer mode,
it is not built, but instead it is treated as "enabled" and an associated
`<PROJECT>-developer-config.cmake` file is written into the build directory of
the superbuild. This may be used with `-C` on a standalone build of the
associated project to have it use the superbuild dependencies. Note that
updating environment variables such as `PATH`, `LD_LIBRARY_PATH`,
`DYLD_LIBRARY_PATH`, or `PKG_CONFIG_PATH` to include the relevant paths in the
superbuild install prefix may be required in order to use the resulting build
tree.

### `ENABLE_<PROJECT>`

If set, the associated project will be built. All dependent required projects
are enabled as well.

### `SUPERBUILD_DEFAULT_INSTALL`

If the project creates an install target, it is installed as a specific package
layout. This variable allows for the selection of which package should be
installed.

### `SUPERBUILD_OFFLINE_BUILD`

If set, all implicit updates of direct VCS repositories will be disabled. This
means that if a project is watching the `master` branch of its source
repository, a reconfigure will not trigger a `git fetch` unless the source
directory does not exist at all. This is useful when performing builds on
machines with limited network access.

### `SUPERBUILD_PACKAGE_MODE`

For projects which support it, the package to build for this project.

### `SUPERBUILD_PROJECT_PARALLELISM`

This defaults to the number of CPUs on the machine. It is used as the `-j`
argument for `Makefiles`-based CMake generators. If the number of CPUs cannot
be determined, a reasonably beefy machine is assumed (8 cores).

### `SUPERBUILD_SKIP_PYTHON_PROJECTS`

If set, Python projects are not actually built by the superbuild instead it
merely results in the generation of a `requirements.txt` file which all the
Python packages enabled.

### `SUPERBUILD_USE_JOBSERVER`

If using a `Makefiles` generator, `make`-using projects may be configured to
use the number of jobs given at the top-level rather than per project. Note
that when this is set, the top-level build is no longer implicitly parallel, so
a `-j` flag is required to get a parallel build.

### `SUPPRESS_<PROJECT>_OUTPUT`

If set, output from the build and install steps of the given project will be
logged to a file instead of appearing in the main build output. This can be
used to hide project warnings and errors from CDash and interactive builds.

### `USE_SYSTEM_<PROJECT>`

Some projects may support finding an external ("system") copy of the project.
Those that do offer this variable which, if set, directs the project to find an
external copy of itself.

## Advanced variables

These variables are workarounds or control internal features. They should only
be used if needed. If these end up being necessary for your builds, please open
a report on the project's page so that we can better track down when these
workarounds are necessary.

### `PASS_LD_LIBRARY_PATH_FOR_BUILDS`

On Linux, there may be cases where errors of the form:

```
libz.so.1: no version information available
```

may appear. In this case, the `PASS_LD_LIBRARY_PATH_FOR_BUILDS` variable should
be turned off.
