"""InstallUtils.py

"""

import os, sys
import stat
import logging
import subprocess
import tempfile
import string
import shutil
import Fit, Legacy

class SubprocessMixin:

    V1 = "V1"
    V2 = "V2"

    def check_call(self, *args, **kwargs):
        args = list(args)
        kwargs = dict(kwargs)

        cwd = kwargs.pop('cwd', None)
        if cwd is not None:
            self.log.debug("+ cd " + cwd)

        if args:
            cmd = args.pop(0)
        else:
            cmd = kwargs.pop('cmd')

        vmode = kwargs.pop('vmode', None)
        if vmode == self.V1 and self.log.isEnabledFor(logging.DEBUG):
            if isinstance(cmd, basestring):
                raise ValueError("vmode=V1 requires a list")
            cmd = list(cmd)
            cmd[1:1] = ['-v',]
        if vmode == self.V2 and self.log.isEnabledFor(logging.DEBUG):
            stdout = kwargs.pop('stdout', None)
            stderr = kwargs.pop('stderr', None)
            if stdout is not None:
                raise ValueError("vmode=V2 conflicts with stdout")
            if stderr is not None and stderr != subprocess.STDOUT:
                raise ValueError("vmode=V2 conflicts with stderr")
            fno, v2Out = tempfile.mkstemp(prefix='subprocess-',
                                          suffix='out')
            kwargs['stdout'] = fno
            kwargs['stderr'] = subprocess.STDOUT

        if isinstance(cmd, basestring):
            self.log.debug("+ " + cmd)
        else:
            self.log.debug("+ " + " ".join(cmd))

        if vmode == self.V2 and self.log.isEnabledFor(logging.DEBUG):
            try:
                subprocess.check_call(cmd, *args, cwd=cwd, **kwargs)
            finally:
                with open(v2Out) as fd:
                    sys.stderr.write(fd.read())
                os.unlink(v2Out)
        else:
            subprocess.check_call(cmd, *args, cwd=cwd, **kwargs)

    def check_output(self, *args, **kwargs):
        args = list(args)
        kwargs = dict(kwargs)

        cwd = kwargs.pop('cwd', None)
        if cwd is not None:
            self.log.debug("+ cd " + cwd)

        if args:
            cmd = args.pop(0)
        else:
            cmd = kwargs.pop('cmd')

        vmode = kwargs.pop('vmode', None)
        if vmode == self.V1 and self.log.isEnabledFor(logging.DEBUG):
            if isinstance(cmd, basestring):
                raise ValueError("vmode=V1 requires a list")
            cmd = list(cmd)
            cmd[1:1] = ['-v',]
        if vmode == self.V2 and self.log.isEnabledFor(logging.DEBUG):
            stdout = kwargs.pop('stdout', None)
            stderr = kwargs.pop('stderr', None)
            if stdout is not None:
                raise ValueError("vmode=V2 conflicts with stdout")
            if stderr is not None and stderr != subprocess.STDOUT:
                raise ValueError("vmode=V2 conflicts with stderr")
            fno, v2Out = tempfile.mkstemp(prefix='subprocess-',
                                          suffix='out')
            kwargs['stderr'] = fno

        if isinstance(cmd, basestring):
            self.log.debug("+ " + cmd)
        else:
            self.log.debug("+ " + " ".join(cmd))

        if vmode == self.V2 and self.log.isEnabledFor(logging.DEBUG):
            try:
                return subprocess.check_output(cmd, *args, cwd=cwd, **kwargs)
            finally:
                with open(v2Out) as fd:
                    sys.stderr.write(fd.read())
                os.unlink(v2Out)
        else:
            try:
                return subprocess.check_output(cmd, *args, cwd=cwd, **kwargs)
            except subprocess.CalledProcessError:
                return ''

    def rmdir(self, path):
        self.log.debug("+ /bin/rmdir %s", path)
        os.rmdir(path)

    def unlink(self, path):
        self.log.debug("+ /bin/rm %s", path)
        os.unlink(path)

    def rmtree(self, path):
        self.log.debug("+ /bin/rm -fr %s", path)
        shutil.rmtree(path)

    def mkdtemp(self, *args, **kwargs):
        path = tempfile.mkdtemp(*args, **kwargs)
        self.log.debug("+ /bin/mkdir %s", path)
        return path

    def copy2(self, src, dst):
        self.log.debug("+ /bin/cp -a %s %s", src, dst)
        shutil.copy2(src, dst)

    def copyfile(self, src, dst):
        self.log.debug("+ /bin/cp %s %s", src, dst)
        shutil.copyfile(src, dst)

    def mkdir(self, path):
        self.log.debug("+ /bin/mkdir %s", path)
        os.mkdir(path)

    def makedirs(self, path):
        self.log.debug("+ /bin/mkdir -p %s", path)
        os.makedirs(path)

    def symlink(self, tgt, dst):
        self.log.debug("+ /bin/ln -s %s %s", tgt, dst)
        os.symlink(tgt, dst)

    def mkdosfs(self, dev, label=None):
        if label is not None:
            cmd = ('mkdosfs', '-n', label, dev,)
        else:
            cmd = ('mkdosfs', dev,)
        self.check_call(cmd, vmode=self.V1)

    def mke2fs(self, dev, label=None):
        if label is not None:
            cmd = ('mkfs.ext2', '-L', label, dev,)
        else:
            cmd = ('mkfs.ext2', dev,)
        self.check_call(cmd, vmode=self.V1)

    def mke4fs(self, dev, label=None, huge_file=True):
        if label is not None:
            cmd = ['mkfs.ext4', '-L', label, dev,]
        else:
            cmd = ['mkfs.ext4', dev,]

        if not huge_file:
            cmd[1:1] = ['-O', '^huge_file',]
        # hack needed for some old ONIE kernels

        self.check_call(cmd, vmode=self.V1)

    def mkfs(self, dev, fstype):
        mkfs = 'mkfs.%s' % fstype
        cmd = (mkfs, dev,)

        # 'mkfs -h' says to use '-V' for verbose,
        # don't believe it
        self.check_call(cmd, vmode=self.V1)

    def cpR(self, srcRoot, dstRoot):
        srcRoot = os.path.abspath(srcRoot)
        dstRoot = os.path.abspath(dstRoot)
        dstRoot = os.path.join(dstRoot, os.path.split(srcRoot)[1])
        for r, dl, fl in os.walk(srcRoot):

            for de in dl:
                src = os.path.join(r, de)
                subdir = src[len(srcRoot)+1:]
                dst = os.path.join(dstRoot, subdir)
                if not os.path.exists(dst):
                    self.makedirs(dst)

            for fe in fl:
                src = os.path.join(r, fe)
                subdir = src[len(srcRoot)+1:]
                dst = os.path.join(dstRoot, subdir)
                self.copy2(src, dst)

