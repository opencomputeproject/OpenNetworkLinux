"""BaseInstall.py

Base classes for installers.
"""

import os, stat
import subprocess
import re
import tempfile
import logging
import StringIO
import parted
import yaml

from InstallUtils import MountContext, BlkidParser, PartedParser, SubprocessMixin
import onl.YamlUtils

class Base:

    class installmeta:
        def __init__(self,
                     installerConf=None,
                     machineConf=None,
                     platformConf=None,
                     grubEnv=None, ubootEnv=None):
            self.installerConf = installerConf
            self.machineConf = machineConf
            self.platformConf = platformConf
            self.grubEnv = grubEnv
            self.ubootEnv = ubootEnv

        def isOnie(self):
            if self.machineConf is None: return False
            plat = getattr(self.machineConf, 'onie_platform', None)
            return plat is not None

    def __init__(self,
                 machineConf=None, installerConf=None, platformConf=None,
                 grubEnv=None, ubootEnv=None,
                 log=None):
        self.machineConf = machineConf
        self.installerConf = installerConf
        self.im = self.installmeta(installerConf=installerConf,
                                   machineConf=machineConf,
                                   platformConf=platformConf,
                                   grubEnv=grubEnv,
                                   ubootEnv = ubootEnv)
        self.grubEnv = grubEnv
        self.ubootEnv = ubootEnv
        self.log = log or logging.getLogger(self.__class__.__name__)

    def run(self):
        self.log.error("not implemented")
        return 1

    def shutdown(self):
        pass

    def installSwi(self):

        swis = [x for x in os.listdir(self.installerConf.installer_dir) if x.endswith('.swi')]
        if not swis:
            self.log.info("No ONL Software Image available for installation.")
            self.log.info("Post-install ZTN installation will be required.")
            return
        if len(swis) > 1:
            self.log.warn("Multiple SWIs found in installer: %s", " ".join(swis))
            return

        base = swis[0]
        src = os.path.join(self.installerConf.installer_dir, base)

        self.log.info("Installing ONL Software Image (%s)...", base)
        dev = self.blkidParts['ONL-IMAGES']
        with MountContext(dev.device, log=self.log) as ctx:
            dst = os.path.join(ctx.dir, base)
            self.copy2(src, dst)

        return 0

    def backupConfig(self, dev):
        """Back up the ONL-CONFIG partition for later restore."""
        self.configArchive = tempfile.mktemp(prefix="onl-config-",
                                             suffix=".tar.gz")
        self.log.info("backing up ONL-CONFIG partition %s to %s",
                      dev, self.configArchive)
        with MountContext(dev, log=self.log) as ctx:
            self.log.debug("+ tar -zcf %s -C %s .",
                           self.configArchive, ctx.dir)
            pipe = subprocess.Popen(["tar", "-zcf", self.configArchive, ".",],
                                    cwd=ctx.dir)
            pipe.communicate()
            code = pipe.wait()
        if code:
            raise SystemExit("backup of ONL-CONFIG failed")

    def restoreConfig(self, dev):
        """Restore the saved ONL-CONFIG."""
        archive, self.configArchive = self.configArchive, None
        self.log.info("restoring ONL-CONFIG archive %s to %s",
                      archive, dev)
        with MountContext(dev, log=self.log) as ctx:
            self.log.debug("+ tar -zxf %s -C %s",
                           archive, ctx.dir)
            pipe = subprocess.Popen(["tar", "-zxf", archive,],
                                    cwd=ctx.dir)
            pipe.communicate()
            code = pipe.wait()
        if code:
            raise SystemExit("backup of ONL-CONFIG failed")
        self.unlink(archive)

GRUB_TPL = """\
#serial --port=0x3f8 --speed=115200 --word=8 --parity=no --stop=1
serial %(serial)s
terminal_input serial
terminal_output serial
set timeout=5

menuentry OpenNetworkLinux {
  search --no-floppy --label --set=root ONL-BOOT
  echo 'Loading Open Network Linux ...'
  insmod gzio
  insmod part_msdos
  #linux /kernel-3.9.6-x86-64-all nopat console=ttyS0,115200n8 onl_platform=x86-64-kvm-x86-64-r0
  linux /%(kernel)s %(args)s onl_platform=%(platform)s
  initrd /%(initrd)s
}

# Menu entry to chainload ONIE
menuentry ONIE {
  search --no-floppy --label --set=root ONIE-BOOT
  echo 'Loading ONIE ...'
  chainloader +1
}
"""

