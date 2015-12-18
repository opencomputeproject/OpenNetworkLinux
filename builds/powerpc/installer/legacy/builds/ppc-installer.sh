#!/bin/sh

IARCH="ppc"
ARCH=`uname -m`
if [ "$ARCH" != "$IARCH" ]; then
    echo
    echo "------------------------------------"
    echo "Installer Architecture: $IARCH"
    echo "Target Architecture:    $ARCH"
    echo
    echo "This installer cannot be used on this"
    echo "target."
    echo
    echo "------------------------------------"
    sleep 5
    exit 1
fi

set -e
cd $(dirname $0)


installer_script=${0##*/}
installer_zip=$1

if [ -f /etc/machine.conf ]; then
    . /etc/machine.conf

    # Running under ONIE, most likely in the background in installer mode.
    # Our messages have to be sent to the console directly, not to stdout.
    installer_say() {
        echo "$@" > /dev/console
    }
    # Installation failure message.
    trap 'installer_say "Install failed.; cat /var/log/onie.log > /dev/console; installer_say "Install failed. See log messages above for details"; sleep 3; reboot' EXIT

    if [ -z "${installer_platform}" ]; then
        # Our platform identifiers are equal to the ONIE platform identifiers without underscores:
        installer_platform=`echo ${onie_platform} | tr "_" "-"`
        installer_arch=${onie_arch}
    fi
fi

#
# Remount tmpfs larger if possible.
# We will be doing all of our work out of /tmp
#
mount -o remount,size=1024M /tmp || true


# Unpack our distribution
installer_say "Unpacking Open Network Linux installer files..."
installer_dir=`pwd`
if test "$SFX_PAD"; then
  # ha ha, busybox cannot exclude multiple files
  unzip $installer_zip -x $SFX_PAD
elif test "$SFX_UNZIP"; then
  unzip $installer_zip -x $installer_script
else
  dd if=$installer_zip bs=$SFX_BLOCKSIZE skip=$SFX_BLOCKS \
  | unzip - -x $installer_script
fi


if [ -f "${installer_dir}/versions.sh" ]; then
    . "${installer_dir}/versions.sh"
    installer_say "${VERSION_STRING} Installer"
fi

installer_say "Detected platform: ${installer_platform}"

. "${installer_dir}/ppc-install-lib"

# Look for the platform installer directory.
installer_platform_dir="${installer_dir}/lib/platform-config/${installer_platform}"
if [ -d "${installer_platform_dir}" ]; then
    # Source the installer scriptlet
    ONL_PLATFORM=${installer_platform}
    . "${installer_platform_dir}/onl/install/${installer_platform}.sh"
else
    installer_say "This installer does not support the ${installer_platform} platform."
    installer_say "Available platforms are:"
    list=`ls ${installer_dir}/lib/platform-config`
    installer_say "${list}"
    installer_say "Installation cannot continue."
    exit 1
fi

# The platform script must provide this function. This performs the actual install for the platform.
platform_installer

installer_say "Configuring system to boot Open Network Linux..."
envf=/tmp/.env
cp /dev/null "${envf}"
echo "nos_bootcmd ${platform_bootcmd}" >> "${envf}"
fw_setenv_f_s "${envf}"
installer_say "Install finished.  Rebooting to Open Network Linux."
sleep 3
reboot
exit

# Do not add any additional whitespace after this point.
PAYLOAD_FOLLOWS