class TempdirContext(SubprocessMixin):

    def __init__(self, prefix=None, suffix=None, chroot=None, log=None):
        self.prefix = prefix
        self.suffix = suffix
        self.chroot = chroot
        self.dir = None
        self.hostDir = None
        self.log = log or logging.getLogger("mount")

    def __enter__(self):
        if self.chroot is not None:
            self.hostDir = self.mkdtemp(prefix=self.prefix,
                                        suffix=self.suffix,
                                        dir=self.chroot + "/tmp")
            self.dir = self.hostDir[len(self.chroot):]
        else:
            self.dir = self.hostDir = self.mkdtemp(prefix=self.prefix,
                                                   suffix=self.suffix)
        return self

    def __exit__(self, type, value, tb):
        if self.path: self.rmtree(self.hostDir)
        return False

class MountContext(SubprocessMixin):

    def __init__(self, device=None, chroot=None, label=None, fsType=None, log=None):
        self.device = device
        self.chroot = chroot
        self.label = label
        self.fsType = fsType
        self.dir = None
        self.hostDir = self.__hostDir = None
        self.mounted = self.__mounted = False
        self.log = log or logging.getLogger("mount")

        if self.device and self.label:
            raise ValueError("cannot specify device and label")
        if not self.device and not self.label:
            raise ValueError("no device or label specified")

    def __enter__(self):
        dev = self.device
        if dev is None:
            try:
                dev = self.check_output(('blkid', '-L', self.label,)).strip()
            except subprocess.CalledProcessError, what:
                raise ValueError("cannot find label %s: %s"
                                 % (self.label, str(what),))

        if self.chroot is not None:
            self.hostDir = self.mkdtemp(prefix="mount-",
                                        suffix=".d",
                                        dir=self.chroot + "/tmp")
            self.dir = self.hostDir[len(self.chroot):]
        else:
            self.dir = self.hostDir = self.mkdtemp(prefix="mount-",
                                                   suffix=".d")

        if self.fsType is not None:
            cmd = ('mount', '-t', self.fsType, dev, self.hostDir,)
        else:
            cmd = ('mount', dev, self.hostDir,)
        self.check_call(cmd, vmode=self.V1)
        self.mounted = True
        return self

    def shutdown(self):

        mounted = False
        if self.mounted:
            p = ProcMountsParser()
            for e in p.mounts:
                if e.dir == self.hostDir:
                    mounted = True
                    break
        # really mounted?
        # maybe unmounted e.g. if inside a chroot
        if mounted:
            cmd = ('umount', self.hostDir,)
            self.check_call(cmd, vmode=self.V1)

        if self.hostDir is not None:
            self.rmdir(self.hostDir)

    def __exit__(self, type, value, tb):
        self.shutdown()
        return False

    def detach(self):
        self.__mounted, self.mounted = self.mounted, False
        self.__hostDir, self.hostDir = self.hostDir, None

    def attach(self):
        self.mounted = self.__mounted
        self.hostDir = self.__hostDir

