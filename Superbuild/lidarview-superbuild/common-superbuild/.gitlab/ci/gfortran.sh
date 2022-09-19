#!/bin/sh

set -e

readonly version="10.2.0"
readonly sha256sum="ab341c437d9ff145d28ae3e4419c5d4949eff7c77858fa6127d7f2152ae8efcd"
readonly platform="macos10.13"

readonly filename="gcc-$version"
readonly tarball="$filename-$platform.tar.xz"

cd .gitlab

echo "$sha256sum  $tarball" > gfortran.sha256sum
curl -OL "https://www.paraview.org/files/dependencies/$tarball"
shasum -a 256 --check gfortran.sha256sum
mkdir gfortran
tar --strip-components=3 -C gfortran -xf "$tarball"
