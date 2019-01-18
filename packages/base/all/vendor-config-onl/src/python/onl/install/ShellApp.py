"""ShellApp.py
"""

import os, sys
import glob
import tempfile
import logging
import subprocess
import argparse
import string
import struct
from InstallUtils import InitrdContext, MountContext
from InstallUtils import SubprocessMixin
from InstallUtils import ProcMountsParser, ProcMtdParser
from InstallUtils import BlkidParser
from InstallUtils import UbootInitrdContext

class AppBase(SubprocessMixin, object):

    @property
    def PROG(self):
        raise NotImplementedError

    def __init__(self, command=None, log=None):

        if log is not None:
            self.log = log
        else:
            self.log = logging.getLogger(self.__class__.__name__)
        self.command = command

    def _runInitrdShell(self, initrd):
        with InitrdContext(initrd=initrd, log=self.log) as ctx:
            if self.command is not None:
                cmd = ('chroot', ctx.dir,
                       '/bin/sh', '-c', 'IFS=;' + self.command)
            else:
                cmd = ('chroot', ctx.dir,
                       '/bin/sh', '-i')
            try:
                self.check_call(cmd)
            except subprocess.CalledProcessError, what:
                pass
        return 0

    def _runFitShell(self, device):
        with UbootInitrdContext(path=device, log=self.log) as ctx:
            return self._runInitrdShell(ctx.initrd)

    def shutdown(self):
        pass

    @classmethod
    def main(cls):

        logging.basicConfig()
        logger = logging.getLogger(cls.PROG)
        logger.setLevel(logging.INFO)

        ap = argparse.ArgumentParser(prog=cls.PROG)
        ap.add_argument('-v', '--verbose', action='store_true',
                        help='Enable verbose logging')
        ap.add_argument('-q', '--quiet', action='store_true',
                        help='Suppress logging')
        ap.add_argument('-c', type=str, dest='command',
                        help='Run a batch command')

        try:
            args = ap.parse_args()
        except SystemExit, what:
            sys.exit(what.code)

        if args.verbose:
            logger.setLevel(logging.DEBUG)
        if args.quiet:
            logger.setLevel(logging.ERROR)

        app = cls(command=args.command, log=logger)
        try:
            code = app.run()
        except:
            logger.exception("runner failed")
            code = 1
        app.shutdown()
        sys.exit(code)

class OnieBootContext:
    """Find the ONIE initrd and unpack/mount it."""

    def __init__(self, log=None):
        self.log = log or logging.getLogger(self.__class__.__name__)

        self.initrd = None

        self.pm = self.blkid = self.mtd = None
        self.ictx = self.dctx = self.fctx = None
        self.onieDir = None
        self.initrdDir = None

        self.__ictx = self.__dctx = self.__fctx = None

    def __enter__(self):

        self.pm = ProcMountsParser()
        self.blkid = BlkidParser(log=self.log.getChild("blkid"))
        self.mtd = ProcMtdParser(log=self.log.getChild("mtd"))

        def _g(d):
            pat = os.path.join(d, "onie/initrd.img*")
            l = glob.glob(pat)
            if l: return l[0]
            return None

        # try to find a mounted, labeled partition
        try:
            dev = self.blkid['ONIE-BOOT'].device
        except IndexError:
            dev = None
        if dev is not None:
            self.log.debug("found ONIE boot device %s", dev)

            parts = [p for p in self.pm.mounts if p.device == dev]
            if parts:
                self.log.debug("found ONIE boot mounted at %s", parts[0].dir)
                initrd = _g(parts[0].dir)
                if initrd is None:
                    raise ValueError("cannot find ONIE initrd on %s" % parts[0].dir)
                self.onieDir = parts[0].dir
                self.log.debug("found ONIE initrd at %s", initrd)
                with InitrdContext(initrd=initrd, log=self.log) as self.ictx:
                    self.initrd = initrd
                    self.initrdDir = self.ictx.dir
                    self.ictx.detach()
                    return self

            # else, try to mount the directory containing the initrd
            with MountContext(dev, log=self.log) as self.dctx:
                initrd = _g(self.dctx.dir)
                if initrd is None:
                    raise ValueError("cannot find ONIE initrd on %s" % dev)
                self.onieDir = self.dctx.dir
                self.dctx.detach()
                self.log.debug("found ONIE initrd at %s", initrd)
                with InitrdContext(initrd=initrd, log=self.log) as self.ictx:
                    self.initrd = initrd
                    self.initrdDir = self.ictx.dir
                    self.ictx.detach()
                    return self

            raise ValueError("cannot find an ONIE initrd")

        # try to find onie initrd on a mounted fs (GRUB);
        # for ONIE images this is usually /mnt/onie-boot
        for part in self.pm.mounts:
            if not part.device.startswith('/dev/'): continue
            initrd = _g(part.dir)
            if initrd is None:
                self.log.debug("cannot find ONIE initrd on %s (%s)",
                               part.device, part.dir)
            else:
                self.onieDir = part.dir
                self.log.debug("found ONIE initrd at %s", initrd)
                with InitrdContext(initrd=initrd, log=self.log) as self.ictx:
                    self.initrd = initrd
                    self.initrdDir = self.ictx.dir
                    self.ictx.detach()
                    return self

        # grovel through MTD devices (u-boot)
        parts = [p for p in self.mtd.parts if p.label == "onie"]
        if parts:
            part = parts[0]
            self.log.debug("found ONIE MTD device %s",
                           part.charDevice or part.blockDevice)
            with UbootInitrdContext(part.blockDevice, log=self.log) as self.fctx:
                with InitrdContext(initrd=self.fctx.initrd, log=self.log) as self.ictx:
                    self.initrd = self.fctx.initrd
                    self.fctx.detach()
                    self.initrdDir = self.ictx.dir
                    self.ictx.detach()
                    return self

        if self.mtd.mounts:
            raise ValueError("cannot find ONIE MTD device")

        raise ValueError("cannot find ONIE initrd")

    def shutdown(self):
        ctx, self.fctx = self.fctx, None
        if ctx is not None:
            ctx.shutdown()
            ctx.attach()
            ctx.shutdown()
        ctx, self.ictx = self.ictx, None
        if ctx is not None:
            ctx.shutdown()
            ctx.attach()
            ctx.shutdown()
        ctx, self.dctx = self.dctx, None
        if ctx is not None:
            ctx.shutdown()
            ctx.attach()
            ctx.shutdown()

    def __exit__(self, eType, eValue, eTrace):
        self.shutdown()
        return False

    def detach(self):
        self.__fctx, self.fctx = self.fctx, None
        self.__ictx, self.ictx = self.ictx, None
        self.__dctx, self.dctx = self.dctx, None

    def attach(self):
        self.fctx = self.__fctx
        self.ictx = self.__ictx
        self.dctx = self.__dctx

