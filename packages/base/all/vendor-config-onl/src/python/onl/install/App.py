"""App.py

top-level install app
"""

import subprocess
import sys, os
import logging
import imp
import glob
import distutils.sysconfig

from InstallUtils import InitrdContext
from InstallUtils import SubprocessMixin
import ConfUtils, BaseInstall

import onl.platform.current

class App(SubprocessMixin):

    def __init__(self, log=None):

        if log is not None:
            self.log = log
        else:
            self.log = logging.getLogger(self.__class__.__name__)

        self.installer = None
        self.machineConf = None
        self.installerConf = None
        self.onlPlatform = None

    def run(self):

        self.log.info("getting installer configuration")
        self.machineConf = ConfUtils.MachineConf()
        self.installerConf = ConfUtils.InstallerConf()

        ##self.log.info("using native GRUB")
        ##self.grubEnv = ConfUtils.GrubEnv(log=self.log.getChild("grub"))

        pat = "/mnt/onie-boot/onie/initrd.img*"
        l = glob.glob(pat)
        if l:
            initrd = l[0]
            self.log.info("using native ONIE initrd+chroot GRUB (%s)", initrd)
            initrdDir = InitrdContext.mkChroot(initrd, log=self.log)
            self.grubEnv = ConfUtils.ChrootGrubEnv(initrdDir,
                                                   bootDir="/mnt/onie-boot",
                                                   path="/grub/grubenv",
                                                   log=self.log.getChild("grub"))
            # direct access using ONIE initrd as a chroot
            # (will need to fix up bootDir and bootPart later)
        else:
            self.log.info("using proxy GRUB")
            self.grubEnv = ConfUtils.ProxyGrubEnv(self.installerConf,
                                                  bootDir="/mnt/onie-boot",
                                                  path="/grub/grubenv",
                                                  chroot=False,
                                                  log=self.log.getChild("grub"))
            # indirect access through chroot host
            # (will need to fix up bootDir and bootPart later)

        if os.path.exists(ConfUtils.UbootEnv.SETENV):
            self.ubootEnv = ConfUtils.UbootEnv(log=self.log.getChild("u-boot"))
        else:
            self.ubootEnv = None

        self.log.info("ONL Installer %s", self.installerConf.onl_version)

        code = self.findPlatform()
        if code: return code

        self.onlPlatform = onl.platform.current.OnlPlatform()

        if 'grub' in self.onlPlatform.platform_config:
            self.log.info("trying a GRUB based installer")
            iklass = BaseInstall.GrubInstaller
        elif 'flat_image_tree' in self.onlPlatform.platform_config:
            self.log.info("trying a U-Boot based installer")
            iklass = BaseInstall.UbootInstaller
        else:
            self.log.error("cannot detect installer type")
            return 1

        # run the platform-specific installer
        self.installer = iklass(machineConf=self.machineConf,
                                installerConf=self.installerConf,
                                platformConf=self.onlPlatform.platform_config,
                                grubEnv=self.grubEnv,
                                ubootEnv=self.ubootEnv,
                                log=self.log)
        try:
            code = self.installer.run()
        except:
            self.log.exception("installer failed")
            code = 1
            if self.log.level < logging.INFO:
                self.post_mortem()
        if code: return code

        if getattr(self.installer, 'grub', False):
            code = self.finalizeGrub()
            if code: return code
        if getattr(self.installer, 'uboot', False):
            code = self.finalizeUboot()
            if code: return code

        self.log.info("Install finished.")
        return 0

    def findPlatform(self):

        plat = getattr(self.machineConf, 'onie_platform', None)
        arch = getattr(self.machineConf, 'onie_arch', None)
        if plat and arch:
            self.log.info("ONL installer running under ONIE.")
            plat = plat.replace('_', '-')
            self.installerConf.installer_platform = plat
            self.installerConf.installer_arch = arch
        else:
            self.log.error("The installation platform cannot be determined.")
            self.log.error("It does not appear that we are running under ONIE or the ONL loader.")
            self.log.error("If you know what you are doing you can re-run this installer")
            self.log.error("with an explicit 'installer_platform=<platform>' setting,")
            self.log.error("though this is unlikely to be the correct procedure at this point.")
            return 1

        self.log.info("Detected platform %s", self.installerConf.installer_platform)

        self.installerConf.installer_platform_dir = ("/lib/platform-config/%s"
                                                     % (self.installerConf.installer_platform,))
        if not os.path.isdir(self.installerConf.installer_platform_dir):
            self.log.error("This installer does not support the %s platform.",
                           self.installerConf.installer_platform)
            self.log.error("Available platforms are:")
            for d in os.listdir("/lib/platform-config"):
                self.log.error("  %s", d)
            self.log.error("Installation cannot continue.")
            return 1

        return 0

    def finalizeGrub(self):

        def _m(src, dst):
            val = getattr(self.installerConf, src, None)
            if val is not None:
                setattr(self.grubEnv, dst, val)
            else:
                delattr(self.grubEnv, dst)

        _m('installer_md5', 'onl_installer_md5')
        _m('onl_version', 'onl_installer_version')
        _m('installer_url', 'onl_installer_url')

        return 0

    def finalizeUboot(self):

        if self.installer.platform.isOnie():
            def _m(src, dst):
                val = getattr(self.installerConf, src, None)
                if val is not None:
                    setattr(self.ubootEnv, dst, val)
                else:
                    delattr(self.ubootEnv, dst)

            _m('installer_md5', 'onl_installer_md5')
            _m('onl_version', 'onl_installer_version')
            _m('installer_url', 'onl_installer_url')
        else:
            self.log.info("To configure U-Boot to boot ONL automatically, reboot the switch,")
            self.log.info("enter the U-Boot shell, and run these 2 commands:")
            self.log.info("=> setenv bootcmd '%s'", self.installer.platform.str_bootcmd())
            self.log.info("saveenv")

        return 0

    def shutdown(self):

        installer, self.installer = self.installer, None
        if installer is not None:
            installer.shutdown()

    def post_mortem(self):
        self.log.info("re-attaching to tty")
        fdno = os.open("/dev/console", os.O_RDWR)
        os.dup2(fdno, sys.stdin.fileno())
        os.dup2(fdno, sys.stdout.fileno())
        os.dup2(fdno, sys.stderr.fileno())
        os.close(fdno)

        self.log.info("entering Python debugger (installer_debug=1)")
        import pdb
        pdb.post_mortem(sys.exc_info()[2])

    @classmethod
    def main(cls):

        logging.basicConfig()
        logger = logging.getLogger("install")
        logger.setLevel(logging.DEBUG)

        # send to ONIE log
        hnd = logging.FileHandler("/dev/console")
        logger.addHandler(hnd)
        logger.propagate = False

        debug = 'installer_debug' in os.environ
        if debug:
            logger.setLevel(logging.DEBUG)

        app = cls(log=logger)
        try:
            code = app.run()
        except:
            logger.exception("runner failed")
            code = 1
            if debug:
                app.post_mortem()

        app.shutdown()
        sys.exit(code)

main = App.main

if __name__ == "__main__":
    main()
