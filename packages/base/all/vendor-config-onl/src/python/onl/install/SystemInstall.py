"""App.py

Application code for swl-install.
"""

import logging
import os, sys
import json
import tempfile
import zipfile
import shutil
import argparse
import fnmatch
import subprocess

from onl.install.InstallUtils import InitrdContext, MountContext
from onl.install.InstallUtils import BlkidParser, ProcMountsParser
from onl.install.ConfUtils import MachineConf, InstallerConf
from onl.install.ShellApp import Onie, Upgrader
import onl.install.App
from onl.sysconfig import sysconfig
from onl.platform.current import OnlPlatform

from onl.install.InstallUtils import SubprocessMixin

class UpgradeHelper(Upgrader):

    def __init__(self, callback=None, log=None):
        super(UpgradeHelper, self).__init__(log=log)
        self.callback = callback

    def _runInitrdShell(self, p):
        if self.callback is not None:
            self.callback(self, p)

class OnlBootContext(object):

    def __init__(self, log=None):
        self.log = log or logging.getLogger(self.__class__.__name__)
        self.dctx = None
        self.dir = None

    def __enter__(self):
        pm = ProcMountsParser()
        logger = self.log.getChild("blkid")
        blkid = BlkidParser(log=logger)

        try:
            dev = blkid['ONL-BOOT'].device
        except IndexError:
            dev = None
        if dev is None:
            raise ValueError("cannot find ONL-BOOT")

        parts = [p for p in pm.mounts if p.device == dev]
        if parts:
            self.log.debug("found ONL-BOOT at %s", parts[0].dir)
            self.dir = parts[0].dir
            return self

        # else, mount this:
        with MountContext(dev, log=self.log) as self.dctx:
            self.log.debug("mounted ONL-BOOT %s --> %s",
                           dev, self.dctx.dir)
            self.dir = self.dctx.dir
            self.dctx.detach()

        return self

    def __exit__(self, type, value, tb):
        dctx, self.dctx = self.dctx, None
        if dctx is not None:
            dctx.attach()
            dctx.shutdown()
        return False

