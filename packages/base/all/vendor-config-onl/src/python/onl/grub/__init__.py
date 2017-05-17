from onl.mounts import OnlOnieBootContext, OnlMountContextReadWrite
import subprocess
import os

ONIE_BOOT_MODES = [ 'install',
                    'rescue',
                    'uninstall',
                    'update',
                    'embed',
                    'diag',
                    'none'
                    ]

def onie_boot_mode_set(mode):
    if mode not in ONIE_BOOT_MODES:
        raise ValueError("%s is not a valid onie boot mode." % mode)

    with OnlOnieBootContext() as ob:
        subprocess.check_call("%s/onie/tools/bin/onie-boot-mode -o %s" % (ob.directory, mode), shell=True)

def _makedirs(d):
    if not os.path.exists(d):
        os.makedirs(d)

def onie_fwpkg_exists():
    with OnlOnieBootContext() as ob:
        return os.path.exists(os.path.join(ob.directory, "onie/tools/bin/onie-fwpkg"))

def onie_fwpkg(arguments):
    with OnlOnieBootContext() as ob:
        # This is necessary if we've upgraded ONIE but haven't booted into it yet...
        _makedirs("%s/onie/update/pending" % ob.directory)
        _makedirs("%s/onie/update/attempts" % ob.directory)
        subprocess.check_call("%s/onie/tools/bin/onie-fwpkg %s" % (ob.directory, arguments), shell=True)

def boot_entry_set(index):
    with OnlMountContextReadWrite("ONL-BOOT", logger=None) as ob:
        subprocess.check_call("/usr/sbin/grub-set-default --boot-directory=%s %d" % (ob.directory, index), shell=True)

def boot_onie():
    return boot_entry_set(1)
