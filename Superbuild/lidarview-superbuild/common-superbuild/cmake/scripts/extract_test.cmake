# A wrapper around a process call which checks the command's result.
function (_extract_process input)
  execute_process(${ARGN}
    RESULT_VARIABLE res
    OUTPUT_VARIABLE out
    ERROR_VARIABLE  out)
  if (res)
    message(FATAL_ERROR "Failed to extract ${input}:\n${out}")
  endif ()
endfunction ()

# Extract a package with CMake.
function (_extract_with_cmake output input)
  _extract_process("${input}"
    COMMAND "${CMAKE_COMMAND}" -E tar
            xf "${input}"
    WORKING_DIRECTORY "${output}")
endfunction ()

# Detect a tarbomb package.
function (_detect_tarbomb var dir)
  file(GLOB contents "${dir}/*")

  if (NOT IS_DIRECTORY "${contents}")
    set(contents "${dir}")
  endif ()

  set("${var}"
    "${contents}"
    PARENT_SCOPE)
endfunction ()

# Do the dance to mount and extract a .app from a .dmg file.
function (_extract_dmg output mount input)
  _extract_process("${input}"
    COMMAND /bin/sh -c
            "yes | hdiutil attach -mountpoint '${mount}' '${input}'")
  file(GLOB apps "${mount}/*.app")
  foreach (app IN LISTS apps)
    get_filename_component(app_name "${app}" NAME)
    _extract_process("${app}"
      COMMAND "${CMAKE_COMMAND}" -E copy_directory
              "${app}"
              "${output}/${app_name}/")
  endforeach ()
  _extract_process("${input} (detach)"
    COMMAND hdiutil
            detach
            "${mount}")
endfunction ()

# Extract a binary from the given directory composed of the given glob into the
# output directory. For simple composition packages (tarballs, zip, etc.),
# non-tarbomb files have their intermediate directories removed.
function (extract_binary dir glob output)
  file(REMOVE_RECURSE "${output}")
  file(MAKE_DIRECTORY "${output}")

  file(GLOB file "${dir}/${glob}")
  if (NOT file)
    message(FATAL_ERROR "Failed to locate package: ${dir}/${glob}.")
  elseif (NOT EXISTS "${file}")
    message(FATAL_ERROR "Ambiguous glob: ${dir}/${glob}:\n${file}.")
  endif ()

  set(output_dir "${dir}/ctest/extract/__workdir")
  file(MAKE_DIRECTORY "${output_dir}")

  message("Using package: ${file}")
  get_filename_component(file_ext "${file}" EXT)
  if (file_ext MATCHES "(\\.|=)(7z|tar\\.bz2|tar\\.gz|tar\\.xz|tbz2|tgz|txz|zip)$")
    _extract_with_cmake("${output_dir}" "${file}")
    _detect_tarbomb(output_dir "${output_dir}")
  elseif (file_ext MATCHES "\\.dmg$")
    _extract_dmg("${output_dir}" "${dir}/__mount" "${file}")
  endif ()

  get_filename_component(templocation "${dir}/__package__" ABSOLUTE)
  file(RENAME "${output_dir}" "${templocation}")
  file(REMOVE_RECURSE "${output}")
  file(RENAME "${templocation}" "${output}")
  message("Package available under '${output}'")
endfunction ()

extract_binary("${test_dir}" "${binary_glob}" "${output_dir}")
