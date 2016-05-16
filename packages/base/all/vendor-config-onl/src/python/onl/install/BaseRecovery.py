"""BaseRecovery.py

Base classes for recovery.
"""

import subprocess, os, stat
import tempfile
import binascii
import glob
import logging
from InstallUtils import TempdirContext, MountContext, SubprocessMixin, ProcMountsParser
from InstallUtils import InitrdContext, BlkidParser
from ConfUtils import ChrootGrubEnv

class Base(SubprocessMixin):

    class recovermeta:

        bootConfig = "/mnt/flash/boot-config"
        bootConfigDfl = "/etc/boot-config.default"

        @property
        def needRecovery(self):
            if os.path.exists('/mnt/flash/.notmounted'): return True
            if os.path.exists('/mnt/flash2/.notmounted'): return True
            return False

    def __init__(self,
                 ubootEnv=None,
                 log=None):
        self.platform = self.recovermeta()
        self.ubootEnv = ubootEnv
        self.log = log or logging.getLogger(self.__class__.__name__)

    def recoverFull(self):
        self.log.error("not implemented")
        return 1

    def recoverConfig(self):
        if os.path.exists(self.platform.bootConfig): return 0
        self.copy2(self.platform.bootConfigDfl, self.platform.bootConfig)
        return 0

    def run(self):

        if self.platform.needRecovery:
            self.log.info("Attempting recovery")
            code = self.recoverFull()
            if code: return code

        code = self.recoverConfig()
        if code: return code

        return 0

    def umountAny(self, device=None, label=None):
        p = ProcMountsParser()
        if label is not None:
            b = BlkidParser(log=self.log)
            for e in b.parts:
                if label == e.label:
                    device = e.device
                    break

        for m in p.mounts:
            if device is not None and device in m.device:
                try:
                    self.check_call(('umount', m.device,),
                                    vmode=self.V1)
                except CalledProcessError, what:
                    self.log.warn("cannot umount %s: %s",
                                  m.device, str(what))
        return 0

    def shutdown(self):
        pass

class GrubRecovery(Base):

    class recovermeta(Base.recovermeta):
        pass

    def recoverX86(self):

        def _u(l):
            self.umountAny(label=l)
        def _l(l):
            try:
                return self.check_output(('blkid', '-L', l,)).strip()
            except subprocess.CalledProcessError:
                return None
        def _r(l):
            _u(l)
            dev = _l(l)
            if dev is not None:
                self.log.info("Recovering %s partition", l)
                self.check_call(('mkdosfs', '-n', l, dev,),
                                vmode=self.V1)

        _r('FLASH')
        _r('FLASH2')

        return 0

    def recoverGrubConfig(self):

        with MountContext(label='ONIE-BOOT', log=self.log) as octx:

            pat = "%s/onie/initrd.img*" % octx.dir
            l = glob.glob(pat)
            if not l:
                raise ValueError("cannot find ONIE initrd")
            initrd = l[0]

            with InitrdContext(initrd=initrd, log=self.log) as ictx:

                # copy the Switch Light grubenv out of its GRUB directory
                dst = os.path.join(ictx.dir, "tmp/grubenv")
                with MountContext(label='SL-BOOT', log=self.log) as sctx:
                    src = os.path.join(sctx.dir, "grub/grubenv")
                    self.copy2(src, dst)

                # use the ONIE runtime's GRUB tools to read it
                grubEnv = ChrootGrubEnv(ictx.dir, mounted=True,
                                        bootDir="/",
                                        path="/tmp/grubenv",
                                        log=self.log)
                buf = getattr(grubEnv, 'boot_config_default', None)

        if buf is None:
            raise ValueError("Cannot recover filesystem(s) -- missing boot_config_default.")
        if buf == "":
            raise ValueError("Cannot recover filesystem(s) -- empty boot_config_default.")
        try:
            buf = buf.decode('base64', 'strict')
        except binascii.Error:
            raise ValueError("Cannot recover filesystem(s) -- corrupted boot_config_default.")
        if "SWI=flash" in buf:
            raise ValueError("Cannot recover filesystem(s) -- local SWI cannot be recovered.")

        with MountContext(label='FLASH', log=self.log) as ctx:
            dst = os.path.join(ctx.dir, 'boot-config')
            with open(dst, "w") as fd:
                self.log.debug("+ cat > %s", dst)
                fd.write(buf)

        return 0

    def recoverFull(self):
        self.log.info("Recovering flash partitions.")

        code = self.recoverX86()
        if code: return code

        code = self.recoverGrubConfig()
        if code: return code

        self.check_call(('initmounts',))

        return 0

class UbootRecovery(Base):

    class recovermeta(Base.recovermeta):

        def __init__(self, ubootEnv=None):
            self.ubootEnv = ubootEnv

        device = None
        # fill this in per-platform

        @property
        def bootConfigEnv(self):
            if self.ubootEnv is None:
                raise ValueError("missing u-boot environment tools")
            buf = getattr(self.ubootEnv, 'boot-config-default', None)
            if buf is None:
                raise ValueError("Cannot recover filesystem(s) -- missing boot-config-default.")
            if buf == "":
                raise ValueError("Cannot recover filesystem(s) -- empty boot-config-default.")
            try:
                buf = buf.decode('base64', 'strict')
            except binascii.Error:
                raise ValueError("Cannot recover filesystem(s) -- corrupted boot-config-default.")
            if "SWI=flash" in buf:
                raise ValueError("Cannot recover filesystem(s) -- local SWI cannot be recovered.")
            return buf

    def __init__(self,
                 ubootEnv=None,
                 log=None):
        self.ubootEnv = ubootEnv
        self.platform = self.recovermeta(ubootEnv=ubootEnv)
        self.log = log or logging.getLogger(self.__class__.__name__)

        self.flashDev = self.platform.device + '2'
        self.flash2Dev = self.platform.device + '3'

    def recoverUboot(self):
        if not os.path.exists(self.platform.device):
            self.log.error("missing block device, cannot recover")
            return 1
        st = os.stat(self.platform.device)
        if not stat.S_ISBLK(st[stat.ST_MODE]):
            self.log.error("invalid block device")
            return 1

        code = self.umountAny(device=self.platform.device)
        if code: return code

        self.log.info("Re-formatting %s", self.platform.device)
        cmd = ('mkdosfs', '-n', 'FLASH', self.flashDev,)
        self.check_call(cmd, vmode=self.V1)
        cmd = ('mkdosfs', '-n', 'FLASH2', self.flash2Dev,)
        self.check_call(cmd, vmode=self.V1)
        return 0

    def recoverUbootConfig(self):
        with MountContext(self.flashDev, log=self.log) as ctx:
            dst = os.path.join(ctx.dir, 'boot-config')
            with open(dst, "w") as fd:
                self.log.debug("+ cat > %s", dst)
                fd.write(self.platform.bootConfigEnv)
        return 0

    def recoverFull(self):

        code = self.recoverUboot()
        if code: return code

        self.recoverUbootConfig()
        if code: return code

        self.log.info("syncing block devices")
        self.check_call(('sync',))
        # XXX roth probably not needed

        self.check_call(('initmounts',))

        return 0