class GrubInstaller(SubprocessMixin, Base):
    """Installer for grub-based systems (x86)."""

    class installmeta(Base.installmeta):
        grub = True

    def __init__(self, *args, **kwargs):
        Base.__init__(self, *args, **kwargs)

        self.device = None
        self.minpart = None
        self.nextBlock = None

        self.blkidParts = []

        self.partedDevice = None
        self.partedDisk = None

        self.configArchive = None
        # backup of ONL-CONFIG during re-partitioning

    def findGpt(self):
        self.blkidParts = BlkidParser(log=self.log.getChild("blkid"))

        deviceOrLabel = self.im.platformConf['grub']['device']
        if deviceOrLabel.startswith('/dev'):
            tgtDevice, tgtLabel = deviceOrLabel, None
        else:
            tgtDevice, tgtLabel = None, deviceOrLabel

        # enumerate labeled partitions to try to identify
        # the boot device
        for part in self.blkidParts:
            dev, partno = part.splitDev()
            if tgtLabel is not None and tgtLabel == part.label:
                if not len(partno):
                    self.log.error("cannot use whole disk")
                    return 1
                if self.device is None:
                    self.device = dev
                else:
                    self.log.error("found multiple devices: %s, %s",
                                   dev, self.device)
                    return 1
            elif tgtDevice is not None and tgtDevice == dev:
                if not len(partno):
                    self.log.error("cannot use whole disk")
                    return 1
                if self.device is None:
                    self.device = dev
                else:
                    self.log.error("found multiple devices: %s, %s",
                                   dev, self.device)
                    return 1
        if self.device is None:
            self.log.error("cannot find an install device")
            return 1

        # optionally back up a config partition
        # if it's on the boot device
        for part in self.blkidParts:
            dev, partno = part.splitDev()
            if dev == self.device and part.label == 'ONL-CONFIG':
                self.backupConfig(part.device)

        self.partedDevice = parted.getDevice(self.device)
        self.partedDisk = parted.newDisk(self.partedDevice)

        # enumerate the partitions that will stay and go
        minpart = -1
        for part in self.partedDisk.partitions:

            if part.getFlag(parted.PARTITION_HIDDEN):
                minpart = max(minpart, part.number+1)
                continue

            # else, the partition should exist
            blkidParts = [x for x in self.blkidParts if x.device == part.path]
            if not blkidParts:
                self.log.warn("cannot identify partition %s", part)
                continue

            blkidPart = blkidParts[0]
            if not blkidPart.isOnieReserved(): continue

            # else, check the GPT label for reserved-ness
            if (part.name
                and ('GRUB' in part.name
                     or 'ONIE-BOOT' in part.name
                     or 'DIAG' in part.name)):
                minpart = max(minpart, part.number+1)

        if minpart < 0:
            self.log.error("cannot find an install partition")
            return 1
        self.minpart = minpart

        return 0

    def deletePartitions(self):

        nextBlock = -1
        for part in self.partedDisk.partitions:
            self.log.info("examining %s part %d",
                          self.partedDisk.device.path, part.number)
            if part.number < self.minpart:
                self.log.info("skip this part")
                nextBlock = max(part.geometry.start+part.geometry.length,
                                nextBlock)
            else:
                self.log.info("deleting this part")
                self.partedDisk.removePartition(part)

        if nextBlock < 0:
            self.log.error("cannot find a starting block")
            return 1

        self.nextBlock = nextBlock
        return 0

    def partitionGpt(self):

        constraint = self.partedDevice.optimalAlignedConstraint
        # default partition layout constraint

        devices = {}

        def _u2s(sz, u):
            bsz = sz * u
            bsz = bsz + self.partedDevice.physicalSectorSize - 1
            return bsz / self.partedDevice.physicalSectorSize

        UNITS = {
            'GiB' : 1024 * 1024 * 1024,
            'G' : 1000 * 1000 * 1000,
            'MiB' : 1024 * 1024,
            'M' : 1000 * 1000,
            'KiB' : 1024,
            'K' : 1000,
        }

        for part in self.im.platformConf['installer']:

            label, sz = list(part.items())[0]
            if type(sz) == dict:
                sz, fmt = sz['='], sz.get('format', 'ext4')
            else:
                fmt = 'ext4'

            cnt = None
            for ul, ub in UNITS.items():
                if sz.endswith(ul):
                    cnt = _u2s(int(sz[:-len(ul)], 10), ub)
                    break
            if sz == '100%':
                cnt = self.partedDevice.getLength() - self.nextBlock
            if cnt is None:
                self.log.error("invalid size (no units) for %s: %s",
                               part, sz)
                return 1

            start = self.nextBlock
            end = start + cnt - 1
            if end <= self.partedDevice.getLength():
                self.log.info("Allocating %d sectors for %s",
                              cnt, label)
            else:
                self.log.warn("%s: start sector %d, end sector %d, max %d",
                              label, start, end,
                              self.partedDevice.getLength())
                self.log.error("invalid partition %s [%s] (too big)",
                               label, sz)
                return 1

            geom = parted.Geometry(device=self.partedDevice,
                                   start=start, length=end-start+1)
            fs = parted.FileSystem(type=fmt, geometry=geom)
            part = parted.Partition(disk=self.partedDisk,
                                    type=parted.PARTITION_NORMAL,
                                    fs=fs,
                                    geometry=geom)
            part.getPedPartition().set_name(label)
            self.partedDisk.addPartition(part, constraint=constraint)
            self.partedDisk.commit()

            self.log.info("Formatting %s (%s) as %s",
                          part.path, label, fmt)
            if fmt == 'msdos':
                cmd = ('mkdosfs', '-n', label, part.path,)
            else:
                cmd = ('mkfs.%s' % fmt, '-L', label, part.path,)
            self.check_call(cmd, vmode=self.V1)

            self.nextBlock, self.minpart = end+1, self.minpart+1

            devices[label] = part.path

            if label == 'ONL-CONFIG' and self.configArchive is not None:
                self.restoreConfig(part.path)

        self.blkidParts = BlkidParser(log=self.log.getChild("blkid"))
        # re-read the partitions

        return 0

    def installBootConfig(self):
        dev = self.blkidParts['ONL-BOOT']
        self.log.info("Installing boot-config to %s", dev.device)

        src = os.path.join(self.installerConf.installer_dir, 'boot-config')
        with MountContext(dev.device, log=self.log) as ctx:
            dst = os.path.join(ctx.dir, 'boot-config')
            self.copy2(src, dst)

        with open(src) as fd:
            ecf = fd.read().encode('base64', 'strict').strip()
            setattr(self.grubEnv, 'boot_config_default', ecf)

        return 0

    def installLoader(self):

        ctx = {}

        kernel = self.im.platformConf['grub']['kernel']
        ctx['kernel'] = kernel['='] if type(kernel) == dict else kernel

        initrd = self.im.platformConf['grub']['initrd']
        ctx['initrd'] = initrd['='] if type(initrd) == dict else initrd

        ctx['args'] = self.im.platformConf['grub']['args']
        ctx['platform'] = self.installerConf.installer_platform
        ctx['serial'] = self.im.platformConf['grub']['serial']

        cf = GRUB_TPL % ctx

        self.log.info("Installing kernel")
        dev = self.blkidParts['ONL-BOOT']
        with MountContext(dev.device, log=self.log) as ctx:
            def _cp(b):
                src = os.path.join(self.installerConf.installer_dir, b)
                if not os.path.isfile(src): return
                if b.startswith('kernel-') or b.startswith('onl-loader-initrd-'):
                    dst = os.path.join(ctx.dir, b)
                    self.copy2(src, dst)
            [_cp(e) for e in os.listdir(self.installerConf.installer_dir)]

            d = os.path.join(ctx.dir, "grub")
            self.makedirs(d)
            dst = os.path.join(ctx.dir, 'grub/grub.cfg')
            with open(dst, "w") as fd:
                fd.write(cf)

        return 0

    def installGrub(self):
        self.log.info("Installing GRUB to %s", self.partedDevice.path)
        self.grubEnv.install(self.partedDevice.path)
        return 0

    def installGpt(self):

        code = self.findGpt()
        if code: return code

        self.log.info("Installing to %s starting at partition %d",
                      self.device, self.minpart)

        self.log.info("disk is %s", self.partedDevice.path)

        if self.partedDisk.type != 'gpt':
            self.log.error("not a GPT partition table")
            return 1
        if self.partedDevice.sectorSize != 512:
            self.log.error("invalid logical block size")
            return 1
        if self.partedDevice.physicalSectorSize != 512:
            self.log.error("invalid physical block size")
            return 1

        self.log.info("found a disk with %d blocks",
                      self.partedDevice.getLength())

        code = self.deletePartitions()
        if code: return code

        self.log.info("next usable block is %s", self.nextBlock)

        code = self.partitionGpt()
        if code: return code

        # once we assign the ONL-BOOT partition,
        # we can re-target the grub environment
        dev = self.blkidParts['ONL-BOOT']
        self.grubEnv.__dict__['bootPart'] = dev.device
        self.grubEnv.__dict__['bootDir'] = None

        code = self.installSwi()
        if code: return code

        code = self.installLoader()
        if code: return code

        code = self.installBootConfig()
        if code: return code

        code = self.installGrub()
        if code: return code

        self.log.info("ONL loader install successful.")
        self.log.info("GRUB installation is required next.")

        return 0

    def run(self):
        if 'grub' not in self.im.platformConf:
            self.log.error("platform config is missing a GRUB section")
            return 1
        label = self.im.platformConf['grub'].get('label', None)
        if label != 'gpt':
            self.log.error("invalid GRUB label in platform config: %s", label)
            return 1
        return self.installGpt()

    def shutdown(self):
        pass