class BlkidEntry:

    def __init__(self, device, **kwargs):

        self.device = device

        kwargs = dict(kwargs)
        self.label = kwargs.pop('label', None)
        self.uuid = kwargs.pop('uuid', None)
        self.fsType = kwargs.pop('fsType', None)

    @classmethod
    def fromLine(cls, line):
        line = line.strip()
        p = line.find(':')
        if p < 0:
            raise ValueError("invalid blkid output %s"
                             % line)
        dev, line = line[:p], line[p+1:].strip()

        attrs = {}
        while line:
            p = line.find('=')
            if p < 0:
                raise ValueError("invalid blkid output %s"
                                 % line)
            key = line[:p].lower()
            if line[p+1:p+2] == "'":
                q = line.find("'", p+2)
                if q < 0:
                    val, line = line[p+1:], ""
                else:
                    val, line = line[p+2:q], line[q+1:].strip()
            elif line[p+1:p+2] == '"':
                q = line.find('"', p+2)
                if q < 0:
                    val, line = line[p+1:], ""
                else:
                    val, line = line[p+2:q], line[q+1:].strip()
            else:
                q = line.find(" ", p+1)
                if q < 0:
                    val, line = line[p+1:], ""
                else:
                    val, line = line[p+1:], line[:q].strip()

            if key == 'type': key = 'fsType'
            attrs[key] = val

        return cls(dev, **attrs)

    def splitDev(self):
        dev, part = self.device, ""
        while dev[-1:] in string.digits:
            dev, part = dev[:-1], dev[-1] + part
        return dev, part

    def isOnieReserved(self):
        if self.label is None: return False

        if 'GRUB' in self.label: return True
        if 'ONIE-BOOT' in self.label: return True
        if 'DIAG' in self.label: return True

        return False

class BlkidParser(SubprocessMixin):

    def __init__(self, log=None):
        self.log = log or logging.getLogger("blkid")
        self.parse()

    def parse(self):
        cmd = ('blkid',)
        lines = self.check_output(cmd).splitlines()
        self.parts = [BlkidEntry.fromLine(line) for line in lines]

    def __getitem__(self, idxOrName):
        if type(idxOrName) == int:
            return self.parts[idxOrName]
        for part in self.parts:
            if part.label == idxOrName: return part
            if part.uuid == idxOrName: return part
        raise IndexError("cannot find partition %s" % repr(idxOrName))

    def __len__(self):
        return len(self.parts)


