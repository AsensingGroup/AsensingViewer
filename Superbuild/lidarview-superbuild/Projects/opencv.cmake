superbuild_add_project(opencv
  DEPENDS eigen
  
  CMAKE_ARGS
  -DCMAKE_CXX_STANDARD=11
  -DCMAKE_CXX_STANDARD_REQUIRED=ON
  -DEIGEN_INCLUDE_PATH=<INSTALL_DIR>/include/eigen3
  -DWITH_FFMPEG=ON
  -DWITH_TBB=ON
  -DWITH_GTK=ON
  -DWITH_V4L=ON
  -DWITH_OPENGL=ON
  -DWITH_CUBLAS=ON
  -DCUDA_NVCC_FLAGS=-D_FORCE_INLINES
  -DWITH_LIBV4L=ON
  -DBUILD_TESTING=OFF
  -DBUILD_EXAMPLES=OFF
  -DGFLAGS=OFF
  )

# windows opencv install architecture doesn't match the one used here
if (WIN32)
  superbuild_project_add_step(opencv-copylibs
  COMMAND   "${CMAKE_COMMAND}"
            -Dinstall_location:PATH=<INSTALL_DIR>
            -P "${CMAKE_CURRENT_LIST_DIR}/scripts/opencv.copylibs.cmake"
  DEPENDEES install
  COMMENT   "Copy .dll files to the bin/ directory"
  WORKING_DIRECTORY <SOURCE_DIR>)
endif ()