class Onie(AppBase):
    """XXX roth -- refactor in from loader.py code."""

    PROG = "onie-shell"

    def run(self):
        with OnieBootContext(log=self.log) as ctx:
            return self._runInitrdShell(ctx.initrd)

class OnieSysinfoApp(SubprocessMixin, object):

    PROG = "onie-sysinfo"

    def __init__(self, args=[], log=None):

        if log is not None:
            self.log = log
        else:
            self.log = logging.getLogger(self.__class__.__name__)
        self.args = args or ['-p',]
        self.output = None

    def _runInitrdShell(self, initrd):
        with InitrdContext(initrd=initrd, log=self.log) as ctx:
            cmd = ['onie-sysinfo',]
            cmd.extend(self.args)
            cmd = 'IFS=;' + " ".join(cmd)
            cmd = ('chroot', ctx.dir,
                   '/bin/sh', '-c', cmd,)
            try:
                self.output = self.check_output(cmd)
                ret = 0
            except subprocess.CalledProcessError, what:
                self.log.error("failed command: %s", " ".join(what.cmd))
                for line in (what.output or "").splitlines():
                    self.log.error(">>> %s", line)
                ret = what.returncode
        return ret

    def run(self):
        with OnieBootContext(log=self.log) as ctx:
            ret = self._runInitrdShell(ctx.initrd)
            if self.output is not None:
                sys.stdout.write(self.output)
            return ret

    def shutdown(self):
        pass

    @classmethod
    def main(cls):

        logging.basicConfig()
        logger = logging.getLogger(cls.PROG)
        logger.setLevel(logging.INFO)

        args = list(sys.argv[1:])
        sysinfoArgs = []
        while args:

            if args[0] in ('-v', '--verbose',):
                logger.setLevel(logging.DEBUG)
                args.pop(0)
                continue

            if args[0] in ('-q', '--quiet',):
                logger.setLevel(logging.ERROR)
                args.pop(0)
                continue

            sysinfoArgs.append(args.pop(0))

        app = cls(args=sysinfoArgs, log=logger)
        try:
            code = app.run()
        except:
            logger.exception("runner failed")
            code = 1
        app.shutdown()
        sys.exit(code)

class OnieSysinfo(OnieSysinfoApp):

    def _runArgs(self, *args):
        self.args = args
        with OnieBootContext(log=self.log) as ctx:
            ret = self._runInitrdShell(ctx.initrd)
            if self.output is not None:
                return self.output.rstrip()
            raise AttributeError("cannot retrieve onie-sysinfo attribute via %s" % str(args))

    @property
    def help(self):
        return self._runArgs('-h')

    @property
    def onie_platform(self):
        return self._runArgs('-p')

    @property
    def onie_arch(self):
        return self._runArgs('-c')

    @property
    def onie_version(self):
        return self._runArgs('-v')

    # XXX roth other switches too

