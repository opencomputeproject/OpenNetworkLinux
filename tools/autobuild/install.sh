#!/bin/bash
############################################################
set -e

ONL="$(realpath $(dirname $BASH_SOURCE[0])/../../)"

BUILD_BRANCH=${BUILD_BRANCH:-$(git rev-parse --abbrev-ref HEAD)}

#
# Release artifacts can be automatically installed on a remote server
# after the build is completed.
#

# Remote server name
REMOTE_SERVER=

# Path on the remote server for builds.
# The final build artifacts will reside in $INSTALL_BASE_DIR/$BUILD_BRANCH/$BUILD_ID
REMOTE_BASE_DIR=

# The remote user for RSYNC
REMOTE_USER=

# The remote password for RSYNC
REMOTE_PASS=

while getopts ":s:d:u:p:b:t:v" opt; do
    case $opt in
        s)
            REMOTE_SERVER=$OPTARG
            ;;
        d)
            REMOTE_BASE_DIR=$OPTARG
            ;;
        u)
            REMOTE_USER=$OPTARG
            ;;
        p)
            REMOTE_PASS=$OPTARG
            ;;
        b)
            BUILD_BRANCH=$OPTARG
            ;;
        v)
            set -x
    esac
done


if [ -z "$REMOTE_SERVER" ]; then
    echo "Remote instlallation requires a server (-s)"
    exit 1
fi

if [ -z "$REMOTE_BASE_DIR" ]; then
    echo "Remote installation requires a branch directory (-d)"
    exit 1
fi

if [ -z "$REMOTE_USER" ]; then
    echo "Remote installation requires a remote user (-u)"
    exit 1
fi

if [ -z "$REMOTE_PASS" ]; then
    echo "Remote installation requires a remote password (-p)"
    exit 1
fi


. $ONL/make/version-onl.sh
REMOTE_DIR="$REMOTE_BASE_DIR/$BUILD_BRANCH/$FNAME_BUILD_ID"


RSYNC=rsync
RSYNC_OPTS=" -v --copy-links --delete -a"

_rsync() {
    $RSYNC $RSYNC_OPTS --rsh="sshpass -p $REMOTE_PASS ssh -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no -l $REMOTE_USER" $1 $2
}

sshpass -p $REMOTE_PASS ssh -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no -l $REMOTE_USER $REMOTE_SERVER mkdir -p $REMOTE_DIR
_rsync $ONL/RELEASE $REMOTE_SERVER:$REMOTE_DIR

_rsync $ONL/REPO $REMOTE_SERVER:$REMOTE_DIR
