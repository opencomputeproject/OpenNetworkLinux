#!/bin/sh
############################################################
set -e

AUTOBUILD_SCRIPT="$(readlink -e "$0")" || exit
PARENT_DIR="${AUTOBUILD_SCRIPT%/*}"

cd "$PARENT_DIR"

./build.sh "$@"
./install.sh "$@"