class Loader(AppBase):
    """Application shell that uses the (installed) loader runtime."""

    PROG = "loader-shell"

    def runGrub(self):

        try:
            dev = self.blkid['ONL-BOOT'].device
        except KeyError:
            pass
        if dev is None:
            self.log.error("cannot find GRUB partition %s", dev)
            return 1

        initrd = self.pc['grub']['initrd']
        if type(initrd) == dict: initrd = initrd['=']

        parts = [p for p in self.pm.mounts if p.device == dev]
        if parts:
            grubDir = parts[0]
            self.log.debug("found loader device %s mounted at %s",
                           dev, grubDir)
            p = os.path.join(grubDir, initrd)
            if not os.path.exists(p):
                self.log.error("cannot find initrd %s", p)
                return 1
            self.log.debug("found loader initrd at %s", p)
            return self._runInitrdShell(p)

        with MountContext(dev, log=self.log) as ctx:
            p = os.path.join(ctx.dir, initrd)
            if not os.path.exists(p):
                self.log.error("cannot find initrd %s:%s", dev, p)
                return 1
            self.log.debug("found loader initrd at %s:%s", dev, p)
            return self._runInitrdShell(p)

    def runUboot(self):

        dev = self.pc['loader']['device']
        self.log.info("found loader device %s", dev)

        parts = self.pc['installer']
        bootPart = None
        bootPartno = None
        for idx, part in enumerate(self.pc['installer']):
            label, pdata = list(part.items())[0]
            if label == 'ONL-BOOT':
                bootPart = pdata
                bootPartno = idx + 1
                break
        if bootPart is None:
            self.log.info("cannot find ONL-BOOT declaration")
            return 1

        fmt = bootPart.get('format', 'ext2')
        if fmt == 'raw':
            bootDevice = dev + str(bootPartno)
        else:
            bootDevice = self.blkid['ONL-BOOT'].device

        # run from a raw partition
        if fmt == 'raw':
            self.log.info("found (raw) boot partition %s", bootDevice)
            return self._runFitShell(bootDevice)

        l = []

        p = self.pc['flat_image_tree']['itb']
        if type(p) == dict: p = p['=']
        if p not in l: l.append(p)

        p = self.platform.platform() + '.itb'
        if p not in l: l.append(p)

        p = 'onl-loader-fit.itb'
        if p not in l: l.append(p)

        self.log.info("looking for loader images %s", ", ".join(l))

        # run from a file in a mounted filesystem
        parts = [p for p in self.pm.mounts if p.device == bootDevice]
        if parts:
            loaderDir = parts[0].dir
            self.log.debug("found loader device mounted at %s", loaderDir)
            for e in l:
                p = os.path.join(loaderDir, e)
                if os.path.exists(p): return self._runFitShell(p)
            self.log.error("cannot find an ITB")
            return 1

        # run from a file in an umounted filesystem
        with MountContext(bootDevice, log=self.log) as ctx:
            self.log.info("found (%s) loader device %s", fmt, bootDevice)
            for e in l:
                p = os.path.join(ctx.dir, e)
                if os.path.exists(p): return self._runFitShell(p)
            self.log.error("cannot find an ITB")
            return 1

    def run(self):

        import onl.platform.current
        self.platform = onl.platform.current.OnlPlatform()
        self.pc = self.platform.platform_config

        self.pm = ProcMountsParser()
        self.blkid = BlkidParser(log=self.log.getChild("blkid"))

        if 'grub' in self.pc:
            return self.runGrub()

        if 'flat_image_tree' in self.pc:
            return self.runUboot()

        self.log.error("invalid platform-config")
        return 1

class Upgrader(AppBase):
    """Application shell that uses on-disk upgrade loader runtime."""

    PROG = "upgrade-shell"

    def runGrub(self):

        from onl.sysconfig import sysconfig
        d = sysconfig.upgrade.loader.package.dir
        for b in sysconfig.upgrade.loader.package.grub:
            p = os.path.join(d, b)
            if os.path.exists(p):
                self.log.debug("found upgrade initrd at %s", p)
                return self._runInitrdShell(p)

        self.log.error("cannot find upgrade initrd")
        return 1

    def runUboot(self):

        from onl.sysconfig import sysconfig
        d = sysconfig.upgrade.loader.package.dir
        for b in sysconfig.upgrade.loader.package.fit:
            p = os.path.join(d, b)
            if os.path.exists(p):
                self.log.debug("found upgrade FIT image %s", p)
                return self._runFitShell(p)

        self.log.error("cannot find FIT image")
        return 1

    def run(self):

        import onl.platform.current
        self.platform = onl.platform.current.OnlPlatform()
        self.pc = self.platform.platform_config

        if 'grub' in self.pc:
            return self.runGrub()

        if 'flat_image_tree' in self.pc:
            return self.runUboot()

        self.log.error("invalid platform-config")
        return 1

main = Onie.main

if __name__ == "__main__":
    main()
