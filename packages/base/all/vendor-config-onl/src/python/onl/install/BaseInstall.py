"""BaseInstall.py

Base classes for installers.
"""

import os, sys, stat
import subprocess
import re
import tempfile
import logging
import StringIO
import parted
import yaml
import zipfile
import shutil
import imp
import fnmatch, glob

from InstallUtils import SubprocessMixin
from InstallUtils import MountContext, BlkidParser, PartedParser, UbinfoParser
from InstallUtils import ProcMountsParser
from Plugin import Plugin

import onl.YamlUtils
from onl.sysconfig import sysconfig

try:
    PartedException = parted._ped.PartedException
    DiskException = parted._ped.DiskException
except AttributeError:
    import _ped
    PartedException = _ped.PartedException
    DiskException = _ped.DiskException

class Base:

    class installmeta:

        grub = False
        uboot = False

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
                 force=False,
                 log=None):
        self.im = self.installmeta(installerConf=installerConf,
                                   machineConf=machineConf,
                                   platformConf=platformConf,
                                   grubEnv=grubEnv,
                                   ubootEnv = ubootEnv)
        self.log = log or logging.getLogger(self.__class__.__name__)

        self.force = force
        # unmount filesystems as needed

        self.device = None
        # target device, initialize this later

        self.minpart = None
        self.nextBlock = None
        # keep track of next partition/next block

        self.blkidParts = []
        self.ubiParts = []
        # current scan of partitions and labels

        self.partedDevice = None
        self.partedDisk = None
        # parted state

        self.configArchive = None
        # backup of ONL-CONFIG during re-partitioning

        self.zf = None
        # zipfile handle to installer archive

        self.plugins = []
        # dynamically-detected plugins

    def run(self):
        self.log.error("not implemented")
        return 1

    def upgradeBootLoader(self):
        self.log.error("not implemented")
        return 1

    def shutdown(self):

        plugins, self.plugins = self.plugins, []
        for plugin in plugins:
            plugin.shutdown()

        zf, self.zf = self.zf, None
        if zf: zf.close()

    def installerCopy(self, basename, dst, optional=False):
        """Copy the file as-is, or get it from the installer zip."""

        src = os.path.join(self.im.installerConf.installer_dir, basename)
        if os.path.exists(src):
            self.copy2(src, dst)
            return True

        if basename in self.zf.namelist():
            self.log.debug("+ unzip -p %s %s > %s",
                           self.im.installerConf.installer_zip, basename, dst)
            with self.zf.open(basename, "r") as rfd:
                with open(dst, "wb") as wfd:
                    shutil.copyfileobj(rfd, wfd)
            return True

        if not optional:
            raise ValueError("missing installer file %s" % basename)

        return False

    def installerDd(self, basename, device):

        p = os.path.join(self.im.installerConf.installer_dir, basename)
        if os.path.exists(p):
            cmd = ('dd',
                   'if=' + basename,
                   'of=' + device,)
            self.check_call(cmd, vmode=self.V2)
            return

        if basename in self.zf.namelist():
            self.log.debug("+ unzip -p %s %s | dd of=%s",
                           self.im.installerConf.installer_zip, basename, device)
            with self.zf.open(basename, "r") as rfd:
                with open(device, "rb+") as wfd:
                    shutil.copyfileobj(rfd, wfd)
            return

        raise ValueError("cannot find file %s" % basename)

    def installerExists(self, basename):
        if basename in os.listdir(self.im.installerConf.installer_dir): return True
        if basename in self.zf.namelist(): return True
        return False

    def installSwi(self):

        files = os.listdir(self.im.installerConf.installer_dir) + self.zf.namelist()
        swis = [x for x in files if x.endswith('.swi')]
        if not swis:
            self.log.info("No ONL Software Image available for installation.")
            self.log.info("Post-install ZTN installation will be required.")
            return
        if len(swis) > 1:
            self.log.warn("Multiple SWIs found in installer: %s", " ".join(swis))
            return

        base = swis[0]

        self.log.info("Installing ONL Software Image (%s)...", base)
            
        dev = self.blkidParts['ONL-IMAGES']
        with MountContext(dev.device, log=self.log) as ctx:
            dst = os.path.join(ctx.dir, base)
            self.installerCopy(base, dst)

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

    def deletePartitions(self):

        nextBlock = -1
        dirty = False
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
                dirty = True

        if dirty:
            self.partedDisk.commit()
            self.check_call(('partprobe', self.device,))

        if nextBlock > -1:
            self.nextBlock = nextBlock
        else:
            self.log.warn("no partitions, starting at block 1")

        return 0

    def partitionParted(self):
        """Build partitions according to the partition spec.

        XXX roth -- hopefully the GPT labels specified here
        work correctly (that is, are ignored) on an msdos label
        """

        constraint = self.partedDevice.optimalAlignedConstraint
        # default partition layout constraint

        devices = {}

        def _u2s(sz, u):
            """Convert to units of logical sectors."""
            bsz = sz * u
            bsz = bsz + self.partedDevice.sectorSize - 1
            return bsz / self.partedDevice.sectorSize

        def _align(spos):
            """Align this sector number."""
            sz = self.partedDevice.sectorSize
            psz = self.partedDevice.physicalSectorSize

            if type(psz) != int: return spos
            if psz <= sz: return spos
            if (psz % sz) != 0: return spos
            if psz > 1048576: return spos

            stride = psz / sz
            off = spos % stride
            off = stride - off
            off = off % stride

            spos += off
            return spos

        UNITS = {
            'GiB' : 1024 * 1024 * 1024,
            'G' : 1000 * 1000 * 1000,
            'MiB' : 1024 * 1024,
            'M' : 1000 * 1000,
            'KiB' : 1024,
            'K' : 1000,
        }

        for part in self.im.platformConf['installer']:

            label, partData = list(part.items())[0]
            if type(partData) == dict:
                sz, fmt = partData['='], partData.get('format', 'ext4')
            else:
                sz, fmt = partData, 'ext4'

            cnt = None
            nextBlock = self.nextBlock or 1
            minpart = self.minpart or 1
            for ul, ub in UNITS.items():
                if sz.endswith(ul):
                    cnt = _u2s(int(sz[:-len(ul)], 10), ub)
                    break
            if sz == '100%':
                cnt = self.partedDevice.getLength() - nextBlock
            if cnt is None:
                self.log.error("invalid size (no units) for %s: %s",
                               part, sz)
                return 1

            start = _align(nextBlock)
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
            if self.partedDisk.type == 'gpt':
                part.getPedPartition().set_name(label)
            self.partedDisk.addPartition(part, constraint=constraint)
            self.partedDisk.commit()
            self.check_call(('partprobe', self.device,))

            if fmt == 'raw':
                self.log.info("Leaving %s (%s) unformatted (raw)",
                              part.path, label)
            else:
                self.log.info("Formatting %s (%s) as %s",
                              part.path, label, fmt)
                if fmt == 'msdos':
                    self.mkdosfs(part.path, label=label)
                elif fmt == 'ext4':
                    self.mke4fs(part.path, label=label, huge_file=False)
                elif fmt == 'ext2':
                    self.mke2fs(part.path, label=label)
                else:
                    self.mkfs(part.path, fstype=fmt)

            self.nextBlock, self.minpart = end+1, minpart+1

            devices[label] = part.path

            if label == 'ONL-CONFIG' and self.configArchive is not None:
                self.restoreConfig(part.path)

        self.blkidParts = BlkidParser(log=self.log.getChild("blkid"))
        # re-read the partitions

        return 0

    def installBootConfig(self):

        try:
            dev = self.blkidParts['ONL-BOOT']
        except IndexError as ex:
            self.log.warn("cannot find ONL-BOOT partition (maybe raw?) : %s", str(ex))
            return 1

        self.log.info("Installing boot-config to %s", dev.device)

        basename = 'boot-config'
        with MountContext(dev.device, log=self.log) as ctx:
            dst = os.path.join(ctx.dir, basename)

            if not self.installerCopy(basename, dst, True):
                return

            with open(dst) as fd:
                buf = fd.read()

        ecf = buf.encode('base64', 'strict').strip()
        if self.im.grub and self.im.grubEnv is not None:
            setattr(self.im.grubEnv, 'boot_config_default', ecf)
        if self.im.uboot and self.im.ubootEnv is not None:
            setattr(self.im.ubootEnv, 'boot-config-default', ecf)

        return 0

    def installOnlConfig(self):

        try:
            dev = self.blkidParts['ONL-CONFIG']
        except IndexError as ex:
            self.log.warn("cannot find ONL-CONFIG partition : %s", str(ex))
            return 1

        with MountContext(dev.device, log=self.log) as ctx:
            for f in self.zf.namelist():
                d = 'config/'
                if f.startswith(d) and f != d:
                    dst = os.path.join(ctx.dir, os.path.basename(f))
                    if not os.path.exists(dst):
                        self.installerCopy(f, dst)

        return 0

    def assertUnmounted(self):
        """Make sure the install device does not have any active mounts."""
        pm = ProcMountsParser()
        for m in pm.mounts:
            if m.device.startswith(self.device):
                if not self.force:
                    self.log.error("mount %s on %s will be erased by install (try --force)",
                                   m.dir, m.device)
                    return 1
                else:
                    self.log.warn("unmounting %s from %s (--force)",
                                  m.dir, m.device)
                    try:
                        self.check_call(('umount', m.dir,))
                    except subprocess.CalledProcessError:
                        self.log.error("cannot unmount")
                        return 1

        return 0

    def loadPluginsFromFile(self, pyPath):
        self.log.info("loading plugins from %s", pyPath)
        with open(pyPath) as fd:
            sfx = ('.py', 'U', imp.PY_SOURCE,)
            moduleName = os.path.splitext(os.path.basename(pyPath))[0]
            mod = imp.load_module("onl_install_plugin_%s" % moduleName, fd, pyPath, sfx)
            for attr in dir(mod):
                klass = getattr(mod, attr)
                if isinstance(klass, type) and issubclass(klass, Plugin):
                    self.log.info("%s: found plugin %s", pyPath, attr)
                    plugin = klass(self)
                    self.plugins.append(plugin)

    def loadPlugins(self):

        # scrape any plugins from the installer working directory
        pat = os.path.join(self.im.installerConf.installer_dir, "plugins", "*.py")
        for src in glob.glob(pat):
            self.loadPluginsFromFile(src)

        # scrape any plugins from the installer archive
        pat = "plugins/*.py"
        for basename in self.zf.namelist():
            if not fnmatch.fnmatch(basename, pat): continue
            try:
                src = None
                with self.zf.open(basename, "r") as rfd:
                    wfno, src = tempfile.mkstemp(prefix="plugin-",
                                                 suffix=".py")
                    with os.fdopen(wfno, "w") as wfd:
                        shutil.copyfileobj(rfd, wfd)
                    self.loadPluginsFromFile(src)
            finally:
                if src and os.path.exists(src):
                    os.unlink(src)

        # scrape plugins from the loader runtime
        # (any plugins dropped into $pydir/onl/install/plugins/*.py)
        try:
            import onl.install.plugins
            plugindir = os.path.dirname(onl.install.plugins.__file__)
        except ImportError:
            plugindir = None
        if plugindir:
            pat = os.path.join(plugindir, "*.py")
            for src in glob.glob(pat):
                self.loadPluginsFromFile(src)

        return 0

    def runPlugins(self, mode):
        self.log.info("running plugins: %s", mode)
        for plugin in self.plugins:
            try:
                code = plugin.run(mode)
            except:
                self.log.exception("plugin failed")
                code = 1
            if code: return code
        return 0

