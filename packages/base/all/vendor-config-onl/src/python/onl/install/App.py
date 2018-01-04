"""App.py

top-level install app
"""

import subprocess
import sys, os
import logging
import imp
import glob
import argparse
import shutil
import urllib
import tempfile
import time

from InstallUtils import InitrdContext
from InstallUtils import SubprocessMixin
from InstallUtils import ProcMountsParser
from ShellApp import OnieBootContext, OnieSysinfo
import ConfUtils, BaseInstall

class App(SubprocessMixin, object):

    def __init__(self, url=None,
                 debug=False, force=False,
                 log=None):

        if log is not None:
            self.log = log
        else:
            self.log = logging.getLogger(self.__class__.__name__)

        self.url = url
        self.force = force
        self.debug = debug
        # remote-install mode

        self.installer = None
        self.machineConf = None
        self.installerConf = None
        self.onlPlatform = None
        # local-install mode

        self.nextUpdate = None

        self.octx = None

    def run(self):

        if self.url is not None:
            return self.runUrl()
        else:
            return self.runLocal()

    def runUrl(self):
        pm = ProcMountsParser()
        for m in pm.mounts:
            if m.dir.startswith('/mnt/onl'):
                if not self.force:
                    self.log.error("directory %s is still mounted", m.dir)
                    return 1
                self.log.warn("unmounting %s (--force)", m.dir)
                self.check_call(('umount', m.dir,))

        def reporthook(blocks, bsz, sz):
            if time.time() < self.nextUpdate: return
            self.nextUpdate = time.time() + 0.25
            if sz:
                pct = blocks * bsz * 100 / sz
                sys.stderr.write("downloaded %d%% ...\r" % pct)
            else:
                icon = "|/-\\"[blocks % 4]
                sys.stderr.write("downloading ... %s\r" % icon)

        p = tempfile.mktemp(prefix="installer-",
                            suffix=".bin")
        try:
            self.log.info("downloading installer from %s --> %s",
                          self.url, p)
            self.nextUpdate = 0
            if os.isatty(sys.stdout.fileno()):
                dst, headers = urllib.urlretrieve(self.url, p, reporthook)
            else:
                dst, headers = urllib.urlretrieve(self.url, p)
            sys.stdout.write("\n")

            self.log.debug("+ chmod +x %s", p)
            os.chmod(p, 0755)

            env = {}
            env.update(os.environ)

            if os.path.exists("/etc/onl/platform"):
                self.log.debug("enabling unzip features for ONL")
                env['SFX_UNZIP'] = '1'
                self.log.debug("+ export SFX_UNZIP=1")
                env['SFX_LOOP'] = '1'
                self.log.debug("+ export SFX_LOOP=1")
                env['SFX_PIPE'] = '1'
                self.log.debug("+ export SFX_PIPE=1")

            self.log.debug("enabling in-place fixups")
            env['SFX_INPLACE'] = '1'
            self.log.debug("+ export SFX_INPLACE=1")

            if self.debug:
                self.log.debug("enabling installer debug")
                env['installer_debug'] = 'y'
                self.log.debug("+ export installer_debug=y")
            if self.log.level < logging.INFO:
                self.log.debug("enabling installer verbose logging")
                env['installer_verbose'] = 'y'
                self.log.debug("+ export installer_verbose=y")

            self.log.info("invoking installer...")
            try:
                self.check_call((p,), env=env)
            except subprocess.CalledProcessError as ex:
                self.log.error("installer failed")
                return ex.returncode
        finally:
            if os.path.exists(p):
                os.unlink(p)

        self.log.info("please reboot this system now.")
        return 0

    def runLocalOrChroot(self):

        if self.machineConf is None:
            self.log.error("missing onie-sysinfo or machine.conf")
            return 1
        if self.installerConf is None:
            self.log.error("missing installer.conf")
            return 1

        self.log.info("ONL Installer %s", self.installerConf.onl_version)

        code = self.findPlatform()
        if code: return code

        try:
            import onl.platform.current
        except:
            self.log.exception("cannot find platform config")
            code = 1
            if self.log.level < logging.INFO:
                self.post_mortem()
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

        self.grubEnv = None

        if 'grub' in self.onlPlatform.platform_config:
            ##self.log.info("using native GRUB")
            ##self.grubEnv = ConfUtils.GrubEnv(log=self.log.getChild("grub"))

            with OnieBootContext(log=self.log) as self.octx:

                self.octx.ictx.attach()
                self.octx.ictx.unmount()
                self.octx.ictx.detach()
                # XXX roth -- here, detach the initrd mounts

                self.octx.detach()

            if self.octx.onieDir is not None:
                self.log.info("using native ONIE initrd+chroot GRUB (%s)", self.octx.onieDir)
                self.grubEnv = ConfUtils.ChrootGrubEnv(self.octx.initrdDir,
                                                       bootDir=self.octx.onieDir,
                                                       path="/grub/grubenv",
                                                       log=self.log.getChild("grub"))
                # direct access using ONIE initrd as a chroot
                # (will need to fix up bootDir and bootPart later)

        if self.grubEnv is None:
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

        # run the platform-specific installer
        self.installer = iklass(machineConf=self.machineConf,
                                installerConf=self.installerConf,
                                platformConf=self.onlPlatform.platform_config,
                                grubEnv=self.grubEnv,
                                ubootEnv=self.ubootEnv,
                                force=self.force,
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

    def runLocal(self):

        self.log.info("getting installer configuration")
        osi = OnieSysinfo(log=self.log.getChild("onie-sysinfo"))
        try:
            halp = osi.help
        except AttributeError:
            halp = None
        if halp is not None:
            self.machineConf = osi
        elif os.path.exists(ConfUtils.MachineConf.PATH):
            self.machineConf = ConfUtils.MachineConf()
        else:
            self.log.warn("missing onie-sysinfo or /etc/machine.conf from ONIE runtime")
            self.machineConf = ConfUtils.MachineConf(path='/dev/null')

        self.installerConf = ConfUtils.InstallerConf()

        return self.runLocalOrChroot()

    def findPlatform(self):

        plat = arch = None

        def _p2a(plat):
            if plat.startswith('x86-64'):
                return 'x86_64'
            else:
                return plat.partition('-')[0]

        # recover platform specifier from installer configuration
        if plat is None:
            plat = getattr(self.installerConf, 'onie_platform', None)
            if plat:
                self.log.info("ONL installer running chrooted.")
                plat = plat.replace('_', '-').replace('.', '-')
            arch = getattr(self.installerConf, 'onie_arch', None)

        # recover platform specifier from legacy ONIE machine.conf
        if plat is None and self.machineConf is not None:
            plat = getattr(self.machineConf, 'onie_platform', None)
            arch = getattr(self.machineConf, 'onie_arch', None)
            if plat:
                self.log.info("ONL installer running under ONIE.")
                plat = plat.replace('_', '-').replace('.', '-')

        # recover platform specifier from ONL runtime
        if plat is None and os.path.exists("/etc/onl/platform"):
            with open("/etc/onl/platform") as fd:
                plat = fd.read().strip()
            self.log.info("ONL installer running under ONL or ONL loader.")

        if plat is not None and arch is None:
            arch = _p2a(plat)

        if plat and arch:
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

        ctx, self.octx = self.octx, None
        if ctx:
            ctx.attach()
            ctx.shutdown()

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

        onie_verbose = 'onie_verbose' in os.environ
        installer_debug = 'installer_debug' in os.environ

        ap = argparse.ArgumentParser()
        ap.add_argument('-v', '--verbose', action='store_true',
                        default=onie_verbose,
                        help="Enable verbose logging")
        ap.add_argument('-D', '--debug', action='store_true',
                        default=installer_debug,
                        help="Enable python debugging")
        ap.add_argument('-U', '--url', type=str,
                        help="Install from a remote URL")
        ap.add_argument('-F', '--force', action='store_true',
                        help="Unmount filesystems before install")
        ops = ap.parse_args()

        if ops.verbose:
            logger.setLevel(logging.DEBUG)

        app = cls(url=ops.url, force=ops.force,
                  log=logger)
        try:
            code = app.run()
        except:
            logger.exception("runner failed")
            code = 1
            if ops.debug:
                app.post_mortem()

        app.shutdown()
        sys.exit(code)

main = App.main

if __name__ == "__main__":
    main()
