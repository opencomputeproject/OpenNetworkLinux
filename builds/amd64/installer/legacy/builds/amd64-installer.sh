#!/bin/sh
############################################################
# <bsn.cl fy=2013 v=none>
#
#        Copyright 2013, 2014 BigSwitch Networks, Inc.
#
#
#
# </bsn.cl>
############################################################
#
# Open Network Linux Installation Script for AMD64.
#
# The purpose of this script is to automatically install Open Network Linux
# on the target system.
#
# This script is ONIE-compatible.
#
# This script is can be run under a manual boot of the Open Network Linux
# Loader as the execution environment for platforms that do not
# support ONIE.
#
############################################################

IARCH="x86_64"
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


############################################################
#
# Installation utility functions
#
############################################################

CR="
"

PATH=$PATH:/sbin:/usr/sbin
DEV=
START_MB=

visit_parted()
{
  local dev diskfn partfn rest
  dev=$1; shift
  diskfn=$1; shift
  partfn=$1; shift
  rest="$@"

  local ifs ifs2 dummy
  ifs=$IFS; IFS=$CR
  for line in `parted -m $dev unit mb print`; do
    IFS=$ifs

    line=`echo "$line" | sed -e 's/[;]$//'`

    case "$line" in
      /dev/*)
        ifs2=$IFS; IFS=:
        set dummy $line
        IFS=$ifs2

        local dev sz model lbsz pbsz typ modelname flags
        shift
        dev=$1; shift
        sz=$1; shift
        model=$1; shift
        lbsz=$1; shift
        pbsz=$1; shift
        typ=$1; shift
        modelname=$1; shift
        flags=$1; shift

        eval $diskfn "$dev" "$sz" "$model" "$typ" "$flags" $rest || return 1

        ;;
      [0-9]:*)
        ifs2=$IFS; IFS=:
        set dummy $line
        IFS=$ifs2

        local part start end sz fs label flags
        shift
        part=$1; shift
        start=$1; shift
        end=$1; shift
        sz=$1; shift
        fs=$1; shift
        label=$1; shift
        flags=$label

        eval $partfn "$part" "$start" "$end" "$sz" "$fs" "$label" "$flags" $rest || return 1

        ;;

      *) continue ;;
    esac

  done
  IFS=$ifs
}

do_handle_disk()
{
  local dev sz model typ flags
  dev=$1; shift
  sz=$1; shift
  model=$1; shift
  typ=$1; shift
  flags=$1; shift

  if test "$typ" != "gpt"; then
    installer_say "*** invalid partition table: $typ"
    return 1
  fi

  return 0
}

do_handle_partitions()
{
  local part start end sz fs label flags
  part=$1; shift
  start=$1; shift
  end=$1; shift
  sz=$1; shift
  fs=$1; shift
  label=$1; shift
  flags=$1; shift

  installer_say "Examining $DEV part $part"


  case "$label" in
      ONIE-BOOT|GRUB-BOOT|*-DIAG)
          installer_say "Partition $DEV$part: $label: Preserving..."
          ;;
      *)
          installer_say "Partition $DEV$part: $label: Deleting..."
          parted $DEV rm $part || return 1
          ;;
  esac

  return 0
}

get_free_space()
{
    local free start
    free=`parted -s $DEV unit mb p free | grep Free | tail -1`
    echo $free | awk '{print $1}' | tr "MB" " "
}

get_part_number()
{
    local dev name part
    dev=$1; shift
    name=$1; shift
    part=$(parted -s $DEV unit mb p all | grep $name | awk {'print $1'})
    if [ -z "$part" ]; then
        installer_say "Failed to discover the partition number for $name"
        return 1
    else
        echo $part
        return 0
    fi

}

partition_gpt()
{
  local start end part

  installer_say "Creating 128MB for Open Network Linux boot"
  start=$1; shift
  end=$(( $start + 128 ))
  echo "start=$start end=$end"

  parted -s $DEV unit mb mkpart "ONL-BOOT" ext4 ${start} ${end} || return 1
  if ! part=$(get_part_number $DEV "ONL-BOOT"); then
      return 1
  fi

  parted -s $DEV set $part boot on || return 1
  mkfs.ext4 -L "ONL-BOOT" ${DEV}${part}
  start=$(( $end + 1 ))

  installer_say "Creating /mnt/flash"
  end=$(( $start + 128 ))

  parted -s $DEV unit mb mkpart "FLASH" fat32 ${start} ${end} || return 1
  if ! part=$(get_part_number $DEV "FLASH"); then
      return 1
  fi
  mkfs.vfat -n "FLASH" ${DEV}${part}

  start=$(( $end + 1 ))

  installer_say "Allocating remainder for /mnt/flash2"
  parted -s $DEV unit mb mkpart "FLASH2" fat32 ${start} "100%" || return 1
  if ! part=$(get_part_number $DEV "FLASH2"); then
      return 1
  fi
  mkfs.vfat -n "FLASH2" ${DEV}${part}

  return 0
}


installer_standard_gpt_install()
{
  DEV=$1; shift

  visit_parted $DEV do_handle_disk do_handle_partitions || return 1
  partition_gpt $(get_free_space) || return 1

  installer_say "Installing boot files..."
  mkdir "$workdir/mnt"

  if [ -f "${installer_dir}/boot-config" ]; then
      installer_say "Installing boot-config"
      mount LABEL="FLASH" "$workdir/mnt"
      cp "${installer_dir}/boot-config" "$workdir/mnt/boot-config"
      umount "$workdir/mnt"
  fi

  SWISRC=`ls ${installer_dir}/*.swi`

  if test -f "${SWISRC}"; then
    if test ! "${SWIDST}"; then
      SWIDST="$(basename ${SWISRC})"
    fi
    installer_say "Installing Open Network Linux Software Image (${SWIDST})..."
    mount LABEL="FLASH2" "$workdir/mnt"
    cp "${SWISRC}" "$workdir/mnt/${SWIDST}"
    umount "$workdir/mnt"
  fi

  installer_say "Installing kernels"
  mount LABEL=ONL-BOOT -t ext4 "$workdir/mnt"
  echo ${installer_dir}
  cp ${installer_dir}/kernel-* "$workdir/mnt/"
  cp "${installer_dir}/initrd-amd64" "$workdir/mnt/."
  mkdir "$workdir/mnt/grub"
  cp "${installer_platform_dir}/onl/boot/grub.cfg" "$workdir/mnt/grub/grub.cfg"

  installer_say "Installing GRUB"
  grub-install --boot-directory="$workdir/mnt" $DEV

  # leave the GRUB directory mounted,
  # so we can manipulate the GRUB environment
  BOOTDIR="$workdir/mnt"

  return 0
}

set -e
cd $(dirname $0)

installer_script=${0##*/}
installer_zip=$1

