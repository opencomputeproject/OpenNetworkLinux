#!/bin/bash
############################################################
#
# Make sure the local module manifest.mk is generated.
#
############################################################
ONL=$1
BUILDER=$2
QUIET=$3

if [ -z "$ONL" ] || [ -z "$BUILDER" ]; then
    echo "usage: $0 onl-dir builder-dir"
    exit 1
fi

if [ ! -f "$ONL/.manifest.mk" ]; then
    cd "$ONL" && "$BUILDER/tools/manifesttool.py" make
    mv "$ONL/Manifest.mk" "$ONL/.manifest.mk"
fi

if [ -z "$QUIET" ]; then
    echo "$ONL/.manifest.mk"
fi



