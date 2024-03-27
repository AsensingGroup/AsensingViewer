include("${CMAKE_CURRENT_LIST_DIR}/../netcdf.cmake")

# Add HDF5 version extraction
# https://patch-diff.githubusercontent.com/raw/Unidata/netcdf-c/pull/2076
superbuild_apply_patch(netcdf hdf5-version
  "Finding HDF5 without a `Find` module was missing version extraction")

# Fix linking to a shared hdf5 on Windows
superbuild_apply_patch(netcdf find-shared-hdf5
  "Link to HDF5 shared on Windows")

# Fix installation rules on Windows
superbuild_apply_patch(netcdf install-fixes
  "Hide globbing install rules")
