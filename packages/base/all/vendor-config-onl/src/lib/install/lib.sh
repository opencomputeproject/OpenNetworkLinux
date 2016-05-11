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

  # special handling for /dev, which usually already has nested mounts
  installer_say "Setting up /dev"
  rm -fr "${rootdir}/dev"/*
  for dev in /dev/*; do
    if test -d "$dev"; then
      mkdir "${rootdir}${dev}"
    else
      cp -a "$dev" "${rootdir}${dev}"
    fi
  done
  mkdir -p "${rootdir}/dev/pts"

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
  mount -t devpts devpts "${rootdir}/dev/pts"

  if test ${TMPDIR+set}; then
    # make the tempdir available to the chroot
    mkdir -p "${rootdir}${TMPDIR}"
  fi

  # export ONIE defines to the installer
  if test -r /etc/machine.conf; then
    cp /etc/machine.conf "${rootdir}/etc/machine.conf"
  fi

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

# Local variables
# mode: sh
# sh-basic-offset: 2
# End:
