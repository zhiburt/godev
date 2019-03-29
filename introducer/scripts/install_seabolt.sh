#!/bin/sh

export DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH:../iternal/libraries/seabolt-1.7/build/dist/lib64
export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:../iternal/libraries/seabolt-1.7/build/dist/share/pkgconfig
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:../iternal/libraries/seabolt-1.7/build/dist/lib64