"""ConfUtils.py

Config interfaces to different backend mechanisms.
"""

import os
import logging
import subprocess
from InstallUtils import SubprocessMixin, ChrootSubprocessMixin, MountContext
from cStringIO import StringIO

class ConfBase:

    def __init__(self):
        self._parse()

    def _parse(self):
        raise NotImplementedError

    def _feedLine(self, line):
        line = line.strip()
        if not line: return

        idx = line.find('=')
        if idx < 0:
            raise ValueError("invalid line in %s: %s"
                             % (self.path, line,))
        key, val = line[:idx], line[idx+1:]
        if val[:1] == '"' and val[-1:] == '"':
            val = val[1:-1]
        if val[:1] == "'" and val[-1:] == "'":
            val = val[1:-1]
        self.__dict__['_data'][key] = val

    def __getattr__(self, attr, *args):
        if len(args) == 1:
            return self.__dict__['_data'].get(attr, args[0])
        elif len(args) == 0:
            try:
                return self.__dict__['_data'][attr]
            except KeyError, what:
                raise AttributeError(str(what))
        else:
            raise ValueError("extra arguments")

    def __setattr__(self, attr, val):
        self.__dict__['_data'][attr] = val

    def dumps(self):
        """Generate a serialized representation."""
        buf = StringIO()
        data = self.__dict__.get('_data', {})
        for key, val in data.iteritems():
            buf.write("%s=\"%s\"\n" % (key, val,))
        return buf.getvalue()

class ConfFileBase(ConfBase):

    PATH = None
    # Override me

    def __init__(self, path=None):
        self.__dict__['path'] = path or self.PATH
        ConfBase.__init__(self)

    def _parse(self):
        self.__dict__['_data'] = {}
        with open(self.path) as fd:
            for line in fd.xreadlines():
                self._feedLine(line)

class MachineConf(ConfFileBase):
    PATH = "/etc/machine.conf"

class InstallerConf(ConfFileBase):
    PATH = "/etc/onl/installer.conf"

class ConfBuf(ConfBase):

    def __init__(self, buf):
        self.__dict__['buf'] = buf
        ConfBase.__init__(self)

    def _parse(self):
        self.__dict__['_data'] = {}
        for line in self.buf.splitlines():
            self._feedLine(line)

class GrubEnv(SubprocessMixin):

    INSTALL = "grub-install"
    EDITENV = "grub-editenv"
    # system default

    ENV_PATH = "/grub/grubenv"
    # override me

    def __init__(self,
                 bootDir=None, bootPart=None,
                 path=None,
                 log=None):

        if bootDir and bootPart:
            raise ValueError("cannot specify bootDir and bootPart")
        if not bootDir and not bootPart:
            raise ValueError("missing bootDir or bootPart")
        self.__dict__['bootDir'] = bootDir
        self.__dict__['bootPart'] = bootPart
        # location of GRUB boot files (mounted directory or unmounted partition)

        self.__dict__['path'] = path or self.ENV_PATH
        # path to grubenv, relative to above

        self.__dict__['log'] = log or logging.getLogger("grub")

    def mountCtx(self, device):
        return MountContext(device, fsType='ext4', log=self.log)

    def asDict(self):
        if self.bootPart:
            with self.mountCtx(self.bootPart) as ctx:
                p = os.path.join(ctx.dir, self.path.lstrip('/'))
                buf = self.check_output((self.EDITENV, p, 'list',)).strip()
        else:
            p = os.path.join(self.bootDir, self.path.lstrip('/'))
            buf = self.check_output((self.EDITENV, p, 'list',)).strip()
        cf = ConfBuf(buf)
        return cf.__dict__['_data']

    toDict = asDict

    def __getattr__(self, *args):

        args = list(args)
        attr = args.pop(0)

        d = self.asDict()
        if args:
            return d.get(attr, args[0])
        try:
            return d[attr]
        except KeyError, what:
            raise AttributeError(str(what))

    def __setattr__(self, attr, val):
        if self.bootPart:
            with self.mountCtx(self.bootPart) as ctx:
                p = os.path.join(ctx.dir, self.path.lstrip('/'))
                cmd = (self.EDITENV, p, 'set', ("%s=%s" % (attr, val,)),)
                self.check_call(cmd)
        else:
            p = os.path.join(self.bootDir, self.path.lstrip('/'))
            cmd = (self.EDITENV, p, 'set', ("%s=%s" % (attr, val,)),)
            self.check_call(cmd)

    def __delattr__(self, attr):
        if self.bootPart:
            with self.mountCtx(self.bootPart) as ctx:
                p = os.path.join(ctx.dir, self.path.lstrip('/'))
                cmd = (self.EDITENV, p, 'unset', attr,)
                self.check_call(cmd)
        else:
            p = os.path.join(self.bootDir, self.path.lstrip('/'))
            cmd = (self.EDITENV, p, 'unset', attr,)
            self.check_call(cmd)

    def install(self, device):
        if self.bootDir is not None:
            self.check_call((self.INSTALL, '--boot-directory=' + self.bootDir, device,))
        elif self.bootPart is not None:
            with self.mountCtx(self.bootPart) as ctx:
                self.check_call((self.INSTALL, '--boot-directory=' + ctx.dir, device,))
        else:
            self.check_call((self.INSTALL, device,))

