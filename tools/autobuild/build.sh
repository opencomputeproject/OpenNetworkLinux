#!/bin/bash
############################################################
set -e

AUTOBUILD_SCRIPT="$(realpath ${BASH_SOURCE[0]})"
ONL="$(realpath $(dirname $AUTOBUILD_SCRIPT)/../../)"


# Default build branch
BUILD_BRANCH=master

while getopts ":b:v78" opt; do
    case $opt in
        7)
            ONLB_OPTIONS=--7
            if [ -z "$DOCKER_IMAGE" ]; then
                echo "Selecting Debian 7 build..."
            fi
            ;;
        8)
            ONLB_OPTIONS=--8
            if [ -z "$DOCKER_IMAGE" ]; then
                echo "Selecting Debian 8 build..."
            fi
            ;;
        b)
            BUILD_BRANCH=$OPTARG
            ;;
        v)
            set -x
            ;;
    esac
done

if [ -z "$ONLB_OPTIONS" ]; then
    # Build both suites
    $AUTOBUILD_SCRIPT --7 $@
    $AUTOBUILD_SCRIPT --8 $@
    exit $?
fi




#
# Restart under correct builder environment.
#
if [ -z "$DOCKER_IMAGE" ]; then
    # Execute ourselves under the builder
    ONLB=/usr/local/bin/onlbuilder
    if [ -x $ONLB ]; then
        $ONLB $ONLB_OPTIONS --isolate $ONL --non-interactive -c $AUTOBUILD_SCRIPT $@
        exit $?
    else
        echo "Not running in a docker workspace and the onlbuilder script is not available."
        exit 1
    fi
fi

echo "Now running under $DOCKER_IMAGE..."


# The expectation is that we will already be on the required branch.
# This is to normalize environments where the checkout might instead
# be in a detached head (like jenkins)
echo "Switching to branch $BUILD_BRANCH..."
cd $ONL && git checkout $BUILD_BRANCH


#
# Full build
#
cd $ONL
. setup.env

if ! make all; then
    echo Build Failed.
    exit 1
fi

echo Build Succeeded.
