#!/bin/bash
############################################################
set -e

PARENT_DIR="$(realpath $(dirname $BASH_SOURCE[0]))"
cd $PARENT_DIR

./build.sh $@
./install.sh $@