class App(SubprocessMixin):

    def __init__(self, force=False, log=None):

        if log is not None:
            self.log = log
        else:
            self.log = logging.getLogger(self.__class__.__name__)

        self.force = force

        self.onieHelper = None

    def _runInitrd(self, helper, path):
        with InitrdContext(initrd=path, log=self.log) as ctx:

            tdir = os.path.join(ctx.dir, "tmp")
            abs_idir = tempfile.mkdtemp(dir=tdir,
                                        prefix="installer-", suffix=".d")
            chroot_idir = abs_idir[len(ctx.dir):]

            self.onieHelper = onl.install.App.OnieHelper(log=self.log)
            code = self.onieHelper.run()
            if code:
                self.log.error("cannot find/unpack ONIE initrd")
                return code
            self.log.info("onie directory is %s", self.onieHelper.onieDir)
            self.log.info("initrd directory is %s", self.onieHelper.initrdDir)

            src = os.path.join(self.onieHelper.initrdDir, "etc/machine.conf")
            dst = os.path.join(ctx.dir, "etc/machine.conf")
            self.log.debug("+ /bin/cp %s %s", src, dst)
            shutil.copy2(src, dst)

            h, self.onieHelper = self.onieHelper, None
            if h is not None:
                h.shutdown()

            src = "/etc/fw_env.config"
            if os.path.exists(src):
                dst = os.path.join(ctx.dir, "etc/fw_env.config")
                self.log.debug("+ /bin/cp %s %s", src, dst)
                shutil.copy2(src, dst)

            srcRoot = "/etc/onl"
            dstRoot = os.path.join(ctx.dir, "etc")
            self.cpR(srcRoot, dstRoot)

            # constitute an /etc/onl/installer.conf in place
            installerConf = InstallerConf(path="/dev/null")

            with open("/etc/onl/loader/versions.json") as fd:
                data = json.load(fd)
                installerConf.onl_version = data['VERSION_ID']

            installerConf.installer_dir = chroot_idir

            abs_postinst = tempfile.mktemp(dir=abs_idir,
                                           prefix="postinst-", suffix=".sh")
            chroot_postinst = abs_postinst[len(ctx.dir):]
            installerConf.installer_postinst = chroot_postinst

            # make an empty(ish) zip file (local path in installer_dir) for collateral
            zipPath = tempfile.mktemp(dir=abs_idir,
                                      prefix="install-", suffix=".zip")
            with zipfile.ZipFile(zipPath, "w") as zf:
                pass
            installerConf.installer_zip = os.path.split(zipPath)[1]

            # finalize the local installer.conf
            dst = os.path.join(ctx.dir, "etc/onl/installer.conf")
            with open(dst, "w") as fd:
                fd.write(installerConf.dumps())

            # populate installer_dir with the contents of the loader upgrade
            # See also Loader_Upgrade_x86_64.do_upgrade
            # Here the initrd filename is as per the installer.zip;
            # it is renamed on install to the grub directory
            sdir = sysconfig.upgrade.loader.package.dir

            # get kernels for grub installs:
            pats = ["kernel-*",]
            for f in os.listdir(sdir):
                for pat in pats:
                    if fnmatch.fnmatch(f, pat):
                        src = os.path.join(sdir, f)
                        dst = os.path.join(abs_idir, f)
                        self.log.debug("+ /bin/cp %s %s", src, dst)
                        shutil.copy2(src, dst)
            try:
                l = sysconfig.upgrade.loader.package.grub
            except AttributeError:
                l = []
            for f in l:
                src = os.path.join(sdir, f)
                if os.path.exists(src):
                    dst = os.path.join(abs_idir, f)
                    self.log.debug("+ /bin/cp %s %s", src, dst)
                    shutil.copy2(src, dst)

            # get FIT files from powerpc installs:
            try:
                l = sysconfig.upgrade.loader.package.fit
            except AttributeError:
                l = []
            for f in l:
                src = os.path.join(sdir, f)
                if os.path.exists(src):
                    dst = os.path.join(abs_idir, f)
                    self.log.debug("+ /bin/cp %s %s", src, dst)
                    shutil.copy2(src, dst)

            with OnlBootContext(log=self.log) as octx:
                src = os.path.join(octx.dir, "boot-config")
                dst = os.path.join(abs_idir, "boot-config")
                self.log.debug("+ /bin/cp %s %s", src, dst)
                shutil.copy2(src, dst)

            # chroot to the onl-install script
            ##cmd = ('chroot', ctx.dir,
            ##       '/bin/sh', '-i')
            if self.log.level < logging.INFO:
                cmd = ('chroot', ctx.dir, "/usr/bin/onl-install", "--verbose", "--force",)
            else:
                cmd = ('chroot', ctx.dir, "/usr/bin/onl-install", "--force",)
            try:
                self.check_call(cmd)
            except subprocess.CalledProcessError, what:
                pass

    def run(self):
        """XXX roth -- migrate this to onl.install.App.App

        XXX roth -- assume TMPDIR=/tmp.
        """

        pm = ProcMountsParser()

        # resize /tmp to be large enough for the initrd, see tmpfs
        # nonsense in installer.sh.in
        tflags = None
        tdev = os.stat('/tmp').st_dev
        pdir = None
        for m in pm.mounts:
            if m.fsType in ('ramfs', 'tmpfs',):
                dev = os.stat(m.dir).st_dev
                if dev == tdev:
                    self.log.info("found tmpfs/ramfs %s (%s)", dev, m.flags)
                    pdir = m.dir
                    tflags = m.flags

        # XXX glean this from install.sh.in (installer_tmpfs_kmin)
        if pdir is None:
            self.check_call(('mount',
                             '-o', 'size=1048576k',
                             '-t', 'tmpfs',
                             'tmpfs', '/tmp',))
        else:
            self.check_call(('mount',
                             '-o', 'remount,size=1048576k',
                             pdir,))

        for m in pm.mounts:
            if m.dir.startswith('/mnt/onl'):
                if not self.force:
                    self.log.error("directory %s is still mounted (try --force)", m.dir)
                    return 1
                self.log.warn("unmounting %s (--force)", m.dir)
                self.check_call(('umount', m.dir,))

        upgrader = UpgradeHelper(callback=self._runInitrd, log=self.log)
        try:
            code = upgrader.run()
        except:
            self.log.exception("upgrader failed")
            code = 1
        upgrader.shutdown()
        return code

    def shutdown(self):

        h, self.onieHelper = self.onieHelper, None
        if h is not None:
            h.shutdown()

    @classmethod
    def main(cls):

        logging.basicConfig()
        logger = logging.getLogger("swl-install")
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
        ap.add_argument('-F', '--force', action='store_true',
                        help="Unmount filesystems before install")
        ops = ap.parse_args()

        if ops.verbose:
            logger.setLevel(logging.DEBUG)

        app = cls(force=ops.force,
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
