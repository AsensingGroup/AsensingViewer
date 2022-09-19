#!/bin/bash

if [ $# != 1 ]
then
    echo "Usage: $0  <executable file> [package name]"
    exit 0
fi

VERSION="1.0.0"
PACKAGE_NAME=AsensingViewer-linux-x86_64
INSTALL_DIR=$PACKAGE_NAME
LIB_DIR=$PWD/lib

if [ ! -n $3 ]
then
    INSTALL_DIR = $3
fi

echo "Package name: $PACKAGE_NAME"
echo "Install path: $PWD/$INSTALL_DIR"

mkdir $INSTALL_DIR
mkdir $LIB_DIR

Target=$1

echo "Collect libraries ..."
lib_array=($(ldd $Target | grep -o "/.*" | grep -o "/.*/[^[:space:]]*"))

for Variable in ${lib_array[@]}
do
    cp "$Variable" $LIB_DIR
done

mv $LIB_DIR $INSTALL_DIR
cp $Target $INSTALL_DIR

echo "Packing $PACKAGE_NAME.tar.gz ..."
tar zcf $PACKAGE_NAME.tar.gz $INSTALL_DIR
echo "Clean ..."
rm -r $INSTALL_DIR
echo "Done."