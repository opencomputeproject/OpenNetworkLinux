#!/bin/sh
#
######################################################################
#
# helper functions for install
#
######################################################################

installer_reboot() {
  local dummy sts timeout trapsts
  if test $# -gt 0; then
    timeout=$1; shift
  else
    timeout=3
  fi

  installer_say "Rebooting in ${timeout}s"

  unset dummy trapsts
  # ha ha, 'local' auto-binds the variables

  trap "trapsts=130" 2
  if read -t $timeout -r -p "Hit CR to continue, CTRL-D or CTRL-C to stop... " dummy; then
    sts=0
  else
    sts=$?
  fi
  trap - 2
  test "$trapsts" && sts=$trapsts

  if test ${dummy+set}; then
    if test $sts -eq 0; then
      installer_say "CR, rebooting"
      exit
    else
      installer_say "CTRL-D, stopped"
      exit
    fi
  fi

  # ha ha, busybox does not report SIGALRM
  if test "${trapsts+set}"; then
    :
  else
    installer_say "timeout, rebooting"
    reboot
  fi

  signo=$(( $sts - 128 ))
  if test $signo -eq 14; then
    # SIGALRM, possibly irrelevant for busybox
    installer_say "timeout, rebooting"
    reboot
  fi

  # e.g. SIGQUIT
  installer_say "signal $signo, stopped"
  exit
}

