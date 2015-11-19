#!/bin/bash
set -e

if [ "${UID}" != 0 ]; then
    exec sudo $0 "$@"
fi

if [ -z "$1" ] || [ -z "$2" ]; then
    echo "usage: $0 src-dir dst-cpio-gz-file"
    exit 1
fi

SRCDIR=`readlink -f $1`
DSTCPIOGZ=`readlink -f $2`

if [ ! -d "$SRCDIR" ]; then
    echo "src-dir does not exist or is not a directory."
    exit 1
fi

if [ -e "$DSTCPIOGZ" ]; then
    echo "Removing existing $DSTCPIOGZ"
fi

cd "$SRCDIR" && find . | cpio -H newc -o | gzip -f > "$DSTCPIOGZ"
