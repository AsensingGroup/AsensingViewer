#[==[.md
# Specifying revisions

When building a package, its sources must come from somewhere. In order to keep
the "what to build" (sources) separate from the "how to build" (the project
files) and separately updatable, the superbuild gathers source information
separately.

There are multiple ways to specify a revision with the functions in the
superbuild, but in general, a package will use the first-specified revision
that is encountered. This is so that a specific project can, for example, use a
different version of Boost than is provided in the common superbuild
infrastructure.
#]==]

include(CMakeParseArguments)

#[==[.md
## Simple use case

The simplest use case is when a project just has a given location for its
sources that isn't configurable at build time by a user.

```
superbuild_set_revision(<NAME> <ARG>...)
```

The `superbuild_set_revision` function stores the given `ARG` for use when then
`NAME` project is built. See the documentation for [ExternalProject][] for the
supported download location arguments.

If `<NAME>_SKIP_VERIFICATION` is defined and evaluates to TRUE, then URL_MD5 and
URL_HASH arguments passed to this function are skipped thus avoiding any archive
verification for the project.

Note that validation of the arguments only happens when the project is being
built.

[ExternalProject]: https://cmake.org/cmake/help/v3.9/module/ExternalProject.html
#]==]
function (superbuild_set_revision name)
  get_property(have_revision GLOBAL
    PROPERTY
      "${name}_revision" SET)

  cmake_parse_arguments(_rev_args "FORCE" "" "" ${ARGN})

  if (have_revision AND NOT _rev_args_FORCE)
    return ()
  endif ()

  set(args "${_rev_args_UNPARSED_ARGUMENTS}")

  if (${name}_SKIP_VERIFICATION)
    set(keys URL_HASH URL_MD5)
    cmake_parse_arguments(_args "" "${keys}" "" ${args})
    set(args "${_args_UNPARSED_ARGUMENTS}")
  endif()
  set_property(GLOBAL
    PROPERTY
      "${name}_revision" "${args}")
endfunction ()

#[==[.md INTERNAL
## Customizable revisions

```
_superbuild_set_customizable_revision(<NAME> <ARG>...)
```

This is used for `CUSTOMIZABLE` revisions in the
`superbuild_set_selectable_source` function. The following keys are turned into
cache variables. Others are ignored.

  - `GIT_REPOSITORY`
  - `GIT_TAG`
  - `URL`
  - `URL_HASH`
  - `URL_MD5`
  - `SOURCE_DIR`

The cache variables are named `<NAME>_<KEY>`.
#]==]
function (_superbuild_set_customizable_revision name)
  set(keys
    GIT_REPOSITORY GIT_TAG
    URL URL_HASH URL_MD5
    SOURCE_DIR)
  cmake_parse_arguments(_args "FORCE" "${keys}" "" ${ARGN})
  set(customized_args)

  foreach (key IN LISTS keys)
    if (_args_${key})
      set(option_name "${name}_${key}")
      set(cache_type STRING)
      if (key STREQUAL "SOURCE_DIR")
        set(cache_type PATH)
      endif ()
      if (_args_FORCE)
        set("${option_name}" "${_args_${key}}"
          CACHE "${cache_type}" "${key} for project '${name}'" FORCE)
      else ()
        set("${option_name}" "${_args_${key}}"
          CACHE "${cache_type}" "${key} for project '${name}'")
      endif ()
      if (NOT key STREQUAL "SOURCE_DIR")
        mark_as_advanced(${option_name})
      else ()
        mark_as_advanced(CLEAR ${option_name})
      endif ()
      list(APPEND customized_args
        "${key}" "${${option_name}}")
    endif ()
  endforeach ()
  set(use_force)
  if (_args_FORCE)
    set(use_force FORCE)
  endif ()
  superbuild_set_revision("${name}"
    ${customized_args}
    ${_args_UNPARSED_ARGUMENTS} ${use_force})
endfunction ()

#[==[.md
## User-selectable sources

Some projects may have different locations for sources that a user might want
to choose between. To facilitate this, a project may have a "selectable"
source. This creates a user-facing cache variable `<NAME>_SOURCE_SELECTION`
which chooses a selection that then indicates where the sources should be
retrieved. It is an error to choose a selection that does not exist.

The signature is:

