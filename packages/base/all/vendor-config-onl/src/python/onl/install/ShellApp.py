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
import Fit

class AppBase(SubprocessMixin):

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

    def _runMtdShell(self, device):
        self.log.debug("parsing FIT image in %s", device)
        p = Fit.Parser(path=device, log=self.log)
        node = p.getInitrdNode()
        if node is None:
            self.log.error("cannot find initrd node in FDT")
            return 1
        prop = node.properties.get('data', None)
        if prop is None:
            self.log.error("cannot find initrd data property in FDT")
            return 1
        with open(device) as fd:
            self.log.debug("reading initrd at [%x:%x]",
                           prop.offset, prop.offset+prop.sz)
            fd.seek(prop.offset, 0)
            buf = fd.read(prop.sz)
        try:
            fno, initrd = tempfile.mkstemp(prefix="initrd-",
                                           suffix=".img")
            self.log.debug("+ cat > %s", initrd)
            with os.fdopen(fno, "w") as fd:
                fd.write(buf)
            return self._runInitrdShell(initrd)
        finally:
            self.unlink(initrd)

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

class Onie(AppBase):

    PROG = "onie-shell"

    def run(self):

        def _g(d):
            pat = os.path.join(d, "onie/initrd.img*")
            l = glob.glob(pat)
            if l: return l[0]
            return None

        # try to find onie initrd on a mounted fs (GRUB)
        initrd = _g("/mnt/onie-boot")
        if initrd is not None:
            self.log.debug("found ONIE initrd at %s", initrd)
            return self._runInitrdShell(initrd)

        # try to find the onie boot partition elsewhere
        pm = ProcMountsParser()
        try:
            dev = self.check_output(('blkid', '-L', 'ONIE-BOOT',)).strip()
        except subprocess.CalledProcessError, what:
            dev = None
        if dev is not None:
            self.log.debug("found ONIE boot device %s", dev)
            parts = [p for p in pm.mounts if p.device == dev]
            if parts:
                onieDir = parts[0]
                self.log.debug("found ONIE boot mounted at %s", onieDir)
                initrd = _g(onieDir)
                if initrd is not None:
                    self.log.debug("found ONIE initrd at %s", initrd)
                    return _runInitrdShell(initrd)
                else:
                    self.log.error("cannot find ONIE initrd")
                    return 1
            else:
                with MountContext(dev, fsType='ext4', log=self.log) as ctx:
                    initrd = _g(ctx.dir)
                    if initrd is not None:
                        self.log.debug("found ONIE initrd at %s", initrd)
                        return self._runInitrdShell(initrd)
                    else:
                        self.log.error("cannot find ONIE initrd")
                        return 1

        # grovel through MTD devices (u-boot)
        pm = ProcMtdParser(log=self.log)
        parts = [p for p in pm.parts if p.label == "onie"]
        if parts:
            part = parts[0]
            self.log.debug("found ONIE MTD device %s",
                           part.charDevice or part.blockDevice)
            return self._runMtdShell(part.blockDevice)
        elif pm.parts:
            self.log.error("cannot find ONIE MTD device")
            return 1

        self.log.error("cannot find ONIE initrd")
        return 1

class Loader(AppBase):

    PROG = "loader-shell"

    def run(self):

        def _g(d):
            pat = os.path.join(d, "initrd-*")
            l = glob.glob(pat)
            if l: return l[0]
            return None

        # try to find the loader boot partition as a formatted block device
        pm = ProcMountsParser()
        try:
            dev = self.check_output(('blkid', '-L', 'SL-BOOT',)).strip()
        except subprocess.CalledProcessError, what:
            dev = None
        if dev is not None:
            self.log.debug("found loader device %s", dev)
            parts = [p for p in pm.mounts if p.device == dev]
            if parts:
                loaderDir = parts[0]
                self.log.debug("found loader device mounted at %s", loaderDir)
                initrd = _g(loaderDir)
                if initrd is not None:
                    self.log.debug("found loader initrd at %s", initrd)
                    return _runInitrdShell(initrd)
                else:
                    self.log.error("cannot find loader initrd")
                    return 1
            else:
                with MountContext(dev, fsType='ext4', log=self.log) as ctx:
                    initrd = _g(ctx.dir)
                    if initrd is not None:
                        self.log.debug("found loader initrd at %s", initrd)
                        return self._runInitrdShell(initrd)
                    else:
                        self.log.error("cannot find loader initrd")
                        return 1

        # try to find the loader partition on the same desk as /mnt/flash
        try:
            flashDev = self.check_output(('blkid', '-L', 'FLASH',)).strip()
        except subprocess.CalledProcessError, what:
            flashDev = None
        if flashDev is not None:
            self.log.debug("found flash device hint %s", flashDev)
            loaderDev = flashDev
            while loaderDev and loaderDev[-1] in string.digits:
                loaderDev = loaderDev[:-1]
            loaderDev = loaderDev + '1'
            with open(loaderDev) as fd:
                buf = fd.read(4)
                magic = struct.unpack(">I", buf)[0]
            if magic == Fit.Parser.FDT_MAGIC:
                self.log.debug("found loader device %s", loaderDev)
                return self._runMtdShell(loaderDev)
            else:
                self.log.error("bad FDT signature on %s %x",
                               loaderDev, magic)
                return 1

        # grovel through MTD devices (u-boot)
        pm = ProcMtdParser(log=self.log)
        parts = [p for p in pm.parts if p.label == "sl-boot"]
        if parts:
            part = parts[0]
            self.log.debug("found loader MTD device %s",
                           part.charDevice or part.blockDevice)
            return self._runMtdShell(part.blockDevice)
        elif pm.parts:
            self.log.error("cannot find loader MTD device")
            return 1

        self.log.error("cannot find loader initrd")
        return 1

main = Onie.main

if __name__ == "__main__":
    main()
