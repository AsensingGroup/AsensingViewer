cmake_minimum_required(VERSION 3.19)

message("Avoid CTest truncation: CTEST_FULL_OUTPUT")

find_program(FILE_COMMAND NAMES file)

function (check_binary path)
  if (NOT deployment_target)
    return ()
  endif ()

  execute_process(
    COMMAND otool -l "${path}"
    OUTPUT_VARIABLE out
    ERROR_VARIABLE  err
    RESULT_VARIABLE res
    OUTPUT_STRIP_TRAILING_WHITESPACE)

  if (res)
    message(SEND_ERROR
      "Failed to run `otool -l` on ${path}: ${err}")
  endif ()

  string(REPLACE "\n" ";" out "${out}")
  list(FILTER out INCLUDE REGEX "(LC_VERSION_MIN_MACOSX|version|LC_BUILD_VERSION|minos)")

  if (NOT out)
    message(WARNING
      "No deployment target for ${path}")
    return ()
  endif ()

  set(ok 0)
  set(found_mode "")
  set(found_lines)
  foreach (item IN LISTS out)
    string(STRIP "${item}" item)

    if (item MATCHES "LC_VERSION_MIN_MACOSX") # x86_64
      set(found_mode "LC_VERSION_MIN_MACOSX")
    elseif (item MATCHES "LC_BUILD_VERSION") # arm64
      set(found_mode "LC_BUILD_VERSION")
    elseif (found_mode STREQUAL "LC_VERSION_MIN_MACOSX" AND
            item MATCHES "version ${deployment_target}")
      set(ok 1)
      set(found_mode "")
    elseif (found_mode STREQUAL "LC_BUILD_VERSION" AND
            item MATCHES "minos ${deployment_target}")
      set(ok 1)
      set(found_mode "")

    # XXX(scipy): TBB is pre-built. 10.11 binaries are OK here.
    elseif (path MATCHES "libtbb")
      if (found_mode STREQUAL "LC_VERSION_MIN_MACOSX" AND
          item MATCHES "version 10.11")
        set(ok 1)
        set(found_mode "")
      elseif (found_mode STREQUAL "LC_BUILD_VERSION" AND
              item MATCHES "minos 10.11")
        set(ok 1)
        set(found_mode "")
      endif ()

    # XXX(scipy): SciPy is special for some reason. 10.9 binaries are OK here.
    elseif (path MATCHES "/scipy/")
      if (found_mode STREQUAL "LC_VERSION_MIN_MACOSX" AND
          item MATCHES "version 10.9")
        set(ok 1)
        set(found_mode "")
      elseif (found_mode STREQUAL "LC_BUILD_VERSION" AND
              item MATCHES "minos 10.9")
        set(ok 1)
        set(found_mode "")
      endif ()

    # XXX(cryptography): cryptography uses wheels, so allow its target version.
    elseif (path MATCHES "/cryptography/")
      if (found_mode STREQUAL "LC_VERSION_MIN_MACOSX" AND
          item MATCHES "version 10.10")
        set(ok 1)
        set(found_mode "")
      elseif (found_mode STREQUAL "LC_BUILD_VERSION" AND
              item MATCHES "minos 10.10")
        set(ok 1)
        set(found_mode "")
      endif ()

    elseif (found_mode)
      list(APPEND found_lines "${item}")
    endif ()
  endforeach ()

  if (NOT ok)
    message(SEND_ERROR
      "Invalid deployment target for ${path}: ${found_lines}")
  endif ()
endfunction ()

file(GLOB binaries "${install_dir}/bin/*")
foreach (binary IN LISTS binaries)
  execute_process(
    COMMAND "${FILE_COMMAND}" -h "${binary}"
    OUTPUT_VARIABLE out
    ERROR_VARIABLE  err
    RESULT_VARIABLE res
    OUTPUT_STRIP_TRAILING_WHITESPACE)

  if (res)
    message(SEND_ERROR
      "Failed to run `file` on ${binary}: ${err}")
  endif ()

  # Skip non-Mach-O files.
  if (NOT out MATCHES "Mach-O")
    continue ()
  endif ()

  if (NOT out MATCHES "executable ${arch}")
    message(SEND_ERROR
      "Invalid architecture for ${binary}: ${out}")
  endif ()

  check_binary("${binary}")
endforeach ()

file(GLOB_RECURSE libraries
  "${install_dir}/lib/*.dylib"
  "${install_dir}/lib/*.so")
foreach (library IN LISTS libraries)
  execute_process(
    COMMAND "${FILE_COMMAND}" -h "${library}"
    OUTPUT_VARIABLE out
    ERROR_VARIABLE  err
    RESULT_VARIABLE res
    OUTPUT_STRIP_TRAILING_WHITESPACE)

  if (res)
    message(SEND_ERROR
      "Failed to run `file` on ${library}: ${err}")
  endif ()

  # Skip non-Mach-O files.
  if (NOT out MATCHES "Mach-O")
    continue ()
  endif ()

  if (NOT out MATCHES "(library|bundle) ${arch}")
    message(SEND_ERROR
      "Invalid architecture for ${library}: ${out}")
  endif ()

  check_binary("${library}")
endforeach ()
