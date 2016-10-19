#!/usr/bin/python
############################################################
#
# ONL Loader Upgrade
#
############################################################
import os
import sys
import fnmatch

from onl.upgrade import ubase
from onl.sysconfig import sysconfig
from onl.mounts import OnlMountManager, OnlMountContextReadOnly, OnlMountContextReadWrite

class LoaderUpgradeBase(ubase.BaseUpgrade):
    name="loader"
    Name="Loader"
    title="Loader Upgrade Check"
    atype="A Loader"

    current_version_key="Current Loader Version"
    next_version_key="Next Loader Version"

    def auto_upgrade_default(self):
        return sysconfig.upgrade.loader.auto

    def init_versions(self):

        #
        # Current Loader version file.
        # If this file doesn't exist then in-place upgrade is not supported.
        #
        ETC_LOADER_VERSIONS_JSON = sysconfig.upgrade.loader.versions

        # Upgrade Loader Version file.
        NEXT_LOADER_VERSIONS_JSON = os.path.join(sysconfig.upgrade.loader.package.dir, "manifest.json")


        self.current_version = self.load_json(ETC_LOADER_VERSIONS_JSON,
                                              "RELEASE_ID",
                                              None)

        self.next_version = self.load_json(NEXT_LOADER_VERSIONS_JSON,
                                           "version", {}).get('RELEASE_ID', None)

    def summarize(self):
        self.logger.info("Current Loader Version: %s" % self.current_version)
        self.logger.info("   Next Loader Version: %s" % self.next_version)
        self.logger.info("")


    def upgrade_notes(self):
        return """
    * A single reboot will be required to complete this upgrade.
"""


class LoaderUpgrade_Fit(LoaderUpgradeBase):

    def do_upgrade(self, forced=False):

        fit_image = None
        for f in sysconfig.upgrade.loader.package.fit:
            fp = os.path.join(sysconfig.upgrade.loader.package.dir, f)
            if os.path.exists(fp):
                fit_image = fp;
                break

        if fit_image is None:
            self.abort("The FIT upgrade image is missing. Upgrade cannot continue.")

        with OnlMountContextReadWrite("ONL-BOOT", self.logger) as d:
            self.copyfile(fit_image, os.path.join(d.directory, "%s.itb" % (self.platform.platform())))

        self.reboot()


class LoaderUpgrade_x86_64(LoaderUpgradeBase):

    def do_upgrade(self, forced=False):

        X86_64_UPGRADE_DIR=sysconfig.upgrade.loader.package.dir
        X86_64_UPGRADE_KERNEL_PATTERNS = [ "kernel-*" ]

        with OnlMountContextReadWrite("ONL-BOOT", self.logger) as d:
            for f in os.listdir(X86_64_UPGRADE_DIR):
                for pattern in X86_64_UPGRADE_KERNEL_PATTERNS:
                    if fnmatch.fnmatch(f, pattern):
                        self.copyfile(os.path.join(X86_64_UPGRADE_DIR, f), os.path.join(d.directory, f))

            initrd = None
            for c in sysconfig.upgrade.loader.package.grub:
                initrd = os.path.join(X86_64_UPGRADE_DIR, c)
                if os.path.exists(initrd):
                    break
                else:
                    initrd = None

            if initrd:
                self.copyfile(initrd, os.path.join(d.directory, "%s.cpio.gz" % self.platform.platform()))
            else:
                self.abort("Initrd is missing. Upgrade cannot continue.")

            # Disabled until it can be resolved with the new installer.
            #src = "/lib/platform-config/current/onl/boot/grub.cfg"
            #dst = os.path.join(d.directory, "grub/grub.cfg")
            #if os.path.exists(src):
            #    self.copyfile(src, dst)

        self.reboot()


import platform
arch = platform.machine()
LoaderUpgrade = None

if arch in [ 'ppc', 'armv7l', 'aarch64', 'arm64' ]:
    LoaderUpgrade = LoaderUpgrade_Fit
elif arch == 'x86_64':
    LoaderUpgrade = LoaderUpgrade_x86_64