class ChrootGrubEnv(ChrootSubprocessMixin, GrubEnv):

    def __init__(self,
                 chrootDir,
                 mounted=False,
                 bootDir=None, bootPart=None,
                 path=None,
                 log=None):
        self.__dict__['chrootDir'] = chrootDir
        self.__dict__['mounted'] = mounted
        GrubEnv.__init__(self,
                         bootDir=bootDir, bootPart=bootPart,
                         path=path,
                         log=log)

    def mountCtx(self, device):
        return MountContext(device,
                            chroot=self.chrootDir, fsType='ext4',
                            log=self.log)

class ProxyGrubEnv:
    """Pretend to manipulate the GRUB environment.

    Instead, write a trace of shell commands to a log
    so that e.g. the chroot's host can execute it with
    the proper GRUB runtime.
    """

    INSTALL = "grub-install"
    EDITENV = "grub-editenv"
    EFIBOOTMGR = "efibootmgr"
    # system defaults

    ENV_PATH = "/grub/grubenv"
    # override this

    def __init__(self,
                 installerConf,
                 bootDir=None, chroot=True, bootPart=None,
                 path=None,
                 log=None):

        self.__dict__['installerConf'] = installerConf
        # installer state, to retrieve e.g. chroot directory and trace log

        if bootDir and bootPart:
            raise ValueError("cannot specify bootDir and bootPart")
        if not bootDir and not bootPart:
            raise ValueError("missing bootDir or bootPart")
        self.__dict__['bootDir'] = bootDir
        self.__dict__['bootPart'] = bootPart
        # location of GRUB boot files (mounted directory or unmounted partition)

        self.__dict__['chroot'] = chroot
        # True of the bootDir is inside the chroot,
        # else bootDir is in the host's file namespace

        self.__dict__['path'] = path or self.ENV_PATH
        # path to grubenv, relative to above

        self.__dict__['log'] = log or logging.getLogger("grub")

    def asDict(self):
        raise NotImplementedError("proxy grubenv list not implemented")

    toDict = asDict

    def __getattr__(self, *args):
        raise NotImplementedError("proxy grubenv list not implemented")

    def __setattr__(self, attr, val):
        self.log.warn("deferring commands to %s...", self.installerConf.installer_postinst)

        cmds = []
        if self.bootDir and self.chroot:
            p = os.path.join(self.installerConf.installer_chroot,
                             self.bootDir.lstrip('/'),
                             self.path.lstrip('/'))
            cmds.append(("%s %s set %s=\"%s\"" % (self.EDITENV, p, attr, val,)))
        elif self.bootDir:
            p = os.path.join(self.bootDir,
                             self.path.lstrip('/'))
            cmds.append(("%s %s set %s=\"%s\"" % (self.EDITENV, p, attr, val,)))
        else:
            p = ("${mpt}/%s"
                 % (self.path.lstrip('/'),))
            cmds.append("mpt=$(mktemp -t -d)")
            cmds.append("mount %s $mpt" % self.bootPart)
            cmds.append(("sts=0; %s %s set %s=\"%s\" || sts=$?"
                         % (self.EDITENV, p, attr, val,)))
            cmds.append("umount $mpt")
            cmds.append("rmdir $mpt")
            cmds.append("test $sts -eq 0")

        with open(self.installerConf.installer_postinst, "a") as fd:
            for cmd in cmds:
                self.log.debug("+ [PROXY] " + cmd)
                fd.write(cmd)
                fd.write("\n")

    def __delattr__(self, attr):
        self.log.warn("deferring commands to %s...", self.installerConf.installer_postinst)

        cmds = []
        if self.bootDir and self.chroot:
            p = os.path.join(self.installerConf.installer_chroot,
                             self.bootDir.lstrip('/'),
                             self.path.lstrip('/'))
            cmds.append(("%s %s unset %s" % (self.EDITENV, p, attr,)))
        elif self.bootDir:
            p = os.path.join(self.bootDir,
                             self.path.lstrip('/'))
            cmds.append(("%s %s unset %s" % (self.EDITENV, p, attr,)))
        else:
            p = ("$mpt%s"
                 % (self.path.lstrip('/'),))
            cmds.append("mpt=$(mktemp -t -d)")
            cmds.append("mount %s $mpt" % self.bootPart)
            cmds.append(("sts=0; %s %s unset %s || sts=$?"
                         % (self.EDITENV, p, attr,)))
            cmds.append("umount $mpt")
            cmds.append("rmdir $mpt")
            cmds.append("test $sts -eq 0")

        with open(self.installerConf.installer_postinst, "a") as fd:
            for cmd in cmds:
                self.log.debug("+ [PROXY] " + cmd)
                fd.write(cmd)
                fd.write("\n")

    def install(self, device, isUEFI=False):
        self.log.warn("deferring commands to %s...", self.installerConf.installer_postinst)
        cmds = []
        if self.bootDir and self.chroot:
            p = os.pat.join(self.installerConf.installer_chroot,
                            self.bootDir.lstrip('/'))
            cmds.append(("%s --boot-directory=\"%s\" %s" % (self.INSTALL, p, device,)))
        elif self.bootDir:
            p = self.bootDir
            cmds.append(("%s --boot-directory=\"%s\" %s" % (self.INSTALL, p, device,)))
        elif self.bootPart:
            cmds.append("mpt=$(mktemp -t -d)")
            cmds.append("mount %s $mpt" % self.bootPart)
            if isUEFI:
                cmds.append("[ -n \"$(efibootmgr -v | grep 'Open Network Linux')\" ] && (efibootmgr -b $(efibootmgr | grep \"Open Network Linux\" | sed 's/^.*Boot//g'| sed 's/** Open.*$//g') -B)")
                cmds.append(("sts=0; %s --target=x86_64-efi --no-nvram --bootloader-id=ONL --efi-directory=/boot/efi --boot-directory=\"$mpt\" --recheck %s || sts=$?"
                             % (self.INSTALL, device,)))
                cmds.append("test $sts -eq 0")
                cmds.append(("sts=0; %s --quiet --create --label \"Open Network Linux\" --disk %s --part 1 --loader /EFI/ONL/grubx64.efi || sts=$?"
                             % (self.EFIBOOTMGR , device,)))
            else:
                cmds.append(("sts=0; %s --boot-directory=\"$mpt\" %s || sts=$?"
                         % (self.INSTALL, device,)))
            cmds.append("umount $mpt")
            cmds.append("rmdir $mpt")
            cmds.append("test $sts -eq 0")
        else:
            cmds.append(("%s %s"
                         % (self.INSTALL, device,)))

        with open(self.installerConf.installer_postinst, "a") as fd:
            for cmd in cmds:
                self.log.debug("+ [PROXY] " + cmd)
                fd.write(cmd)
                fd.write("\n")