class UbinfoParser(SubprocessMixin):

    def __init__(self, log=None):
        self.log = log or logging.getLogger("ubinfo -a")
        self.parse()

    def parse(self):
        self.parts = []
        lines = ''
        try:
            cmd = ('ubinfo', '-a',)
            lines = self.check_output(cmd).splitlines()
        except Exception as ex:
            return self

        dev = None
        volId = None
        name = None
        attrs = {}
        for line in lines:
            line = line.strip()

            p = line.find(':')
            if p < 0: continue
            name, value = line[:p], line[p+1:].strip()
            if 'Volume ID' in name:
                p = value.find('(')
                if p < 0: continue
                volumeId = value[:p].strip()
                attrs['Volume ID'] = volumeId
                p = value.find('on')
                if p < 0: continue
                dev = value[p+2:-1].strip()
                attrs['device'] = dev

            if 'Name' in name:
               dev = "/dev/" + dev + "_" + volumeId
               p = line.find(':')
               if p < 0: continue
               attrs['Name'] = line[p+1:].strip()
               attrs['fsType'] = 'ubifs'
               self.parts.append(attrs)
               dev = None
               volId = None
               name = None
               attrs = {}

    def __getitem__(self, idxOrName):
        if type(idxOrName) == int:
            return self.parts[idxOrName]
        for part in self.parts:
            if part['Name'] == idxOrName: return part
        raise IndexError("cannot find partition %s" % repr(idxOrName))

    def __len__(self):
        return len(self.parts)

class ProcMtdEntry:

    def __init__(self,
                 charDevice, blockDevice,
                 offset, size, eraseSize,
                 label=None):

        self.charDevice = charDevice
        self.blockDevice = blockDevice
        self.offset = offset
        self.size = size
        self.eraseSize = eraseSize
        self.label = label

    @classmethod
    def fromLine(cls, line, offset=0):
        buf = line.strip()
        p = buf.find(':')
        if p < 0:
            raise ValueError("invalid /proc/mtd entry %s"
                             % line)
        dev, buf = buf[:p], buf[p+1:].strip()
        dev = '/dev/' + dev
        if not os.path.exists(dev):
            raise ValueError("invalid /proc/mtd entry %s (missing device)"
                             % line)
        st = os.stat(dev)
        if stat.S_ISBLK(st.st_mode):
            cdev, bdev = None, dev
        elif stat.S_ISCHR(st.st_mode):
            cdev, bdev = dev, None
        else:
            cdev, bdev = None, None

        if cdev and not bdev:
            if cdev.startswith("/dev/mtd") and not cdev.startswith("/dev/mtdblock"):
                bdev = "/dev/mtdblock" + cdev[8:]
                if not os.path.exists(bdev):
                    raise ValueError("invalid /proc/mtd entry %s (cannot find block device)"
                                     % line)
                st = os.stat(bdev)
                if not stat.S_ISBLK(st.st_mode):
                    raise ValueError("invalid /proc/mtd entry %s (cannot find block device)"
                                     % line)
            else:
                raise ValueError("invalid /proc/mtd entry %s (cannot find block device)"
                                 % line)
        elif not bdev:
            raise ValueError("invalid /proc/mtd entry %s (not a block or char device)"
                             % line)

        p = buf.find(" ")
        if p < 0:
            raise ValueError("invalid /proc/mtd entry %s (missing size)"
                             % line)
        sz, buf = buf[:p], buf[p+1:].strip()
        sz = int(sz, 16)

        if not buf:
            raise ValueError("invalid /proc/mtd entry %s (missing erase size)"
                             % line)
        p = buf.find(" ")
        if p < 0:
            esz, buf = buf, ""
        else:
            esz, buf = buf[:p], buf[p+1:].strip()
        esz = int(esz, 16)

        if not buf:
            label = None
        elif len(buf) > 1 and buf[0:1] == "'" and buf[-1:] == "'":
            label = buf[1:-1]
        elif len(buf) > 1 and buf[0:1] == '"' and buf[-1:] == '"':
            label = buf[1:-1]
        else:
            label = buf

        return cls(cdev, bdev, offset, sz, esz, label=label)

class ProcMtdParser():

    def __init__(self, log=None):
        self.log = log or logging.getLogger("blkid")
        self.parse()

    def parse(self):
        self.parts = []
        offset = 0
        if os.path.exists("/proc/mtd"):
            with open("/proc/mtd") as fd:
                for line in fd.xreadlines():
                    if line.startswith("dev:"):
                        pass
                    else:
                        part = ProcMtdEntry.fromLine(line, offset=offset)
                        offset += part.size
                        self.parts.append(part)

    def __getitem__(self, idxOrName):
        if type(idxOrName) == int:
            return self.parts[idxOrName]
        for part in self.parts:
            if part.label == idxOrName: return part
        raise IndexError("cannot find MTD partition %s" % repr(idxOrName))

    def __len__(self):
        return len(self.parts)

