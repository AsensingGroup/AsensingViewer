#[==[.md
@brief Declare plugin file(s) for a project in superbuild

~~~
superbuild_declare_paraview_xml_files(<project>
  FILE_NAMES <filename>...
  [OMIT_PLUGINS <plugin_name>...]
  [DIRECTORY_HINTS <hint>...])
~~~

All values are optional unless otherwise noted. The following arguments are
supported:

  * `FILE_NAMES`: (Required) List of ParaView XML plugin files to declare for this project
  * `OMIT_PLUGINS`: List of plugins to omit by default
  * `DIRECTORY_HINTS`: List of path hints
#]==]
function (superbuild_declare_paraview_xml_files project)
  if (superbuild_build_phase AND ${project}_enabled)
    cmake_parse_arguments(PARSE_ARGV 1 plugin
      ""
      ""
      "FILE_NAMES;DIRECTORY_HINTS;OMIT_PLUGINS")

    if (NOT plugin_FILE_NAMES)
      message(FATAL_ERROR "Missing argument FILE_NAMES")
    endif ()

    if (plugin_UNPARSED_ARGUMENTS)
      message(FATAL_ERROR
        "Unrecognized arguments passed to `superbuild_declare_paraview_xml_files`: "
        "${plugin_UNPARSED_ARGUMENTS}")
    endif ()

    get_property(project_list GLOBAL PROPERTY sb_projects_with_plugins)
    if (NOT project IN_LIST project_list)
      set_property(GLOBAL APPEND PROPERTY sb_projects_with_plugins "${project}")
    endif ()
    foreach (file IN LISTS plugin_FILE_NAMES)
      foreach (ex_project IN LISTS project_list)
        get_property(plugin_files GLOBAL PROPERTY "${ex_project}_plugin_files")
        if (file IN_LIST plugin_files)
          message(SEND_ERROR
            "Duplicate paraview XML (${file}) declared in ${project}\n"
            "Previously declared in ${ex_project}")
        endif ()
      endforeach ()
    endforeach ()

    set_property(GLOBAL APPEND PROPERTY "${project}_plugin_files" "${plugin_FILE_NAMES}")
    set_property(GLOBAL APPEND PROPERTY "${project}_plugin_directory_hints" "${plugin_DIRECTORY_HINTS}")
    if (plugin_OMIT_PLUGINS)
      set_property(GLOBAL APPEND PROPERTY "${project}_plugins_omit" "${plugin_OMIT_PLUGINS}")
    endif ()
  endif ()
endfunction ()

#[==[.md
@brief Binding a plugin file(s) to a project in superbuild

~~~
superbuild_omit_plugins_from_project(<project>
  [PLUGINS <plugin_name>...])
~~~

  * `PLUGINS`: List of plugins to omit by default
#]==]
function (superbuild_omit_plugins_from_project project)
  get_property(project_has_plugins GLOBAL PROPERTY "${project}_plugin_files" SET)
  if (project_has_plugins AND superbuild_build_phase AND ${project}_enabled)
    cmake_parse_arguments(PARSE_ARGV 1 omit
      ""
      ""
      "PLUGINS")
    if (omit_UNPARSED_ARGUMENTS)
      message(FATAL_ERROR
        "Unrecognized arguments passed to `superbuild_omit_plugins_from_project`: "
        "${omit_UNPARSED_ARGUMENTS}")
    endif ()
    set_property(GLOBAL APPEND PROPERTY "${project}_plugins_omit" "${omit_PLUGINS}")
  endif ()
endfunction ()
