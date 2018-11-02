#!/bin/sh
############################################################

# Default build branch
BUILD_BRANCH='master'

while getopts ':b:s:d:u:p:l:a:vVc789r:' opt; do
    case "$opt" in
        7)
            ONLB_OPTIONS='--7'
            if [ -z "$DOCKER_IMAGE" ]; then
                echo 'Selecting Debian 7 build...'
            fi
            ;;
        8)
            ONLB_OPTIONS='--8'
            if [ -z "$DOCKER_IMAGE" ]; then
                echo 'Selecting Debian 8 build...'
            fi
            ;;
        9)
            ONLB_OPTIONS='--9'
            if [ -z "$DOCKER_IMAGE" ]; then
                echo 'Selecting Debian 9 build...'
            fi
            ;;
        c)
            BUILD_CLOSED=1
            ;;
        b)
            BUILD_BRANCH="$OPTARG"
            ;;
        a)
            ARCH="$OPTARG"
            ;;
        l)
            PLATFORM_LIST="$OPTARG"
            ;;
        v)
            BUILD_VERBOSE=1
            ;;
        V)
            export VERBOSE=1
            ;;
        r)
            export BUILDROOTMIRROR="$OPTARG"
            ;;
        *)
            ;;
    esac
done

set -e ${BUILD_VERBOSE:+-x}

AUTOBUILD_SCRIPT="$(readlink -e "$0")" || exit
ONL="${AUTOBUILD_SCRIPT%/*/*/*}"

#
# Restart with correct build options
#
if [ -z "$ONLB_OPTIONS" ]; then
    # Build both 8 and 9
    "$AUTOBUILD_SCRIPT" --8 "$@"
    "$AUTOBUILD_SCRIPT" --9 "$@"
    exit $?
fi

#
# Restart under correct builder environment.
#
if [ -z "$DOCKER_IMAGE" ]; then
    # Execute ourselves under the builder
    ONLB="$ONL/docker/tools/onlbuilder"
    if [ -x "$ONLB" ]; then
        "$ONLB" $ONLB_OPTIONS --volumes "$ONL" --non-interactive -c "$AUTOBUILD_SCRIPT" "$@"
        exit $?
    else
        echo 'Not running in a docker workspace and the onlbuilder script is not available.'
        exit 1
    fi
fi

echo "Now running under $DOCKER_IMAGE..."

cd "$ONL"

# The expectation is that we will already be on the required branch.
# This is to normalize environments where the checkout might instead
# be in a detached head (like jenkins)
echo "Switching to branch $BUILD_BRANCH..."
git checkout "$BUILD_BRANCH"

# Fetch closed platforms submodules when requested
[ -z "$BUILD_CLOSED" ] ||
    git submodule update --init --recursive packages/platforms-closed

#
# Full build
#
. ./setup.env

apt-cacher-ng >/dev/null 2>&1 ||:

if [ -n "$PLATFORM_LIST" ]; then
    export PLATFORM_LIST
    export PLATFORMS="$(IFS=','; echo ${PLATFORM_LIST})"
fi

if ! make "${ARCH:-all}"; then
    echo 'Build Failed.'
    exit 1
fi

#
# Cleanup
#
make -C REPO build-clean

# Remove all installer/rootfs/swi packages from the repo.
# These do not need to be kept and take significant
# amounts of time to transfer.
find REPO \( -name '*-installer_0.*' -o -name '*-rootfs_0.*' -o -name '*-swi_0.*' \) -a -delete

echo 'Build Succeeded.'