class PartedDiskEntry:

    def __init__(self, device, blocks, lbsz, pbsz,
                 model=None, typ=None, flags=[]):
        self.device = device

        self.blocks = blocks
        self.lbsz = lbsz
        self.pbsz = pbsz

        self.model = model
        self.typ = typ
        self.flags = flags

    @classmethod
    def fromLine(cls, line):

        line = line.strip()
        if not line.endswith(';'):
            raise ValueError("invalid parted line %s" % line)
        line = line[:-1]
        rec = line.split(':')

        def _s():
            secs = rec.pop(0)
            if secs[-1:] != 's':
                raise ValueError("invalid sector count %s" % secs)
            return int(secs[:-1])

        dev = rec.pop(0)
        blocks = _s()
        model = rec.pop(0) or None
        lbsz = int(rec.pop(0), 10)
        pbsz = int(rec.pop(0), 10)
        typ = rec.pop(0)
        label = rec.pop(0) or None
        flags = rec.pop(0)
        flags = [x.strip() for x in flags.split(',')]

        if rec:
            raise ValueError("invalid parted line %s" % line)

        return cls(dev, blocks, lbsz, pbsz,
                   model=model, typ=typ,
                   flags=flags)

class PartedPartEntry:

    def __init__(self, part, start, end, sz,
                 fs=None, label=None, flags=[]):
        self.part = part
        self.start = start
        self.end = end
        self.sz = sz
        self.fs = fs
        self.label = label
        self.flags = flags

    @classmethod
    def fromLine(cls, line):

        line = line.strip()
        if not line.endswith(';'):
            raise ValueError("invalid parted line %s" % line)
        line = line[:-1]
        rec = line.split(':')

        def _s():
            secs = rec.pop(0)
            if secs[-1:] != 's':
                raise ValueError("invalid sector count %s" % secs)
            return int(secs[:-1])

        part = int(rec.pop(0), 10)
        if part < 1:
            raise ValueError("invalid partition %d" % part)
        start = _s()
        end = _s()
        sz = _s()
        fs = rec.pop(0) or None
        label = rec.pop(0) or None
        flags = rec.pop(0)
        flags = [x.strip() for x in flags.split(',')]

        if rec:
            raise ValueError("invalid parted line %s" % line)

        return cls(part, start, end, sz,
                   fs=fs, label=label,
                   flags=flags)

class PartedParser(SubprocessMixin):

    def __init__(self, device, log=None):
        self.device = device
        self.log = log or logging.getLogger("parted")
        self.parse()

    def parse(self):

        cmd = ('parted', '-m', self.device,
               'unit', 's',
               'print',)
        lines = self.check_output(cmd).splitlines()
        self.disk = None
        parts = {}
        for line in lines:
            if line.startswith('/dev/'):
                self.disk = PartedDiskEntry.fromLine(line)
            elif line[0:1] in string.digits:
                ent = PartedPartEntry.fromLine(line)
                if ent.part in parts:
                    raise ValueError("duplicate partition")
                parts[ent.part] = ent

        self.parts = []
        for partno in sorted(parts.keys()):
            self.parts.append(parts[partno])

        if self.disk is None:
            raise ValueError("no partition table found")

    def __len__(self):
        return len(self.parts)

