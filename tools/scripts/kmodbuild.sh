#!/bin/bash
set -e

#
# kmodbuild.sh kernel-packages module-directories platform-name
#

function build_module
{
    KERNEL=`onlpm --find-dir $1 mbuilds`
    BUILD_DIR=`mktemp -d`
    cp -R $2/* "$BUILD_DIR"
    make -C $KERNEL M=$BUILD_DIR modules
    make -C $KERNEL M=$BUILD_DIR INSTALL_MOD_PATH=`pwd` INSTALL_MOD_DIR="$3" modules_install
}

for kernel in $1; do
    for module in $2; do
        build_module $kernel $module $3
    done
done