```
superbuild_set_selectable_source(<NAME>
  [SELECTS_WITH <PARENT>]
  <SELECT <SELECTION> [DEFAULT] [CUSTOMIZABLE] [PROMOTE] [FALLBACK]
    <ARG>...>...)
```

Each selection is followed by a set of arguments which is used as the source
arguments for the project if it is used. The default selection is either the
one marked by the `DEFAULT` argument or the first selection if none is
specified.

A selection may be `CUSTOMIZABLE` which means that the values to the arguments
may be edited by the user. This implies `PROMOTE` because the values are in the
cache as `<NAME>_<FIELD>`. Adding `PROMOTE` will ensure the calling scope has
the keys set with the same variable names.

A project may also `SELECTS_WITH` another project. If given, the selection of
the `PARENT` project will be used as the selection for this project as well if
it exists. Rather than the `DEFAULT` keyword, a `SELECTS_WITH` project may use
`FALLBACK` which indicates the selection that should be used if the parent
project uses a selection that is not valid for the current project.

Some conventions are used for certain selections, but are not enforced.
Usually, at least the `git` and `source` selections are available for "primary"
projects within a superbuild. Both of these should be marked as `CUSTOMIZABLE`.

As an example:

```cmake
superbuild_set_selectable_source(myproject
  SELECT v1.0
    URL     "https://hostname/path/to/myproject-1.0.tar.gz"
    URL_MD5 00000000000000000000000000000000
  SELECT v2.0 DEFAULT
    URL     "https://hostname/path/to/myproject-2.0.tar.gz"
    URL_MD5 00000000000000000000000000000000
  SELECT git CUSTOMIZABLE
    GIT_REPOSITORY  "https://path/to/myproject.git"
    GIT_TAG         "origin/master"
  SELECT source CUSTOMIZABLE
    SOURCE_DIR  "path/to/local/directory")

superbuild_set_selectable_source(myprojectdocs
  SELECTS_WITH myproject
  SELECT v1.0
    URL     "https://hostname/path/to/myprojectdocs-1.0.tar.gz"
    URL_MD5 00000000000000000000000000000000
  SELECT v2.0
    URL     "https://hostname/path/to/myprojectdocs-2.0.tar.gz"
    URL_MD5 00000000000000000000000000000000
  SELECT git FALLBACK
    GIT_REPOSITORY  "https://path/to/myprojectdocs.git"
    GIT_TAG         "origin/master")
```