class ProcMountsEntry:

    def __init__(self, device, dir, fsType, flags={}):
        self.device = device
        self.dir = dir
        self.fsType = fsType
        self.flags = flags

    @classmethod
    def fromLine(cls, line):
        buf = line.strip()

        idx = buf.find(' ')
        if idx < 0:
            raise ValueError("invalid /proc/mounts line %s", line)

        device, buf = buf[:idx], buf[idx+1:].strip()

        idx = buf.find(' ')
        if idx < 0:
            raise ValueError("invalid /proc/mounts line %s", line)

        dir, buf = buf[:idx], buf[idx+1:].strip()

        idx = buf.find(' ')
        if idx < 0:
            raise ValueError("invalid /proc/mounts line %s", line)

        fsType, buf = buf[:idx], buf[idx+1:].strip()

        idx = buf.rfind(' ')
        if idx < 0:
            raise ValueError("invalid /proc/mounts line %s", line)

        buf, _ = buf[:idx], buf[idx+1:].strip()

        idx = buf.rfind(' ')
        if idx < 0:
            buf = ""
        else:
            buf, _ = buf[:idx], buf[idx+1:].strip()

        flags = {}
        if buf:
            for flag in buf.split(','):
                idx = flag.find('=')
                if idx > -1:
                    key, val = flag[:idx], flag[idx+1:]
                else:
                    key, val = flag, True
                flags[key] = val

        return cls(device, dir, fsType, flags)

class ProcMountsParser:

    def __init__(self):
        self.parse()

    def parse(self):
        self.mounts = []
        with open("/proc/mounts") as fd:
            for line in fd.readlines():
                self.mounts.append(ProcMountsEntry.fromLine(line))

