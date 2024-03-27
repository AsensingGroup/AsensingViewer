set(_superbuild_install_cmake_scripts_dir "${CMAKE_CURRENT_LIST_DIR}")

function (superbuild_unix_install_python_module destination module search_paths location)
  foreach (search_path IN LISTS search_paths)
    if (EXISTS "${search_path}/${module}.py")
      file(INSTALL
        FILES       "${search_path}/${module}.py"
        DESTINATION "${destination}/${location}")
    endif ()

    if (EXISTS "${search_path}/${module}.zip")
      file(INSTALL
        FILES       "${search_path}/${module}.zip"
        DESTINATION "${destination}/${location}")
    endif()

    if (EXISTS "${search_path}/${module}.so")
      set(module_so "${search_path}/${module}.so")
    else ()
      # some modules have names that are prefixed with build specific extension
      # e.g. `kiwisolver.cpython-37m-x86_64-linux-gnu.so`, so we do a glob if
      # direct lookup fails.
      file(GLOB module_so LIST_DIRECTORIES false "${search_path}/${module}.*.so")
      if (module_so)
        list(GET module_so 0 module_so)
      endif()
    endif ()

    if (EXISTS "${module_so}")
      execute_process(
        COMMAND "${superbuild_python_executable}"
                "${_superbuild_install_cmake_scripts_dir}/fixup_bundle.unix.py"
                --destination "${bundle_destination}"
                ${fixup_bundle_arguments}
                --location    "${location}"
                --manifest    "${bundle_manifest}"
                --type        module
                --libdir      "${libdir}"
                --source      "${superbuild_install_location}"
                "${module_so}"
        RESULT_VARIABLE res
        ERROR_VARIABLE  err)
      if (res)
        message(FATAL_ERROR "Failed to install Python module ${module}:\n${err}")
      endif ()
    endif ()

    if (IS_DIRECTORY "${search_path}/${module}")
      file(GLOB contents RELATIVE "${search_path}/${module}" "${search_path}/${module}/*")
      foreach (item_name IN LISTS contents)
        set(item_path "${search_path}/${module}/${item_name}")
        if (IS_DIRECTORY "${item_path}")
          if (NOT item_name STREQUAL "__pycache__")
            superbuild_unix_install_python_module("${destination}"
              "${item_name}" "${search_path}/${module}" "${location}/${module}")
          endif()
        else()
          # not a directory, check if it's a Python module or arbitrary artifact
          # file.
          if (item_name MATCHES "\\.(py|so)$")
            # it's a Python module; install it by a recursive call.
            string(REGEX REPLACE "\\.(py|so)$" "" item_name "${item_name}")
            superbuild_unix_install_python_module("${destination}"
              "${item_name}" "${search_path}/${module}" "${location}/${module}")
          else()
            # an artifact file; install it.
            file(INSTALL
              FILES       "${search_path}/${module}/${item_name}"
              DESTINATION "${destination}/${location}/${module}")
          endif()
        endif ()
      endforeach()
    endif ()
  endforeach ()
endfunction ()