installer_mkchroot() {
  local rootdir
  rootdir=$1

  local hasDevTmpfs
  if grep -q devtmpfs /proc/filesystems; then
    hasDevTmpfs=1
  fi

  # special handling for /dev, which usually already has nested mounts
  installer_say "Setting up /dev"
  rm -fr "${rootdir}/dev"/*
  if test "$hasDevTmpfs"; then
    :
  else
    for dev in /dev/*; do
      if test -d "$dev"; then
        mkdir "${rootdir}${dev}"
      else
        cp -a "$dev" "${rootdir}${dev}"
      fi
    done
    mkdir -p "${rootdir}/dev/pts"
  fi

  installer_say "Setting up /run"
  rm -fr "${rootdir}/run"/*
  mkdir -p "${rootdir}/run"
  d1=$(stat -c "%D" /run)
  for rdir in /run/*; do
    if test -d "$rdir"; then
      mkdir "${rootdir}${rdir}"
      d2=$(stat -c "%D" $rdir)
      t2=$(stat -f -c "%T" $rdir)
      case "$t2" in
        tmpfs|ramfs)
          # skip tmpfs, we'll just inherit the initrd ramfs
        ;;
        *)
          if test "$d1" != "$d2"; then
            mount -o bind $rdir "${rootdir}${rdir}"
          fi
        ;;
      esac
    fi
  done
  
  installer_say "Setting up mounts"
  mount -t proc proc "${rootdir}/proc"
  mount -t sysfs sysfs "${rootdir}/sys"
  if test "$hasDevTmpfs"; then
    mount -t devtmpfs devtmpfs "${rootdir}/dev"
    mkdir -p ${rootdir}/dev/pts
  fi
  mount -t devpts devpts "${rootdir}/dev/pts"
  if test -d "${rootdir}/sys/firmware/efi/efivars"; then
    modprobe efivarfs || :
    mount -t efivarfs efivarfs "${rootdir}/sys/firmware/efi/efivars"
  fi

  if test ${TMPDIR+set}; then
    # make the tempdir available to the chroot
    mkdir -p "${rootdir}${TMPDIR}"
  fi

  # export ONIE defines to the installer, if they exist
  cp /etc/machine*.conf "${rootdir}/etc/."

  # export ONL defines to the installer
  mkdir -p "${rootdir}/etc/onl"
  if test -d /etc/onl; then
    cp -a /etc/onl/. "${rootdir}/etc/onl/."
  fi

  # export firmware config
  if test -r /etc/fw_env.config; then
    cp /etc/fw_env.config "${rootdir}/etc/fw_env.config"
  fi
}

visit_blkid()
{
  local fn rest
  fn=$1; shift
  rest="$@"

  local ifs
  ifs=$IFS; IFS=$CR
  for line in $(blkid); do
    IFS=$ifs

    local dev
    dev=${line%%:*}
    line=${line#*:}

    local TYPE LABEL PARTLABEL UUID PARTUUID
    while test "$line"; do
      local key
      key=${line%%=*}
      line=${line#*=}
      case "$line" in
        '"'*)
          line=${line#\"}
          val=${line%%\"*}
          line=${line#*\"}
          line=${line## }
        ;;
        *)
          val=${line%% *}
          line=${line#* }
        ;;
      esac
      eval "$key=\"$val\""
    done

    local sts
    if eval $fn \"$dev\" \"$LABEL\" \"$UUID\" \"$PARTLABEL\" \"$PARTUUID\" $rest; then
      sts=0
    else
      sts=$?
    fi
    if test $sts -eq 2; then break; fi
    if test $sts -ne 0; then return $sts; fi

  done
  IFS=$ifs

  return 0
}

##############################
#
# Fixup a corrupted GPT partition, within reason
# See SWL-3971
#
##############################

blkid_find_gpt_boot() {
  local dev label
  dev=$1; shift
  label=$1; shift
  rest="$@"

  installer_say "Examining $dev --> $label"

  # EFI partition shows up as a valid partition with blkid
  if test "$label" = "EFI System"; then
    installer_say "Found EFI System partition at $dev"
    ESP_DEVICE=$(echo "$dev" | sed -e 's/[0-9]$//')

    # this is definitely the boot disk
    return 2
  fi

  # sometimes this is hidden from blkid (no filesystem)
  if test "$label" = "GRUB-BOOT"; then
    installer_say "Found GRUB boot partition at $dev"
    GRUB_DEVICE=$(echo "$dev" | sed -e 's/[0-9]$//')

    # probably the boot disk, look for a GPT header
    return 0
  fi

  # shows up in blkid but may not be GPT
  if test "$label" = "ONIE-BOOT"; then
    installer_say "Found ONIE boot partition at $dev"
    ONIE_DEVICE=$(echo "$dev" | sed -e 's/[0-9]$//')

    # probably the boot disk, look for a GPT header
    return 0
  fi

  # not found, skip
  return 0
}

installer_fixup_gpt() {
  local buf dat sts dev do_recover

  buf=$(mktemp -u -t sgdisk-XXXXXX)

  ESP_DEVICE=
  GRUB_DEVICE=
  ONIE_DEVICE=
  visit_blkid blkid_find_gpt_boot

  dev=
  if test -b "$ESP_DEVICE"; then
    dev=$ESP_DEVICE
  elif test -b "$GRUB_DEVICE"; then
    sgdisk -p "$GRUB_DEVICE" > "$buf" 2>&1 || :
    if grep -q GUID "$buf"; then
      dev=$GRUB_DEVICE
    fi
  elif test -b "$ONIE_DEVICE"; then
    sgdisk -p "$ONIE_DEVICE" > "$buf" 2>&1 || :
    if grep -q GUID "$buf"; then
      # here we assume that the ONIE boot partition is on
      # the boot disk
      # (additionally we could also look for 'GRUB-BOOT')
      dev=$ONIE_DEVICE
    fi
  fi
  test -b "$dev" || return 0

  do_recover=

  # simple validation using sgdisk
  if test "$do_recover"; then
    :
  else
    if sgdisk -p "$dev" > "$buf" 2>&1; then
      sts=0
    else
      sts=$?
    fi
    if test $sts -ne 0; then
      cat "$buf" 1>&2
      rm -f "$buf"
      installer_say "Cannot reliably get GPT partition table"
      return 1
    fi

    case $(cat "$buf") in
      *Caution*|*Warning*)
        cat $buf 1>&2
        installer_say "Found issues with the GPT partition table"
        do_recover=1
        rm -f "$buf"
      ;;
    esac

  fi

  # more thorough validation
  if test "$do_recover"; then
    :
  else

    local inp
    inp=$(mktemp -u -t sgdisk-XXXXXX)
    cat > "$inp" <<-END
	x
	r
	v
	q
	END
    if gdisk "$dev" < "$inp" > "$buf" 2>&1; then
      sts=0
    else
      sts=$?
    fi
    rm -f "$inp"
    if test $sts -ne 0; then
      cat "$buf" 1>&2
      rm -f "$buf"
      installer_say "Cannot reliably verify GPT partition table"
      return 1
    fi

    case $(cat "$buf") in
      *Caution*|*Warning*|*Problem:*)
        cat $buf 1>&2
        installer_say "Found issues with the GPT partition table"
        do_recover=1
        rm -f "$buf"
      ;;
    esac

  fi

  if test "$do_recover"; then
    :
  else
    installer_say "Found a clean GPT partition table"
    rm -f "$buf"
    return 0
  fi
  installer_say "Attempting to correct the GPT partition table"

  # this is the simple method; gdisk/sfgdisk will correct
  # simple errors but not horrendous faults
  dat=$(mktemp -u -t sgdisk-XXXXXX)
  sgdisk -b "$dat" "$dev" || return 1
  sgdisk -l "$dat" "$dev" || return 1
  rm -f "$dat"

  return 0
}

# Local variables
# mode: sh
# sh-basic-offset: 2
# End:
