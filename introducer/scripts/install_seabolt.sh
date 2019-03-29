#!/bin/sh

export DYLD_LIBRARY_PATH=$(pwd)/iternal/libraries/seabolt-1.7/build/dist/lib64
export PKG_CONFIG_PATH=$(pwd)/iternal/libraries/seabolt-1.7/build/dist/share/pkgconfig
export LD_LIBRARY_PATH=$(pwd)/iternal/libraries/seabolt-1.7/build/dist/lib64
cd $(pwd)/iternal/libraries/seabolt-1.7/
cmake .
./make_release.sh
echo $DYLD_LIBRARY_PATH
echo $PKG_CONFIG_PATH
echo $LD_LIBRARY_PATH