class UbootEnv(SubprocessMixin):

    # ha ha, loader and SWI use different paths
    if os.path.exists("/usr/sbin/fw_setenv"):
        SETENV =  "/usr/sbin/fw_setenv"
    elif os.path.exists("/usr/bin/fw_setenv"):
        SETENV = "/usr/bin/fw_setenv"
    else:
        SETENV = "/bin/false"

    if os.path.exists("/usr/sbin/fw_printenv"):
        PRINTENV = "/usr/sbin/fw_printenv"
    elif os.path.exists("/usr/bin/fw_printenv"):
        PRINTENV = "/usr/bin/fw_printenv"
    else:
        PRINTENV = "/bin/false"

    def __init__(self, log=None):
        self.__dict__['log'] = log or logging.getLogger("u-boot")

        self.__dict__['hasForceUpdate'] = False
        try:
            out = self.check_output((self.SETENV, '--help',),
                                    stderr=subprocess.STDOUT)
            if "-f" in out and "Force update" in out:
                self.__dict__['hasForceUpdate'] = True
        except subprocess.CalledProcessError:
            if self.SETENV != '/bin/false':
                raise

    def __getattr__(self, *args):

        args = list(args)
        attr = args.pop(0)

        with open(os.devnull, "w") as nfd:
            try:
                out = self.check_output((self.PRINTENV, '-n', attr,),
                                        stderr=nfd.fileno())
            except subprocess.CalledProcessError:
                out = None

        if out is not None: return out

        if args:
            return args[0]

        raise AttributeError("firmware tag not found")

    def __setattr__(self, attr, val):
        if self.hasForceUpdate:
            self.check_call((self.SETENV, '-f', attr, val,))
        else:
            self.check_call((self.SETENV, attr, val,))

    def __delattr__(self, attr):

        if self.hasForceUpdate:
            self.check_call((self.SETENV, '-f', attr,))
        else:
            self.check_call((self.SETENV, attr,))

    def asDict(self):
        buf = self.check_output((self.PRINTENV,)).strip()
        return ConfBuf(buf).__dict__['_data']

    toDict = asDict
