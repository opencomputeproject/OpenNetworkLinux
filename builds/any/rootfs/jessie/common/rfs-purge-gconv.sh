#!/bin/sh
#
######################################################################
#
# rfs-purge-gconv
#
######################################################################

m=$(uname -m)
gconvdir=/usr/lib/${m}-linux-gnu/gconv

cp /dev/null ${gconvdir}/gconv-modules
iconvconfig --nostdlib -o ${gconvdir}/gconv-modules.cache ${gconvdir}
rm ${gconvdir}/*.so