In this example, the `myproject` project defaults to the `v2.0` selection. In
addition, a `v1.0` is available and the other two are `CUSTOMIZABLE` which
allows a user to set the values with the given arguments. The `myprojectdocs`
project indicates that it `SELECTS_WITH` `myproject`. This means that if the
selection for `myproject` exists for `myprojectdocs` as well, it will be used.
However, if it does not exist, the selection marked as the `FALLBACK` will be
used instead.
#]==]
function (superbuild_set_selectable_source name)
  get_property(have_revision GLOBAL
    PROPERTY
      "${name}_revision" SET)

  if (have_revision)
    return ()
  endif ()

  set(selections)
  set(customizable_selections)
  set(promote_selections)

  set(selection_name)
  set(selection_args)
  set(default_selection)
  set(fallback_selection)
  set(first_selection)
  set(selects_with)

  set(grab)

  foreach (arg IN LISTS ARGN)
    if (arg STREQUAL "SELECTS_WITH")
      if (selects_with)
        message(FATAL_ERROR
          "The ${name} package may only select with a single other project.")
      endif ()

      if (selections)
        message(FATAL_ERROR
          "The `SELECTS_WITH` specifier must occur first.")
      endif ()

      set(grab selects_with)
    elseif (arg STREQUAL "SELECT")
      if (selection_name)
        # Store the first selection.
        if (NOT first_selection)
          set(first_selection "${selection_name}")
        endif ()

        # Make sure there are arguments.
        if (NOT selection_args)
          message(FATAL_ERROR
            "The ${selection_name} is missing arguments")
        endif ()

        list(APPEND selections
          "${selection_name}")
        set("selection_${selection_name}_args"
          "${selection_args}")
      endif ()

      # Clear the selection data.
      set(selection_name)
      set(selection_args)

      set(grab selection_name)
    elseif (arg STREQUAL "DEFAULT")
      # Error out on duplicate defaults.
      if (default_selection)
        message(FATAL_ERROR
          "The ${name} package may only have one default source selection.")
      endif ()

      # Error out if DEFAULT is not after a name.
      if (NOT selection_name)
        message(FATAL_ERROR
          "A `DEFAULT` specifier must come after a selection name.")
      endif ()

      # Error out if DEFAULT is not before the args.
      if (selection_args)
        message(FATAL_ERROR
          "A `DEFAULT` specifier must come before the selection args.")
      endif ()

      # Error out if we're using a separate selection.
      if (selects_with)
        message(FATAL_ERROR
          "A `DEFAULT` selection is not allowed with `SELECTS_WITH`.")
      endif ()

      set(default_selection "${selection_name}")
    elseif (arg STREQUAL "FALLBACK")
      # Error out on duplicate fallbacks.
      if (fallback_selection)
        message(FATAL_ERROR
          "The ${name} package may only have one fallback source selection.")
      endif ()

      # Error out if FALLBACK is not after a name.
      if (NOT selection_name)
        message(FATAL_ERROR
          "A `FALLBACK` specifier must come after a selection name.")
      endif ()

      # Error out if FALLBACK is not before the args.
      if (selection_args)
        message(FATAL_ERROR
          "A `FALLBACK` specifier must come before the selection args.")
      endif ()

      # Error out if we're using a separate selection.
      if (NOT selects_with)
        message(FATAL_ERROR
          "A `FALLBACK` selection is only allowed with `SELECTS_WITH`.")
      endif ()

      set(fallback_selection "${selection_name}")
    elseif (arg STREQUAL "CUSTOMIZABLE")
      # Error out if CUSTOMIZABLE is not after a name.
      if (NOT selection_name)
        message(FATAL_ERROR
          "A `CUSTOMIZABLE` specifier must come after a selection name.")
      endif ()

      # Error out if CUSTOMIZABLE is not before the args.
      if (selection_args)
        message(FATAL_ERROR
          "A `CUSTOMIZABLE` specifier must come before the selection args.")
      endif ()

      list(APPEND customizable_selections
        "${selection_name}")
    elseif (arg STREQUAL "PROMOTE")
      # Error out if PROMOTE is not after a name.
      if (NOT selection_name)
        message(FATAL_ERROR
          "A `PROMOTE` specifier must come after a selection name.")
      endif ()

      # Error out if PROMOTE is not before the args.
      if (selection_args)
        message(FATAL_ERROR
          "A `PROMOTE` specifier must come before the selection args.")
      endif ()

      list(APPEND promote_selections
        "${selection_name}")
    elseif (grab)
      # Store the argument.
      list(APPEND "${grab}"
        "${arg}")

      # If we just got the name, store future arguments in the arguments.
      if ("x${grab}" STREQUAL "xselection_name")
        set(grab selection_args)
      # If we just got the "selects with" project, drop future arguments.
      elseif ("x${grab}" STREQUAL "xselects_with")
        set(grab)
      endif ()
    endif ()
  endforeach ()

  if (selection_name)
    # Store the first selection.
    if (NOT first_selection)
      set(first_selection "${selection_name}")
    endif ()

    # Make sure there are arguments.
    if (NOT selection_args)
      message(FATAL_ERROR
        "The ${selection_name} is missing arguments")
    endif ()

    list(APPEND selections
      "${selection_name}")
    set("selection_${selection_name}_args"
      "${selection_args}")

    set(selection_name)
    set(selection_args)
  endif ()

  # Check that there's at least one selection.
  if (NOT selections)
    message(FATAL_ERROR
      "The ${name} project did not provide any selections")
  endif ()

  # Allow setting the default selection from the top-level build.
  if (DEFINED _superbuild_${name}_default_selection)
    set(default_selection "${_superbuild_${name}_default_selection}")
  endif ()

  # Use the first as the default if one was not specified.
  if (NOT selects_with AND NOT default_selection)
    message(WARNING
      "Using the ${first_selection} selection as the default for ${name} "
      "because no default was specified.")
    set(default_selection "${first_selection}")
  endif ()

  if (selects_with)
    set(selection "${${selects_with}_SOURCE_SELECTION}")

    if (NOT selection_${selection}_args AND fallback_selection)
      set(selection "${fallback_selection}")
    endif ()

    set("${name}_SOURCE_SELECTION" "${selection}"
      CACHE INTERNAL "The source selection for ${name}; based off of ${selects_with}")
  else ()
    set("${name}_SOURCE_SELECTION" "${default_selection}"
      CACHE STRING "The source selection for ${name}")
    set_property(CACHE "${name}_SOURCE_SELECTION"
      PROPERTY
        STRINGS "${selections}")
    set(selection "${${name}_SOURCE_SELECTION}")
  endif ()

  if (NOT selection_${selection}_args)
    string(REPLACE ";" "`, `" available "${selections}")
    message(FATAL_ERROR
      "The ${selection} source selection for ${name} does not exist. This "
      "selection may have existed previously; edit the "
      "`${name}_SOURCE_SELECTION` variable as necessary. Available "
      "selections: `${available}`.")
  endif ()

  # Setup global properties so selection may be changed after
  set_property(GLOBAL PROPERTY "_${name}_selections" ${selections})
  foreach (select IN LISTS selections)
    set_property(GLOBAL PROPERTY "_${name}_${select}_selection_args" ${selection_${select}_args})
    if (select IN_LIST customizable_selections)
      set_property(GLOBAL PROPERTY "_${name}_${select}_CUSTOMIZABLE" TRUE)
    endif ()
    if (select IN_LIST promote_selections)
      set_property(GLOBAL PROPERTY "_${name}_${select}_PROMOTE" TRUE)
    endif ()
  endforeach ()

  if (selection IN_LIST customizable_selections)
    _superbuild_set_customizable_revision("${name}" ${selection_${selection}_args})
  else ()
    if (selection IN_LIST promote_selections)
      set(keys
        GIT_REPOSITORY GIT_TAG
        URL URL_HASH URL_MD5
        SOURCE_DIR)
      cmake_parse_arguments(_args "" "${keys}" "" ${selection_${selection}_args})

      foreach (key IN LISTS keys)
        if (_args_${key})
          set("${name}_${key}" "${_args_${key}}" PARENT_SCOPE)
        endif ()
      endforeach ()
    endif ()

    superbuild_set_revision("${name}" ${selection_${selection}_args})
  endif ()
