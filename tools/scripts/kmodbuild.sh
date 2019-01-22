#!/bin/bash
set -e

#
# kmodbuild.sh kernel-packages module-directories platform-name
#

#
# build <kernel-package> <module-directory> <install-subdir>
#
function build
{
    if [ -d $1 ]; then
        KERNEL=$1
    else
        KERNEL=`onlpm --find-dir $1 mbuilds`
    fi
    BUILD_DIR=$2
    INSTALL_DIR=$3
    make -C $KERNEL M=$BUILD_DIR modules
    make -C $KERNEL M=$BUILD_DIR INSTALL_MOD_PATH=`pwd` INSTALL_MOD_DIR="$3" modules_install
}

#
# build_directory <kernel-package> <module-directory> <install-subdir>
#
function build_directory
{
    BUILD_DIR=`mktemp -d`
    cp -R $2/* "$BUILD_DIR"
    build $1 $BUILD_DIR $3
}

#
# build_source <kernel-package> <source-file> <install-subdir>
#
function build_source
{
    BUILD_DIR=`mktemp -d`
    cp $2 $BUILD_DIR
    if [ -n "$4" ]; then
        cp $4 $BUILD_DIR
    fi
    src=$(basename $2)
    obj=${src%.c}.o
    echo "obj-m := $obj" >> $BUILD_DIR/Kbuild
    build $1 $BUILD_DIR $3
}

for kernel in $1; do
    for module in $2; do
        if [ -d $module ]; then
            build_directory $kernel $module $3
        else
            build_source $kernel $module $3 $4
        fi
    done
done
