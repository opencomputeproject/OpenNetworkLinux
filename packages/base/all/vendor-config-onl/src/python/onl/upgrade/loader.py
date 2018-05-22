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
from onl.install import BaseInstall, ConfUtils, InstallUtils
from onl.install.ShellApp import OnieBootContext, OnieSysinfo
import onl.platform.current
import onl.versions

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

    def prepare_upgrade(self):
        pass

    def summarize(self):
        self.logger.info("Current Loader Version: %s" % self.current_version)
        self.logger.info("   Next Loader Version: %s" % self.next_version)
        self.logger.info("")


    def upgrade_notes(self):
        return """
    * A single reboot will be required to complete this upgrade.
"""

class LoaderUpgrade_Fit(LoaderUpgradeBase):

    installer_klass = BaseInstall.UbootInstaller

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

        onlPlatform = onl.platform.current.OnlPlatform()

        with OnieBootContext(log=self.logger) as octx:
            if os.path.exists("/usr/bin/onie-shell"):
                machineConf = OnieSysinfo(log=self.logger.getChild("onie-sysinfo"))
            else:
                path = os.path.join(octx.initrdDir, "etc/machine.conf")
                if os.path.exists(path):
                    machineConf = ConfUtils.MachineConf(path=path)
                else:
                    machineConf = ConfUtils.MachineConf(path='/dev/null')

        installerConf = ConfUtils.InstallerConf(path="/dev/null")
        # start with an empty installerConf, fill it in piece by piece

        installerConf.installer_platform = onlPlatform.platform()
        installerConf.installer_arch = machineConf.onie_arch
        installerConf.installer_platform_dir = os.path.join("/lib/platform-config",
                                                            onlPlatform.platform())

        mfPath = os.path.join(sysconfig.upgrade.loader.package.dir, "manifest.json")
        mf = onl.versions.OnlVersionManifest(mfPath)
        installerConf.onl_version = mf.RELEASE_ID

        grubEnv = ConfUtils.ProxyGrubEnv(installerConf,
                                         bootDir="/mnt/onie-boot",
                                         path="/grub/grubenv",
                                         chroot=False,
                                         log=self.logger.getChild("grub"))

        ubootEnv = ConfUtils.UbootEnv(log=self.logger.getChild("u-boot"))

        installer = self.installer_klass(machineConf=machineConf,
                                         installerConf=installerConf,
                                         platformConf=onlPlatform.platform_config,
                                         grubEnv=grubEnv,
                                         ubootEnv=ubootEnv,
                                         force=True,
                                         log=self.logger)

        installer.upgradeBootLoader()
        installer.shutdown()

        self.reboot()


class LoaderUpgrade_x86_64(LoaderUpgradeBase, InstallUtils.SubprocessMixin):

    installer_klass = BaseInstall.GrubInstaller

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

        # installer assumes that partitions are unmounted

        self.log = self.logger
        # ha ha, SubprocessMixin api is different

        pm = InstallUtils.ProcMountsParser()
        for m in pm.mounts:
            if m.dir.startswith('/mnt/onl'):
                self.logger.warn("unmounting %s (--force)", m.dir)
                self.check_call(('umount', m.dir,))

        onlPlatform = onl.platform.current.OnlPlatform()

        with OnieBootContext(log=self.log) as octx:

            octx.ictx.attach()
            octx.ictx.unmount()
            octx.ictx.detach()
            # XXX roth -- here, detach the initrd mounts

            octx.detach()
        # hold on to the ONIE boot context for grub access

        if os.path.exists("/usr/bin/onie-shell"):
            machineConf = OnieSysinfo(log=self.logger.getChild("onie-sysinfo"))
        else:
            path = os.path.join(octx.initrdDir, "etc/machine.conf")
            if os.path.exists(path):
                machineConf = ConfUtils.MachineConf(path=path)
            else:
                machineConf = ConfUtils.MachineConf(path='/dev/null')

        installerConf = ConfUtils.InstallerConf(path="/dev/null")

        # XXX fill in installerConf fields
        installerConf.installer_platform = onlPlatform.platform()
        installerConf.installer_arch = machineConf.onie_arch
        installerConf.installer_platform_dir = os.path.join("/lib/platform-config",
                                                            onlPlatform.platform())

        mfPath = os.path.join(sysconfig.upgrade.loader.package.dir, "manifest.json")
        mf = onl.versions.OnlVersionManifest(mfPath)
        installerConf.onl_version = mf.RELEASE_ID

        grubEnv = ConfUtils.ChrootGrubEnv(octx.initrdDir,
                                          bootDir=octx.onieDir,
                                          path="/grub/grubenv",
                                          log=self.logger.getChild("grub"))

        ubootEnv = None

        installer = self.installer_klass(machineConf=machineConf,
                                         installerConf=installerConf,
                                         platformConf=onlPlatform.platform_config,
                                         grubEnv=grubEnv,
                                         ubootEnv=ubootEnv,
                                         force=True,
                                         log=self.logger)

        installer.upgradeBootLoader()
        installer.shutdown()

        self.reboot()


import platform
arch = platform.machine()
LoaderUpgrade = None

if arch in [ 'ppc', 'armv7l', 'aarch64', 'arm64' ]:
    LoaderUpgrade = LoaderUpgrade_Fit
elif arch == 'x86_64':
    LoaderUpgrade = LoaderUpgrade_x86_64