GRUB_TPL = """\
serial %(serial)s
terminal_input serial
terminal_output serial
set timeout=5

# Always boot the saved_entry value
load_env
if [ "${saved_entry}" ] ; then
   set default="${saved_entry}"
fi

menuentry %(boot_menu_entry)s {
  search --no-floppy --label --set=root ONL-BOOT
  # Always return to this entry by default.
  set saved_entry="0"
  save_env saved_entry
  echo 'Loading %(boot_loading_name)s ...'
  insmod gzio
  insmod part_msdos
  linux /%(kernel)s %(args)s onl_platform=%(platform)s
  initrd /%(platform)s.cpio.gz
}

# Menu entry to chainload ONIE
menuentry ONIE {
  search --no-floppy --label --set=root ONIE-BOOT
  # Always return to entry 0 by default.
  set saved_entry="0"
  save_env saved_entry
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

        code = self.assertUnmounted()
        if code: return code

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

    def installLoader(self):

        kernels = []
        for f in set(os.listdir(self.im.installerConf.installer_dir) + self.zf.namelist()):
            if 'kernel' in f:
                kernels.append(f)

        initrd = None
        for f in set(os.listdir(self.im.installerConf.installer_dir) + self.zf.namelist()):
            for i in sysconfig.installer.grub:
                if f == i:
                    initrd = i
                    break

        dev = self.blkidParts['ONL-BOOT']

        self.log.info("Installing kernel to %s", dev.device)

        with MountContext(dev.device, log=self.log) as ctx:
            def _cp(b, dstname=None):
                if dstname is None:
                    dstname = b
                dst = os.path.join(ctx.dir, dstname)
                self.installerCopy(b, dst, optional=True)
            [_cp(e) for e in kernels]
            _cp(initrd, "%s.cpio.gz" % self.im.installerConf.installer_platform)

        return 0

    def installGrubCfg(self):

        dev = self.blkidParts['ONL-BOOT']

        self.log.info("Installing grub.cfg to %s", dev.device)

        ctx = {}

        kernel = self.im.platformConf['grub']['kernel']
        ctx['kernel'] = kernel['='] if type(kernel) == dict else kernel
        ctx['args'] = self.im.platformConf['grub']['args']
        ctx['platform'] = self.im.installerConf.installer_platform
        ctx['serial'] = self.im.platformConf['grub']['serial']

        ctx['boot_menu_entry'] = sysconfig.installer.menu_name
        ctx['boot_loading_name'] = sysconfig.installer.os_name

        cf = GRUB_TPL % ctx

        with MountContext(dev.device, log=self.log) as ctx:
            d = os.path.join(ctx.dir, "grub")
            if not os.path.exists(d):
                self.makedirs(d)
            dst = os.path.join(ctx.dir, 'grub/grub.cfg')
            with open(dst, "w") as fd:
                fd.write(cf)

        return 0

    def installGrub(self):
        self.log.info("Installing GRUB to %s", self.partedDevice.path)
        self.im.grubEnv.install(self.partedDevice.path)
        return 0

    def installGpt(self):

        # get a handle to the installer zip
        p = os.path.join(self.im.installerConf.installer_dir,
                         self.im.installerConf.installer_zip)
        self.zf = zipfile.ZipFile(p)

        code = self.loadPlugins()
        if code: return code

        code = self.runPlugins(Plugin.PLUGIN_PREINSTALL)
        if code: return code

        code = self.findGpt()
        if code: return code

        self.log.info("Installing to %s starting at partition %d",
                      self.device, self.minpart)

        self.log.info("disk is %s", self.partedDevice.path)

        self.log.info("found %s partitions (bsz %s, lbsz %s)",
                      self.partedDisk.type,
                      self.partedDevice.sectorSize,
                      self.partedDevice.physicalSectorSize)
        if self.partedDisk.type != 'gpt':
            self.log.error("not a GPT partition table")
            return 1
        if self.partedDevice.sectorSize != 512:
            self.log.warn("invalid logical block size, expected 512")
        if self.partedDevice.physicalSectorSize != 512:
            self.log.warn("invalid physical block size, expected 512")

        self.log.info("found a disk with %d blocks",
                      self.partedDevice.getLength())

        code = self.deletePartitions()
        if code: return code

        self.log.info("next usable block is %s", self.nextBlock)

        code = self.partitionParted()
        if code: return code

        # once we assign the ONL-BOOT partition,
        # we can re-target the grub environment
        dev = self.blkidParts['ONL-BOOT']
        self.im.grubEnv.__dict__['bootPart'] = dev.device
        self.im.grubEnv.__dict__['bootDir'] = None

        code = self.installSwi()
        if code: return code

        code = self.installLoader()
        if code: return code

        code = self.installGrubCfg()
        if code: return code

        code = self.installBootConfig()
        if code: return code

        code = self.installOnlConfig()
        if code: return code

        code = self.installGrub()
        if code: return code

        code = self.runPlugins(Plugin.PLUGIN_POSTINSTALL)
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

    def upgradeBootLoader(self):
        """Upgrade the boot loader settings."""

        self.blkidParts = BlkidParser(log=self.log.getChild("blkid"))

        code = self.installGrubCfg()
        if code: return code

        return 0

    def shutdown(self):
        Base.shutdown(self)

class UbootInstaller(SubprocessMixin, Base):

    class installmeta(Base.installmeta):

        uboot = True

        def getDevice(self):
            loader = self.platformConf.get('loader', {})
            dev = loader.get('device', None)
            return dev

        def str_bootcmd(self):
            cmds = []
            cmds.append("setenv onl_loadaddr 0x%x"
                       % self.platformConf['loader']['loadaddr'])
            cmds.append("setenv onl_platform %s"
                       % self.installerConf.installer_platform)
            itb = "%s.itb" % self.installerConf.installer_platform
            cmds.append("setenv onl_itb %s" % itb)
            for item in self.platformConf['loader']['setenv']:
                k, v = list(item.items())[0]
                device = self.getDevice()
                if "mtdblock" in device:
                    cmds.append("setenv %s %s ${platformargs} ubi.mtd=%s root=/dev/ram ethaddr=$ethaddr" % (k, v, device[-1],))
                else:
                    cmds.append("setenv %s %s" % (k, v,))
            cmds.extend(self.platformConf['loader']['nos_bootcmds'])
            return "; ".join(cmds)

    def __init__(self, *args, **kwargs):
        Base.__init__(self, *args, **kwargs)

        self.device = self.im.getDevice()

        self.rawLoaderDevice = None
        # set to a partition device for raw loader install,
        # default to None for FS-based install

    def maybeCreateUBIfs(self):
        """Set up an UBI file system."""

        self.partedDevice = parted.getDevice(self.device)
        
        UNITS = {
            'GiB' : 1024 * 1024 * 1024,
            'G' : 1000 * 1000 * 1000,
            'MiB' : 1024 * 1024,
            'M' : 1000 * 1000,
            'KiB' : 1024,
            'K' : 1000,
        }
        try:

            code = 0
            if not code:
                mtd_num = self.device[-1]
                cmd = ('ubiformat', '/dev/mtd' + mtd_num)
                self.check_call(cmd, vmode=self.V2)
                cmd = ('ubiattach', '-m', mtd_num, '-d', '0', '/dev/ubi_ctrl',)
                self.check_call(cmd, vmode=self.V2)
                
                for part in self.im.platformConf['installer']:
                    label, partData = list(part.items())[0]
                    if type(partData) == dict: 
                        sz, fmt = partData['='], partData.get('format', 'ubifs')
                    else:
                       sz, fmt = partData, 'ubifs'
                    cnt = None
                    for ul, ub in UNITS.items():
                        if sz.endswith(ul):
                            cnt = int(sz[:-len(ul)], 10) * ub
                            break
                    if cnt is None:
                        self.log.error("invalid size (no units) for %s: %s",
                               part, sz)
                        return 1
                    label = label.strip()
                    cmd = ('ubimkvol', '/dev/ubi0', '-N', label, '-s', bytes(cnt),)
                    self.check_call(cmd, vmode=self.V2)


        except Exception:
            self.log.exception("cannot create UBI file systemfrom %s",
                               self.device)

        return 0
    def maybeCreateLabel(self):
        """Set up an msdos label."""

            
        self.partedDevice = parted.getDevice(self.device)
        try:
            self.partedDisk = parted.newDisk(self.partedDevice)
            if self.partedDisk.type == 'msdos':
                self.log.info("disk %s is already msdos", self.device)
                return 0
            self.log.warn("disk %s has wrong label %s",
                          self.device, self.partedDisk.type)
        except (DiskException, PartedException) as ex:
            self.log.error("cannot get partition table from %s: %s",
                           self.device, str(ex))
        except Exception:
            self.log.exception("cannot get partition table from %s",
                               self.device)

        self.log.info("clobbering disk label on %s", self.device)
        self.partedDevice.clobber()

        self.log.info("creating msdos label on %s", self.device)
        self.partedDisk = parted.freshDisk(self.partedDevice, 'msdos')

        return 0

    def findMsdos(self):
        """Backup any existing data.

        The GPT version of this function is more tricky since it needs
        to save some of the partitions. Here with and msdos label that
        is on a different block device from u-boot or ONIE, we don't
        really care.
        """

        # optionally back up a config partition
        # if it's on the boot device
        for part in self.blkidParts:
            dev, partno = part.splitDev()
            if dev == self.device and part.label == 'ONL-CONFIG':
                self.backupConfig(part.device)

        self.minPart = -1
        # default, delete all partitions
        # XXX roth -- tweak this if we intent to save e.g.
        # a diag partition from the vendor

        return 0

    def installLoader(self):

        loaderBasename = None
        for c in sysconfig.installer.fit:
            if self.installerExists(c):
                loaderBasename = c
                break

        if not loaderBasename:
            self.log.error("The platform loader file is missing.")
            return 1

        self.log.info("Installing the ONL loader from %s...", loaderBasename)

        if self.rawLoaderDevice is not None:
            self.log.info("Installing ONL loader %s --> %s...",
                          loaderBasename, self.rawLoaderDevice)
            self.installerDd(loaderBasename, self.rawLoaderDevice)
            return 0

        dev = self.blkidParts['ONL-BOOT']
        self.log.info("Installing ONL loader %s --> %s:%s...",
                      loaderBasename, dev.device, loaderBasename)
        with MountContext(dev.device, log=self.log) as ctx:
            dst = os.path.join(ctx.dir, "%s.itb" % self.im.installerConf.installer_platform)
            self.installerCopy(loaderBasename, dst)

        return 0

    def installUbootEnv(self):

        # Special access instructions for initrd
        off = getattr(self.im.installerConf, 'initrd_offset', None)
        if off is not None:
            if self.rawLoaderDevice is not None:
                a = self.rawLoaderDevice
            else:
                a = self.im.installerConf.initrd_archive
            s = int(self.im.installerConf.initrd_offset)
            e = s + int(self.im.installerConf.initrd_size) - 1
            self.im.ubootEnv.onl_installer_initrd = ("%s:%x:%x" % (a, s, e,))
        else:
            try:
                del self.im.installerConf.onl_installer_initrd
            except AttributeError:
                pass

        if self.im.isOnie():
            self.log.info("Setting ONIE nos_bootcmd to boot ONL")
            self.im.ubootEnv.nos_bootcmd = self.im.str_bootcmd()
        else:
            self.log.warn("U-boot boot setting is not changed")

        return 0

    def installUboot(self):

        if self.device is None:
            self.log.error("missing block device YAML config")
            return 1
        st = os.stat(self.device)
        if not stat.S_ISBLK(st[stat.ST_MODE]):
            self.log.error("not a block device: %s", self.device)
            return 1

        # get a handle to the installer zip
        p = os.path.join(self.im.installerConf.installer_dir,
                         self.im.installerConf.installer_zip)
        self.zf = zipfile.ZipFile(p)

        code = self.loadPlugins()
        if code: return code

        code = self.runPlugins(Plugin.PLUGIN_PREINSTALL)
        if code: return code

        code = self.assertUnmounted()
        if code: return code

        #check the install device is flash device or not
        if "mtdblock" in self.device:
            code = self.maybeCreateUBIfs()
            if code: return code

            self.ubiParts = UbinfoParser(log=self.log.getChild("ubinfo -a"))

            files = os.listdir(self.im.installerConf.installer_dir) + self.zf.namelist()
            swis = [x for x in files if x.endswith('.swi')]
            if not swis:
                self.log.info("No ONL Software Image available for installation.")
                self.log.info("Post-install ZTN installation will be required.")
                return 1
            if len(swis) > 1:
                self.log.warn("Multiple SWIs found in installer: %s", " ".join(swis))
                return 1
            base = swis[0]
            self.log.info("Installing ONL Software Image (%s)...", base)
            dstDir = "/tmp/ubifs"
            cmd = ("/bin/mkdir", dstDir, )
            self.check_call(cmd, vmode=self.V2)
            dev = self.ubiParts['ONL-IMAGES']
            device = "/dev/" + dev['device'] + "_" + dev['Volume ID']
            cmd = ('mount', '-t', dev['fsType'], device, dstDir,)
            self.check_call(cmd, vmode=self.V2)
            dst = os.path.join(dstDir, base)
            self.installerCopy(base, dst)
            
            loaderBasename = None
            for c in sysconfig.installer.fit:
                if self.installerExists(c):
                    loaderBasename = c
                    break

            if not loaderBasename:
                self.log.error("The platform loader file is missing.")
                return 1

            self.log.info("Installing the ONL loader from %s...", loaderBasename)
            

            dstDir = "/tmp/ubiloader"
            cmd = ("/bin/mkdir", dstDir, )
            self.check_call(cmd, vmode=self.V2)
            dev = self.ubiParts['ONL-BOOT']
            device = "/dev/" + dev['device'] + "_" + dev['Volume ID']
            cmd = ('mount', '-t', dev['fsType'], device, dstDir,)
            self.check_call(cmd, vmode=self.V2)
            self.log.info("Installing ONL loader %s --> %s:%s...",
                                  loaderBasename, device, loaderBasename)

            dst = os.path.join(dstDir, "%s.itb" % self.im.installerConf.installer_platform)

            self.installerCopy(loaderBasename, dst)

            self.log.info("Installing boot-config to %s", device)
            basename = 'boot-config'
            dst = os.path.join(dstDir, basename)
            if not self.installerCopy(basename, dst, True):
                return 1

            with open(dst) as fd:
                buf = fd.read()

            ecf = buf.encode('base64', 'strict').strip()
            if self.im.grub and self.im.grubEnv is not None:
                setattr(self.im.grubEnv, 'boot_config_default', ecf)
            if self.im.uboot and self.im.ubootEnv is not None:
                setattr(self.im.ubootEnv, 'boot-config-default', ecf)

        
            
            dstDir = "/tmp/ubiconfig"
            cmd = ("/bin/mkdir", dstDir, )
            self.check_call(cmd, vmode=self.V2)
            dev = self.ubiParts['ONL-CONFIG']
            device = "/dev/" + dev['device'] + "_" + dev['Volume ID']
            cmd = ('mount', '-t', dev['fsType'], device, dstDir,)
            self.check_call(cmd, vmode=self.V2)

            for f in self.zf.namelist():
                d = 'config/'
                if f.startswith(d) and f != d:
                    dst = os.path.join(dstDir, os.path.basename(f))
                    if not os.path.exists(dst):
                        self.installerCopy(f, dst)


            self.log.info("syncing block devices")
            self.check_call(('sync',))


            code = self.installUbootEnv()
            if code: return code
            
            code = self.runPlugins(Plugin.PLUGIN_POSTINSTALL)
            if code: return code
            return 0


            cmd = ('umount', dstDir)
            self.check_call(cmd, vmode=self.V2)
            return 0

        code = self.maybeCreateLabel()
        if code: return code

        self.log.info("Installing to %s", self.device)

        self.log.info("found %s partitions (bsz %s, lbsz %s)",
                      self.partedDisk.type,
                      self.partedDevice.sectorSize,
                      self.partedDevice.physicalSectorSize)
        if self.partedDisk.type != 'msdos':
            self.log.error("not an MSDOS partition table")
            return 1
        if self.partedDevice.sectorSize != 512:
            self.log.warn("invalid logical block size, expected 512")
        if self.partedDevice.physicalSectorSize != 512:
            self.log.warn("invalid physical block size, expected 512")

        self.log.info("found a disk with %d blocks",
                      self.partedDevice.getLength())

        code = self.findMsdos()
        if code: return code

        code = self.deletePartitions()
        if code: return code

        self.log.info("next usable block is %s", self.nextBlock)

        code = self.partitionParted()
        if code: return code

        # compute the path to the raw loader partition,
        # if indicated by the configuration

        self.rawLoaderDevice = None
        for item in self.im.platformConf['installer']:
            partIdx, partData = list(item.items())[0]
            label, part = list(partData.items())[0]
            if label == 'ONL-BOOT' and part['format'] == 'raw':
                self.rawLoaderDevice = self.device + str(partIdx+1)
                break

        code = self.installSwi()
        if code: return code

        code = self.installLoader()
        if code: return code

        if self.rawLoaderDevice is None:
            code = self.installBootConfig()
            if code: return code
        else:
            self.log.info("ONL-BOOT is a raw partition (%s), skipping boot-config",
                          self.rawLoaderDevice)


        code = self.installOnlConfig()
        if code: return code

        self.log.info("syncing block devices")
        self.check_call(('sync',))
        # XXX roth probably not needed

        code = self.installUbootEnv()
        if code: return code

        code = self.runPlugins(Plugin.PLUGIN_POSTINSTALL)
        if code: return code

        return 0

    def run(self):

        if 'flat_image_tree' not in self.im.platformConf:
            self.log.error("platform config is missing a FIT section")
            return 1

        return self.installUboot()

    def upgradeBootLoader(self):
        """Upgrade the boot loader settings as part of a loader upgrade."""

        self.blkidParts = BlkidParser(log=self.log.getChild("blkid"))

        # XXX boot-config (and saved boot-config) should be unchanged during loader upgrade

        code = self.installUbootEnv()
        if code: return code

        return 0

    def shutdown(self):
        Base.shutdown(self)
