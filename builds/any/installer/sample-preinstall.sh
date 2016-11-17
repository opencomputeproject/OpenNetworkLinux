#!/bin/sh
#
######################################################################
#
# sample-preinstall.sh
#
# Example script for pre-install hooks.
#
# Add this as a preinstall hook to your installer via
# the 'mkinstaller.py' command line:
#
# $ mkinstaller.py ... --preinstall-script sample-preinstall.sh ...
#
# At install time, this script will
#
# 1. be extracted into the working directory with the other installer
#    collateral
# 2. have the execute bit set
# 3. run in-place with the installer chroot directory passed
#    as the first command line parameter
#
# If the script fails (returns a non-zero exit code) then
# the install is aborted.
#
# This script is executed using the ONIE runtime (outside the chroot),
# before the actual installer (chrooted Python script)
#
# At the time the script is run, the installer environment (chroot)
# has been fully prepared, including filesystem mount-points.
#
######################################################################

rootdir=$1; shift

echo "Hello from preinstall"
echo "Chroot is $rootdir"

exit 0
