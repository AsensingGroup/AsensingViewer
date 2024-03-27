set(meson_path "${install_dir}/bin/meson")

# Move the existing `meson` to a safe place (if it is not our script).
file(READ "${meson_path}" meson_content LIMIT 10)
if (NOT meson_content STREQUAL "#!/bin/sh\n")
  file(RENAME
    "${meson_path}"
    "${install_dir}/bin/meson-real")
endif ()

# Write a shell script to replace the real thing.
file(WRITE
  "${meson_path}"
  "#!/bin/sh

exec \"${superbuild_python_executable}\" \"${install_dir}/bin/meson-real\" \"\$@\"
")
# Make executable.
file(CHMOD "${meson_path}"
  PERMISSIONS
    OWNER_READ OWNER_WRITE OWNER_EXECUTE
    GROUP_READ GROUP_WRITE GROUP_EXECUTE
    WORLD_READ             WORLD_EXECUTE)
