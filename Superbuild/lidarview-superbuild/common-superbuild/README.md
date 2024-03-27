# Common Superbuild Infrastructure

This project started as a refactor of a number of projects which had started
using the ParaView superbuild, but had diverged over time. Features were
added, patches trickled back and forth, but the gap needed to be closed. This
repository represents the common ground between all of them and aims to do so
in a flexible way so that such gaps do not grow too large again.

# Updating projects

Existing projects are listed in `versions.cmake`, and generally point to an
archive in [this spot](https://www.paraview.org/files/dependencies). Ask
for someone who has permissions to push an updated archive, then update
`versions.cmake`. `md5sum` can calculate the hash for you.

# Defining projects

Projects are defined under the `projects/` hierarchy. Under this hierarchy,
files with the name `$project.cmake` are created which define how to build the
project. Other files which have precedence include `$project.common.cmake`,
`$project.functions.cmake`, and
`$project.system.cmake`.

  * `$project.common.cmake`: for projects which differ greatly based on the
    platform, this is intended to be included by platform-specific files and
    tweaked using variables.
  * `$project.functions.cmake`: some projects might offer functions to
    simplify things such as installation or testing. These functions should
    be defined in a separate file so that they may be available during
    packaging and testing as well.
  * `$project.system.cmake`: projects may represent software packages which
    are available as a system-wide copy; this file is used to find the system
    copy and import it into the superbuild infrastructure.

Different platforms can shadow these files by using the following
subdirectories:

  * Windows: `win32/`
  * OS X: `apple/` and `apple-unix/`
  * Linux: `unix/` and `apple-unix/`

Usually these will just be the `$project.cmake` files, patches, and helper
scripts, but it is not impossible that shadowing for the other files would be
useful.

## Related files

Some projects might need support files such as patches or small scripts to
complete some parts of the build step. For these files, the directories
`patches/` and `scripts/` should be used. The files should start with the name
of the project they are used for so that it is obvious why they exist.
Additionally, they should be underneath the directory for the `$project.cmake`
file which uses them. This keeps, e.g., Windows-specific patches underneath
the Windows platform directory.

Scripts should, preferably, be written in CMake code, but as a fallback,
Python should be preferred since it is generally available. Shell and Batch
scripts should be avoided.

## Guidelines

Within a `$project.cmake` and related files, some guidelines should be
followed:

  - clear variables before use: the two-pass setup used by the superbuild may
    cause variables to be non-empty on the second pass;
  - prefix all variables and functions with the project name: variables such
    as `extra_flags` and such might conflict with other projects;
  - comment complicated logic: projects might need non-trivial logic to work
    around weird bugs or other build failures; by documenting why this logic
    exists, the idea is that time will not be needed to rediscover the reason
    it exists.

## ParaView Plugins

ParaView plugins are prevalent in superbuilds. There are support APIs in the
[SuperbuildPluginMacros][plugin-macros] module.

Projects which provide ParaView XML files may use the
`superbuild_declare_paraview_xml_files` function to describe what plugin XML
files will be installed by the project. Packaging may access this information
via the following variables:

  - `projects_with_plugins`: A list of projects with associated plugin XML
    variables set.
  - `${project}_plugin_files`: A list of plugin XML files provided by the
    project.
  - `${project}_plugin_omit`: A list of plugins to omit from the XML files
    (usually because they are static).

Projects which consume plugins may use the
`superbuild_omit_plugins_from_project` function to exclude plugins from a
project.

## Examples

*TODO*: Add some examples of projects.

# Packages

Superbuilds may set the `superbuild_project_roots` variable to a list of paths
in which to discover packages (represented as a directory) for the project.
These packages may provide a `<package>.configure.cmake` file which is included
if it is selected as the primary package.

The default package is `<none>`, but may be set via the
`superbuild_package_default` variable.

Users may select a different default via the exposed `SUPERBUILD_PACKAGE_MODE`
cache variable.

Packages may set the `superbuild_extra_package_projects` variable to a list of
projects to include in the project as well.

# Using the superbuild infrastructure

To create a new superbuild, a repository should be created which contains the
superbuild infrastructure as a submodule. This allows updating to new versions
of the common infrastructure easily and keeps the version which is being used
easy to track.

There is only one required part (`superbuild_find_projects`), but other things
will generally be necessary for a new project, particularly
`superbuild_project_roots` and `superbuild_version_files`. See below for more
specific information on these. After defining the functions and setting up the
variables, all that is required is to add the infrastructure's directory using
`add_subdirectory`. It uses the functions and the variables values to do all
of the required work. This setup is done so that the infrastructure can do
work in between all of the functions without application superbuild needing to
call the right functions in between the different steps.

*Note*: For the purposes of the documentation, the new repository will be referred to
as pertaining to an application to keep the use of the term "project" less
ambiguous. By no means is the superbuild limited to building end-user
applications (though it is the primary use case).

## `superbuild_find_projects` (function)

The core bit which is required by the superbuild infrastructure to work is a
function called `superbuild_find_projects`. This function is used to populate
a variable (passed in as the argument) with the list of projects that are used
for the application. All projects which are required for the application
should be added to the list. As an example:

```cmake
function (superbuild_find_projects var)
    set(projects
        application dep1 dep2)

    if (WIN32)
        list(APPEND projects
            depforwin32)
    endif ()

    set("${var}" "${projects}" PARENT_SCOPE)
endfunction ()
```

Here, the application depends on two other projects, `dep` and `dep2`.
Additionally, on Windows, `depforwin32` is also a dependency.

The superbuild infrastructure will only consider these projects to exist. Any
other projects are completely ignored.

## `superbuild_project_roots` (variable)

This variable is a list of directory paths to treat as roots to look for
`$project.cmake` (and related) files. The relevant platforms will
automatically be searched for within each root. In addition, the common
infrastructure will put its project root at the end of this list.

The typical directory name for these roots is `projects/`. As an example:

```cmake
list(APPEND superbuild_project_roots
    "${CMAKE_CURRENT_SOURCE_DIR}/projects")
```

## `superbuild_version_files` (variable)

This variable is a list of files to include to declare version information for
the projects which will be built. The usual file name for these files is
`versions.cmake`. The following functions will be available in the file:

  - `superbuild_set_revision`
  - `superbuild_set_customizable_revision`
  - `superbuild_set_external_source`

See the [SuperbuildRevisionMacros][revision-macros] file for documentation on
these functions.

The common infrastructure will place its file at the end and it defines
version information for the projects it ships. Applications may override these
versions by defining its own in a file in this list.

## `superbuild_setup_variables` (function)

This function (though usually a macro), if defined, is called after revisions
are defined, but before any projects are included. This is where project-wide
variables should be set so that they are available everywhere in the project.

This is usually where the `superbuild_set_version_variables` function is
called (see the [SuperbuildVersionMacros][version-macros] file for its
documentation) so that the application's version information is available at
all times.

There is additionally the `superbuild_configure_project_version` function which
can extract more detailed version information from the project based on the
source selection in use. Special selection names:

  - `source`: Local revision information will be extracted
  - `git`: The repository's web hosting will be queried for version
    information of the relevant commit.
  - any other: Assumed to be the version number in question.

## `superbuild_sanity_check` (function)

This function, if defined, is called after setting up the entire build. At
this stage, variables of the form `${project}_enabled` are available which
indicate whether a project will be built or not. This function is the place to
check whether project settings conflict with each other or whether an
unsupported configuration has been chosen (e.g., not building the application,
or choosing incompatible versions of Python).

## Post-build tasks

The infrastructure supports running post-build tasks such as packaging and
testing. This uses CPack and CTest.

The preferred method to packaging is to set up an unrelated project which is
basically nothing except for installation logic. This is because CPack likes
to rerun the installation of the top-level project before it runs which can
retrigger builds of the main project. If projects are chasing branches in git
and the branch has updated since the build and packaging, all of a sudden
you're packaging different code than you had built. As such, the old,
deprecated process is not documented here.

### `superbuild_add_packaging` (function)

This function, if defined, should call the `superbuild_add_extra_package_test`
function (see [SuperbuildPackageMacros][package-macros] for documentation) to
create a test which will create a package using a specified generator. Tests
are described by `$project.bundle.cmake` files in the project root
directories.

The bundle files will have the following variables available:

  - `superbuild_install_location`
  - `superbuild_source_directory`
  - `${project}_enabled`
  - `USE_SYSTEM_${project}`

In addition, variables may be exported to the bundle project by setting the
`superbuild_export_variables` variable to a list of variable names to add.

See the [SuperbuildInstallMacros][install-macros] file for documentation on
the variables available in `$project.bundle.cmake` files.

Package suffixes may be computed using the `superbuild_package_suffix`
function, though this only supports basic information (version, platform, and
pointer size).

### `superbuild_add_tests` (function)

This function, if defined, will enable testing for the superbuild itself.
Before testing is enabled, the `superbuild_setup_tests` hook is called (if
set) so that any required variables may be set.

At the top-level (which should be fixed at some point; the function should do
this), the application may set the `superbuild_ctest_custom_files` variable to
a list of paths to `CTestCustom.cmake` files to be included during testing.
Within these files, convenience functions to ignore warnings and errors from
projects are provided:

  - `ignore_project_warnings(project)`
  - `ignore_project_errors(project)`

This may be used to silence warnings or errors from projects which cause noise
on the dashboard.

In addition, the `superbuild_test_projects` global property may be set to a
list of projects which should additionally be tested during a test of the
superbuild. This will cause the project's tests to be available and run as
part of a single top-level run of CTest. Cache variables named
`TEST_${project}` will be added so that the user can disable this behavior. If
the project is either disabled or in a developer mode, the tests are similarly
ignored.

## Advanced customization

### `superbuild_install_location` (variable)

This variable is a path where all of the projects will install themselves. If
it is not set, the default value is `${CMAKE_BINARY_DIR}/install`.

### `mpi_ENABLE_UCX` (variable)

This variable is useful for taking advantage of infiniband devices utilizing the
channel for ucx. This variable will typically be used in combination with the
`mpi_EXTRA_CONFIGURE_ARGUMENTS` advanced variable to specify ucx settings
during the MPI build. By default this is disabled as most machines won't have
this high bandwidth option available and will just use regular ethernet for MPI
communication.

### Flags

The common infrastructure will bring in flags from both the user's settings
and the application. User settings are grabbed from the `CMAKE_C_FLAGS`,
`CMAKE_CXX_FLAGS`, and `CMAKE_SHARED_LINKER_FLAGS` variables. Application
flags use `superbuild_ld_flags`, `superbuild_cpp_flags`,
`superbuild_cxx_flags`, and `superbuild_c_flags`.

[install-macros]: cmake/SuperbuildInstallMacros.cmake
[package-macros]: cmake/SuperbuildPackageMacros.cmake
[plugin-macros]: cmake/SuperbuildPluginMacros.cmake
[revision-macros]: cmake/SuperbuildRevisionMacros.cmake
[version-macros]: cmake/SuperbuildVersionMacros.cmake
