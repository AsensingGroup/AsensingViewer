# Please use https links whenever possible because some people
# cannot clone using ssh (git://) due to a firewalled network.
# This maintains the links for all sources used by this superbuild.
# Simply update this file to change the revision.
# One can use different revision on different platforms.
# e.g.
# if (UNIX)
#   ..
# else (APPLE)
#   ..
# endif()

if (APPLE)
  # Override Python3 Version to fix following bug:
  # https://github.com/pyenv/pyenv/issues/2143
  superbuild_set_revision(python3
    URL     "https://www.python.org/ftp/python/3.9.11/Python-3.9.11.tar.xz"
    URL_MD5 "3c8dde3ebd6da005e969b83b5f0c1886"
    FORCE
  )
endif()

superbuild_set_revision(pythonqt
  GIT_REPOSITORY https://gitlab.kitware.com/LidarView/pythonqt.git
  GIT_TAG "lidarview")

superbuild_set_selectable_source(paraview
  SELECT 5.9.1 DEFAULT
    GIT_REPOSITORY "https://gitlab.kitware.com/LidarView/paraview.git"
    GIT_TAG        "origin/5.9.1CustomForLidarView")

if (WIN32)
  superbuild_set_revision(pcap
    GIT_REPOSITORY http://github.com/patmarion/winpcap.git 
    GIT_TAG master) #At Most 1.5.3 ? #master will likely not change
else()
  superbuild_set_revision(pcap
    GIT_REPOSITORY "https://github.com/the-tcpdump-group/libpcap.git"
    GIT_TAG "libpcap-1.10")
endif()

superbuild_set_revision(eigen
  GIT_REPOSITORY https://gitlab.com/libeigen/eigen.git
  GIT_TAG 3.3.9)

#superbuild_set_revision(liblas
#  URL     "http://www.paraview.org/files/dependencies/libLAS-1.8.1.tar.bz2"
#  URL_MD5 2e6a975dafdf57f59a385ccb87eb5919) # Straight from Paraview-sb versions.txt, WIP check if LASReader PV5.9 can be used insted of ours
  
superbuild_set_revision(liblas
  GIT_REPOSITORY https://gitlab.kitware.com/LidarView/libLAS.git
  GIT_TAG lidarview)
  
superbuild_set_revision(ceres
  GIT_REPOSITORY https://github.com/ceres-solver/ceres-solver
  GIT_TAG 2.1.0rc1)

superbuild_set_revision(glog
  GIT_REPOSITORY https://github.com/google/glog.git
  GIT_TAG v0.5.0)

superbuild_set_revision(pcl
  GIT_REPOSITORY https://github.com/PointCloudLibrary/pcl.git
  GIT_TAG pcl-1.10.1) # 1.11.1 Upgrade tested but has LidarSlam custom LidarPoint undefined symbols

superbuild_set_revision(qhull
  GIT_REPOSITORY https://github.com/qhull/qhull.git
  GIT_TAG master) # Should use projectv available in common-superbuild

superbuild_set_revision(flann
  GIT_REPOSITORY https://gitlab.kitware.com/gabriel.devillers/flann.git
  GIT_TAG 1.9.1-patched) # Could use most updated https://github.com/tkircher/flann 1.9.2

superbuild_set_revision(opencv
  GIT_REPOSITORY https://github.com/opencv/opencv.git
  GIT_TAG 4.5.2)

superbuild_set_revision(nanoflann
  GIT_REPOSITORY https://github.com/jlblancoc/nanoflann.git
  GIT_TAG v1.3.2)

superbuild_set_revision(yaml
  GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
  GIT_TAG yaml-cpp-0.6.3)

superbuild_set_revision(darknet
  GIT_REPOSITORY https://github.com/pjreddie/darknet.git
  GIT_TAG master)

superbuild_set_revision(g2o
  GIT_REPOSITORY https://github.com/RainerKuemmerle/g2o.git
  GIT_TAG master)