class InitrdContext(SubprocessMixin):

    def __init__(self, initrd=None, dir=None, log=None):
        if initrd is None and dir is None:
            raise ValueError("missing initrd or initrd dir")
        if initrd and dir:
            raise ValueError("cannot specify initrd and initrd dir")
        self.initrd = initrd
        self.dir = dir
        self.hlog = log or logging.getLogger("mount")
        self.ilog = self.hlog.getChild("initrd")
        self.ilog.setLevel(logging.INFO)
        self.log = self.hlog

        self.__initrd = None
        self.__dir = None
        self._hasDevTmpfs = False

    def _unpack(self):
        self.dir = self.mkdtemp(prefix="chroot-",
                                suffix=".d")
        with open(self.initrd) as fd:
            mbuf = fd.read(1024)
        if mbuf[0:2] == "\x1f\x8b":
            c1 = ('gzip', '-dc', self.initrd,)
        elif mbuf[0:2] == "BZ":
            c1 = ('bzip2', '-dc', self.initrd,)
        elif mbuf[0:6] == "\xfd7zXZ\x00":
            c1 = ('xz', '-dc', self.initrd,)
        else:
            raise ValueError("cannot decode initrd")
        c2 = ('cpio', '-imd',)
        self.log.debug("+ %s | %s",
                       " ".join(c1), " ".join(c2))
        try:
            p1 = subprocess.Popen(c1,
                                  stdout=subprocess.PIPE)
        except OSError as ex:
            self.log.exception("command not found: %s" % c1[0])
            raise ValueError("cannot start pipe")
        try:
            if self.log.isEnabledFor(logging.DEBUG):
                p2 = subprocess.Popen(c2,
                                      cwd=self.dir,
                                      stdin=p1.stdout)
            else:
                p2 = subprocess.Popen(c2,
                                      cwd=self.dir,
                                      stdin=p1.stdout,
                                      stdout=subprocess.PIPE,
                                      stderr=subprocess.STDOUT)
        except OSError as ex:
            self.log.exception("cannot start command: %s" % c2[0])
            raise ValueError("cannot start pipe")
        c1 = p1.wait()
        out, _ = p2.communicate()
        c2 = p2.wait()
        if c2 and out:
            sys.stderr.write(out)
        if c1 or c2:
            raise ValueError("initrd unpack failed")

    def _prepDirs(self):

        dev2 = os.path.join(self.dir, "dev")
        if not os.path.exists(dev2):
            self.mkdir(dev2)

        for e in os.listdir(dev2):
            dst = os.path.join(dev2, e)
            if os.path.islink(dst):
                self.unlink(dst)
            elif os.path.isdir(dst):
                self.rmtree(dst)
            else:
                self.unlink(dst)

        if not self._hasDevTmpfs:
            for e in os.listdir("/dev"):
                src = os.path.join("/dev", e)
                dst = os.path.join(dev2, e)
                if os.path.islink(src):
                    self.symlink(os.readlink(src), dst)
                elif os.path.isdir(src):
                    self.mkdir(dst)
                elif os.path.isfile(src):
                    self.copy2(src, dst)
                else:
                    st = os.stat(src)
                    if stat.S_ISBLK(st.st_mode):
                        maj, min = os.major(st.st_rdev), os.minor(st.st_rdev)
                        self.log.debug("+ mknod %s b %d %d", dst, maj, min)
                        os.mknod(dst, st.st_mode, st.st_rdev)
                    elif stat.S_ISCHR(st.st_mode):
                        maj, min = os.major(st.st_rdev), os.minor(st.st_rdev)
                        self.log.debug("+ mknod %s c %d %d", dst, maj, min)
                        os.mknod(dst, st.st_mode, st.st_rdev)
                    else:
                        self.log.debug("skipping device %s", src)

        dst = os.path.join(self.dir, "dev/pts")
        if not os.path.exists(dst):
            self.mkdir(dst)

        if 'TMPDIR' in os.environ:
            dst = self.dir + os.environ['TMPDIR']
            if not os.path.exists(dst):
                self.makedirs(dst)

    def __enter__(self):

        with open("/proc/filesystems") as fd:
            buf = fd.read()
        if "devtmpfs" in buf:
            self._hasDevTmpfs = True

        if self.initrd is not None:

            self.log.debug("extracting initrd %s", self.initrd)
            self._unpack()

            self.log.debug("preparing chroot in %s", self.dir)
            try:
                self.log = self.ilog
                self._prepDirs()
            finally:
                self.log = self.hlog

        dst = os.path.join(self.dir, "proc")
        cmd = ('mount', '-t', 'proc', 'proc', dst,)
        self.check_call(cmd, vmode=self.V1)

        dst = os.path.join(self.dir, "sys")
        cmd = ('mount', '-t', 'sysfs', 'sysfs', dst,)
        self.check_call(cmd, vmode=self.V1)

        # maybe mount devtmpfs
        if self._hasDevTmpfs:
            dst = os.path.join(self.dir, "dev")
            cmd = ('mount', '-t', 'devtmpfs', 'devtmpfs', dst,)
            self.check_call(cmd, vmode=self.V1)

            dst = os.path.join(self.dir, "dev/pts")
            if not os.path.exists(dst):
                self.mkdir(dst)

        dst = os.path.join(self.dir, "dev/pts")
        cmd = ('mount', '-t', 'devpts', 'devpts', dst,)
        self.check_call(cmd, vmode=self.V1)

        return self

    def unmount(self):

        p = ProcMountsParser()
        if self.dir is not None:
            dirs = [e.dir for e in p.mounts if e.dir.startswith(self.dir)]
        else:
            dirs = []

        # XXX probabaly also kill files here

        # umount any nested mounts
        if dirs:
            self.log.debug("un-mounting mounts points in chroot %s", self.dir)
            dirs.sort(reverse=True)
            for p in dirs:
                cmd = ('umount', p,)
                self.check_call(cmd, vmode=self.V1)

    def shutdown(self):

        self.unmount()

        if self.initrd and self.dir:
            self.log.debug("cleaning up chroot in %s", self.dir)
            self.rmtree(self.dir)
        elif self.dir:
            self.log.debug("saving chroot in %s", self.dir)

    def __exit__(self, type, value, tb):
        self.shutdown()
        return False

    def detach(self):
        self.__initrd, self.initrd = self.initrd, None
        self.__dir, self.dir = self.dir, None

    def attach(self):
        self.initrd = self.__initrd
        self.dir = self.__dir

    @classmethod
    def mkChroot(cls, initrd, log=None):
        with cls(initrd=initrd, log=log) as ctx:
            initrdDir = ctx.dir
            ctx.detach()
            # save the unpacked directory, do not clean it up
            # (it's inside this chroot anyway)
        return initrdDir