endfunction ()

function (superbuild_set_source_selection name selection)

  if (superbuild_build_phase AND (NOT ${${name}_SOURCE_SELECTION} STREQUAL selection))
    message (FATAL_ERROR "Cannot change the source selection after the build phase begins.")
  endif ()

  get_property(_selections GLOBAL PROPERTY _${name}_selections)
  if (NOT selection IN_LIST _selections)
    message (FATAL_ERROR "Cannot find ${selection} as an selection option for ${name}.")
  endif ()

  cmake_parse_arguments(_selection "CUSTOMIZABLE;PROMOTE" "" "ARGS" ${ARGN})

  if (NOT _selection_PROMOTE)
    get_property(_selection_PROMOTE GLOBAL PROPERTY _${name}_${selection}_PROMOTE SET)
  endif ()
  if (NOT _selection_CUSTOMIZABLE)
    get_property(_selection_CUSTOMIZABLE GLOBAL PROPERTY _${name}_${selection}_CUSTOMIZABLE SET)
  endif ()
  if (NOT _selection_ARGS)
    get_property(_selection_ARGS GLOBAL PROPERTY _${name}_${selection}_selection_args)
  endif ()

  # Update the source selection
  set_property(CACHE "${name}_SOURCE_SELECTION" PROPERTY VALUE ${selection})

  if (_selection_CUSTOMIZABLE)
    _superbuild_set_customizable_revision("${name}" ${_selection_ARGS} FORCE)
  endif ()

  if (_selection_PROMOTE)
    set(keys
      GIT_REPOSITORY GIT_TAG
      URL URL_HASH URL_MD5
      SOURCE_DIR)
    cmake_parse_arguments(_args "" "${keys}" "" ${_selection_ARGS})

    foreach (key IN LISTS keys)
      if (_args_${key})
        set("${name}_${key}" "${_args_${key}}" PARENT_SCOPE)
      endif ()
    endforeach ()
  endif ()

  if (NOT _selection_CUSTOMIZABLE)
    superbuild_set_revision("${name}" ${_selection_ARGS} FORCE)
  endif ()
endfunction ()