BOOTDIR=/mnt/onie-boot
# initial boot partition (onie)

# Pickup ONIE defines for this machine.
if test -r /etc/machine.conf; then . /etc/machine.conf; fi

#
# Installation environment setup.
#
if test "${onie_platform}"; then
  :
else
  echo "Missing onie_platform (invalid /etc/machine.conf)" 1>&2
  exit 1
fi

# Running under ONIE, most likely in the background in installer mode.
# Our messages have to be sent to the console directly, not to stdout.
installer_say()
{
  echo "$@" > /dev/console
}

workdir=$(mktemp -d -t install-XXXXXX)

# Installation failure message.
do_cleanup()
{
  installer_say "Install failed."
  cat /var/log/onie.log > /dev/console
  installer_say "Install failed. See log messages above for details"

  grep "$workdir" /proc/mounts | cut -d' ' -f2 | sort -r | xargs -r -n 1 umount
  cd /tmp
  rm -fr "$workdir"

  sleep 3
  #reboot
}

trap "do_cleanup" 0 1

if test -z "${installer_platform}"; then
  # Our platform identifiers are equal to the ONIE platform identifiers without underscores:
  installer_platform=`echo ${onie_platform} | tr "_" "-"`
  installer_arch=${onie_arch}
fi
installer_say "Open Network Linux installer running under ONIE."

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

# Look for the platform installer directory.
installer_platform_dir="${installer_dir}/lib/platform-config/${installer_platform}"
if test -d "${installer_platform_dir}"; then
  # Source the installer scriptlet
  . "${installer_platform_dir}/onl/install/${installer_platform}.sh"
else
  installer_say "This installer does not support the ${installer_platform} platform."
  installer_say "Available platforms are:"
  list=`ls "${installer_dir}/lib/platform-config"`
  installer_say "${list}"
  installer_say "Installation cannot continue."
  exit 1
fi

# The platform script must provide this function. This performs the actual install for the platform.
platform_installer

trap - 0 1
installer_say "Install finished.  Rebooting to Open Network Linux."
sleep 3
#reboot

exit

# Do not add any additional whitespace after this point.
PAYLOAD_FOLLOWS