class UbootInitrdContext(SubprocessMixin):

    def __init__(self, path, log=None):
        self.path = path
        self.log = log or logging.getLogger(self.__class__.__name__)
        self.initrd = self.__initrd = None

    def _extractFit(self):
        self.log.debug("parsing FIT image in %s", self.path)
        p = Fit.Parser(path=self.path, log=self.log)
        node = p.getInitrdNode()
        if node is None:
            raise ValueError("cannot find initrd node in FDT")
        prop = node.properties.get('data', None)
        if prop is None:
            raise ValueError("cannot find initrd data property in FDT")

        with open(self.path) as fd:
            self.log.debug("reading initrd at [%x:%x]",
                           prop.offset, prop.offset+prop.sz)
            fd.seek(prop.offset, 0)
            buf = fd.read(prop.sz)

        fno, self.initrd = tempfile.mkstemp(prefix="initrd-",
                                            suffix=".img")
        self.log.debug("+ cat > %s", self.initrd)
        with os.fdopen(fno, "w") as fd:
            fd.write(buf)

    def _extractLegacy(self):
        self.log.debug("parsing legacy U-Boot image in %s", self.path)
        p = Legacy.Parser(path=self.path, log=self.log)

        if p.ih_type != Legacy.Parser.IH_TYPE_MULTI:
            raise ValueError("not a multi-file image")

        if p.ih_os != Legacy.Parser.IH_OS_LINUX:
            raise ValueError("invalid OS code")

        sz, off = p.images[1]
        # assume the initrd is the second of three images

        with open(self.path) as fd:
            self.log.debug("reading initrd at [%x:%x]",
                           off, off+sz)
            fd.seek(off, 0)
            buf = fd.read(sz)

        fno, self.initrd = tempfile.mkstemp(prefix="initrd-",
                                            suffix=".img")
        self.log.debug("+ cat > %s", self.initrd)
        with os.fdopen(fno, "w") as fd:
            fd.write(buf)

    def __enter__(self):

        with open(self.path) as fd:
            isFit = Fit.Parser.isFit(stream=fd)
            isLegacy = Legacy.Parser.isLegacy(stream=fd)

        if isFit:
            self._extractFit()
            return self

        if isLegacy:
            self._extractLegacy()
            return self

        raise ValueError("invalid U-Boot image %s" % self.path)

    def shutdown(self):
        initrd, self.initrd = self.initrd, None
        if initrd and os.path.exists(initrd):
            self.unlink(initrd)

    def __exit__(self, eType, eValue, eTrace):
        self.shutdown()
        return False

    def detach(self):
        self.__initrd, self.initrd = self.initrd, None

    def attach(self):
        self.initrd = self.__initrd

class ChrootSubprocessMixin:

    chrootDir = None
    mounted = False
    # initialize this in a concrete class

    def check_call(self, *args, **kwargs):
        args = list(args)
        kwargs = dict(kwargs)

        cwd = kwargs.pop('cwd', None)
        if cwd is not None:
            self.log.debug("+ cd " + cwd)

        if args:
            cmd = args.pop(0)
        else:
            cmd = kwargs.pop('cmd')
        if isinstance(cmd, basestring):
            cmd = ('chroot', self.chrootDir,
                   '/bin/sh', '-c', 'IFS=;' + cmd,)
        else:
            cmd = ['chroot', self.chrootDir,] + list(cmd)

        if not self.mounted:
            with InitrdContext(dir=self.chrootDir, log=self.log) as ctx:
                self.log.debug("+ " + " ".join(cmd))
                subprocess.check_call(cmd, *args, cwd=cwd, **kwargs)
        else:
            self.log.debug("+ " + " ".join(cmd))
            subprocess.check_call(cmd, *args, cwd=cwd, **kwargs)

    def check_output(self, *args, **kwargs):
        args = list(args)
        kwargs = dict(kwargs)

        cwd = kwargs.pop('cwd', None)
        if cwd is not None:
            self.log.debug("+ cd " + cwd)

        if args:
            cmd = args.pop(0)
        else:
            cmd = kwargs.pop('cmd')
        if isinstance(cmd, basestring):
            cmd = ('chroot', self.chrootDir,
                   '/bin/sh', '-c', 'IFS=;' + cmd,)
        else:
            cmd = ['chroot', self.chrootDir,] + list(cmd)

        if not self.mounted:
            with InitrdContext(self.chrootDir, log=self.log) as ctx:
                self.log.debug("+ " + " ".join(cmd))
                return subprocess.check_output(cmd, *args, cwd=cwd, **kwargs)
        else:
            self.log.debug("+ " + " ".join(cmd))
            return subprocess.check_output(cmd, *args, cwd=cwd, **kwargs)
