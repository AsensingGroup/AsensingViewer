#!/bin/sh

# Install build requirements.
dnf install --setopt=install_weak_deps=False -y \
    autoconf automake libtool make

# Install tools used during the build of various projects.
dnf install --setopt=install_weak_deps=False -y \
    file which libxkbcommon-x11-devel mesa-libGL-devel \
    python-unversioned-command perl-FindBin perl-lib \
    bison flex libXext-devel libXrandr-devel

# Install development tools
dnf install --setopt=install_weak_deps=False -y \
    git-core \
    git-lfs

# Install toolchains.
dnf install --setopt=install_weak_deps=False -y \
    gcc-c++ \
    gcc \
    gcc-gfortran

dnf clean all