class UbootInstaller(SubprocessMixin, Base):

    class installmeta(Base.installmeta):

        device = None
        uboot = True

        loaderBlocks = None
        flashBlocks = None
        flash2Blocks = None
        # block count, or -1 for "rest"

        loaderRaw = True
        # true for raw loader partition (FIT image)

        loaderSrc = None
        # default loader source file (auto-detect)

        loaderDst = "onl-loader"
        # destination path on ONL-BOOT for non-raw installs

        bootConf = None
        # optional pre-formatted boot-config contents
        # (string or list of strings)

        bootCmd = None
        # pre-formatted string

        bootCmds = None
        # ... or a list of strings

        def str_bootcmd(self):
            if self.bootCmd is not None: return self.bootCmd
            if self.bootCmds:
                return "; ".join(self.bootCmds)
            raise ValueError("missing boot commands")

    def __init__(self, *args, **kwargs):
        kwargs = dict(kwargs)
        installerConf = kwargs.pop('installerConf', None)
        machineConf = kwargs.pop('machineConf', None)
        platformConf = kwargs.pop('platformConf', None)
        ubootEnv = kwargs.pop('ubootEnv', None)
        self.im = self.installmeta(installerConf=installerConf,
                                   machineConf=machineConf,
                                   platformConf=platormConf,
                                   ubootEnv=ubootEnv)
        Base.__init__(self, *args,
                      machineConf=machineConf, installerConf=installerConf, platformConf=platformConf,
                      ubootEnv=ubootEnv,
                      **kwargs)

        # XXX roth
        self.onlBootDev = None
        self.onlConfigDev = None
        self.onlDataDev = None

    def formatBlockdev(self):

        if self.im.loaderBlocks < 0:
            self.log.error("no size defined for ONL-BOOT")
            return 1
        if self.im.flashBlocks < 0:
            self.log.error("no size defined for FLASH")
            return 1

        self.log.info("Formatting %s as %d:%d:%d",
                      self.im.device,
                      self.im.loaderBlocks,
                      self.im.flashBlocks,
                      self.im.flash2Blocks)

        self.check_call(('parted', '-s', self.im.device,
                         'mklabel', 'msdos',))

        start = 1
        end = start + self.im.loaderBlocks-1

        self.check_call(('parted', '-s', self.im.device,
                         'unit', 's',
                         'mkpart', 'primary', 'fat32', str(start)+'s', str(end)+'s',))

        self.onlBootDev = self.im.device + '1'
        if not self.im.loaderRaw:
            cmd = ('mkdosfs', '-n', 'ONL-BOOT', self.onlBootDev,)
            self.check_call(cmd, vmode=self.V1)

        start = end + 1
        end = start + self.im.flashBlocks-1

        self.check_call(('parted', '-s', self.im.device,
                         'unit', 's',
                         'mkpart', 'primary', 'fat32', str(start)+'s', str(end)+'s',))

        self.onlConfigDev = self.im.device + '2'
        cmd = ('mkdosfs', '-n', 'FLASH', self.onlConfigDev,)
        self.check_call(cmd, vmode=self.V1)

        start = end + 1
        if self.im.flash2Blocks > -1:
            end = start + self.im.flash2Blocks-1
            self.check_call(('parted', '-s', self.im.device,
                             'unit', 's',
                             'mkpart', 'primary', 'fat32', str(start)+'s', str(end)+'s',))
        else:
            self.check_call(('parted', '-s', self.im.device,
                             'unit', 's',
                             'mkpart', 'primary', 'fat32', str(start)+'s', '100%',))

        self.onlDataDev = self.im.device + '3'
        cmd = ('mkdosfs', '-n', 'FLASH2', self.onlDataDev,)
        self.check_call(cmd, vmode=self.V1)

        return 0

    def installLoader(self):

        loaderSrc = None
        for cand in (("%s/%s.itb"
                      % (self.installerConf.installer_dir,
                         self.installerConf.installer_platform,)),
                     os.path.join(self.installerConf.installer_dir, 'powerpc-fit-all.itb'),
                     self.im.loaderSrc,
                     ("%s/onl.%s.loader"
                      % (self.installerConf.installer_dir,
                         self.installerConf.installer_platform,))):
            if os.path.exists(cand):
                loaderSrc = cand
                break
        if not loaderSrc:
            self.log.error("The platform loader file is missing.")
            self.log.error("This is unexpected - %s", loaderSrc)
            return 1

        self.log.info("Installing the ONL loader...")

        if self.im.loaderRaw:
            cmd = ('dd',
                   'if=' + loaderSrc,
                   'of=' + self.onlBootDev,)
            self.check_call(cmd, vmode=self.V2)
        else:
            with MountContext(self.onlBootDev, log=self.log) as ctx:
                dst = os.path.join(ctx, self.im.loaderDst)
                self.copy2(loaderSrc, dst)

        return 0

    def installBootconfig(self):

        cf = None

        p = os.path.join(self.installerConf.installer_dir, 'boot-config')
        if cf is None and os.path.exists(p):
            cf = open(p, "r").read()

        p = os.path.join(self.installerConf.installer_platform_dir, 'boot-config')
        if cf is None and os.path.exists(p):
            cf = open(p, "r").read()

        if cf is None and self.im.bootConf:
            if isinstance(self.im.bootConf, basestring):
                cf = self.im.bootConf
            else:
                cf = "\n".join(cf) + "\n"

        if cf is None:
            buf = StringIO.StringIO()
            buf.write("SWI=images:onl-%s.swi\n"
                      % (self.platformConf.installer_arch,))
            buf.write("NETDEV=ma1\n")
            cf = buf.getvalue()

        self.log.info("Writing boot-config.")
        with MountContext(self.onlConfigDev, log=self.log) as ctx:
            dst = os.path.join(ctx.dir, "boot-config")
            with open(dst, "w") as fd:
                fd.write(cf)

        ecf = cf.encode('base64', 'strict').strip()
        setattr(self.ubootEnv, 'boot-config-default', ecf)

        return 0

    def installUbootEnv(self):

        # Special access instructions for initrd
        off = getattr(self.installerConf, 'initrd_offset', None)
        if off is not None:
            if self.im.loaderRaw:
                a = self.onlBootDev
            else:
                a = self.installerConf.initrd_archive
            s = int(self.installerConf.initrd_offset)
            e = s + int(self.installerConf.initrd_size) - 1
            self.ubootEnv.onl_installer_initrd = ("%s:%x:%x" % (a, s, e,))
        else:
            try:
                del self.installerConf.onl_installer_initrd
            except AttributeError:
                pass

        if self.im.isOnie():
            self.log.info("Setting ONIE nos_bootcmd to boot ONL")
            self.ubootEnv.nos_bootcmd = self.im.str_bootcmd()
        else:
            self.log.warn("U-boot boot setting is not changed")

        return 0

    def installUboot(self):

        st = os.stat(self.im.device)
        if not stat.S_ISBLK(st[stat.ST_MODE]):
            self.log.error("not a block device: %s",
                           self.im.device)
            return 1

        code = self.formatBlockdev()
        if code: return code

        code = self.installLoader()
        if code: return code

        code = self.installBootconfig()
        if code: return code

        code = self.installSwi()
        if code: return code

        self.log.info("syncing block devices")
        self.check_call(('sync',))
        # XXX roth probably not needed

        code = self.installUbootEnv()
        if code: return code

        return 0

    def run(self):
        return self.installUboot()

    def shutdown(self):
        pass
