#!/usr/bin/python
############################################################
#
# Build an ONL Installer
#
############################################################
import os
import sys
import argparse
import logging
import tempfile
import shutil
import subprocess

NAME="mkinstaller"
logging.basicConfig()
logger = logging.getLogger(NAME)
logger.setLevel(logging.DEBUG)

ONL = os.getenv('ONL')
if ONL is None:
    logger.error("$ONL is not set.")
    sys.exit(1)

class InstallerShar(object):
    def __init__(self, arch, template=None, work_dir=None):
        self.ONL = ONL

        if template is None:
            template = os.path.join(self.ONL, 'builds', 'any', 'installer', 'installer.sh.in')
        if not os.path.exists(template):
            self.abort("Template file '%s' is missing." % template)

        with open(template) as f:
            self.template = f.read()

        if work_dir:
            self.work_dir = work_dir
            self.remove_work_dir = False
            if os.path.exists(self.work_dir):
                logger.info("Removing existing work tree @ %s" % self.work_dir)
                shutil.rmtree(self.work_dir)

        else:
            self.work_dir = tempfile.mkdtemp()
            self.remove_work_dir = True

        if not os.path.isdir(self.work_dir):
            os.makedirs(self.work_dir)

        self.files = []
        self.dirs = []
        self.platforms = []
        self.arch = arch

        logger.info("Work Directory is %s" % self.work_dir)

        if self.arch == 'amd64':
            self.setvar("ARCH", 'x86_64')
        else:
            self.setvar("ARCH", self.arch)

    def abort(self, msg):
        logger.error(msg)
        sys.exit(1)

    def find_file(self, package, filename):
        return subprocess.check_output("onlpm --find-file %s %s" % (package, filename), shell=True).strip()

    def setvar(self, name, value):
        self.template = self.template.replace("@%s@" % name, value)

    def add_initrd(self, package, filename, add_platforms=True):
        self.initrd = self.find_file(package, filename)
        self.add_file(self.initrd)
        if add_platforms:
            for platform in subprocess.check_output("onlpm --platform-manifest %s" % (package), shell=True).split():
                logger.info("Adding platform %s..." % platform)
                kernel = subprocess.check_output([os.path.join(self.ONL, 'tools', 'onlplatform.py'),
                                                  platform,
                                                  self.arch,
                                                  'kernel']).strip()

                logger.info("Platform %s using kernel %s..." % (platform, os.path.basename(kernel)))
                self.add_file(kernel)

        self.setvar('INITRD_ARCHIVE', os.path.basename(self.initrd))
        self.setvar('INITRD_OFFSET', '')
        self.setvar('INITRD_SIZE', '')


    def add_fit(self, package, filename, add_platforms=True):
        self.fit = self.find_file(package, filename)
        VONL=os.path.join(self.ONL, "packages", "base", "all", "vendor-config-onl")
        offsets = subprocess.check_output("PYTHONPATH=%s/src/python %s/src/bin/pyfit -v offset %s --initrd" % (VONL, VONL, self.fit), shell=True).split()
        self.setvar('INITRD_ARCHIVE', os.path.basename(self.fit))
        self.setvar('INITRD_OFFSET', offsets[0])
        self.setvar('INITRD_SIZE', str(int(offsets[1]) - int(offsets[0])))
        self.add_file(self.fit)

    def add_file(self, filename):
        if not os.path.exists(filename):
            self.abort("File %s does not exist." % filename)

        logger.info("Adding file %s..." % os.path.basename(filename))
        self.files.append(filename)
        self.files = list(set(self.files))

    def add_dir(self, dir_):
        if not os.path.isdir(dir_):
            self.abort("Directory %s does not exist." % dir_)
        logger.info("Adding dir %s..." % dir_)
        self.dirs.append(dir_)
        self.dirs = list(set(self.dirs))

    def add_swi(self, package):
        edir = os.path.join(self.work_dir, "swidir")
        subprocess.check_output('onlpm --extract-dir %s %s' % (package, edir), shell=True)
        for (root, dirs, files) in os.walk(edir):
            for f in files:
                if f.endswith(".swi"):
                    self.add_file(os.path.join(root, f))


    def build(self, name):

        for f in self.files:
            shutil.copy(f, self.work_dir)

        for d in self.dirs:
            print "Copying %s -> %s..." % (d, self.work_dir)
            subprocess.check_call(["cp", "-R", d, self.work_dir])

        with open(os.path.join(self.work_dir, 'installer.sh'), "w") as f:
            f.write(self.template)
            f.write("PAYLOAD_FOLLOWS\n")

        with open(os.path.join(self.work_dir, "autoperms.sh"), "w") as f:
            f.write("#!/bin/sh\n")
            f.write("set -e\n")
            f.write("set -x\n")


        cwd = os.getcwd()
        os.chdir(self.work_dir)

        mkshar = [ os.path.join(self.ONL, 'tools', 'mkshar'),
                   '--lazy',
                   '--unzip-pad',
                   '--fixup-perms', 'autoperms.sh',
                   name,
                   os.path.join(self.ONL, 'tools', 'scripts', 'sfx.sh.in'),
                   'installer.sh',
                   ] + [ os.path.basename(f) for f in self.files ] + [ os.path.basename(d) for d in self.dirs ]

        subprocess.check_call(mkshar)
        os.chdir(cwd)




if __name__ == '__main__':

    ap = argparse.ArgumentParser(NAME)
    ap.add_argument("--arch", help="Installer Architecture.", required=True,
                    choices = ['amd64', 'powerpc', 'armel', 'arm64'])
    ap.add_argument("--initrd", nargs=2, help="The system initrd.")
    ap.add_argument("--fit", nargs=2, help="The system FIT image.")
    ap.add_argument("--boot-config", help="The boot-config source.", required=True)
    ap.add_argument("--add-file", help="Add the given file  to the installer package.", nargs='+', default=[])
    ap.add_argument("--add-dir", help="Optional directory to include in the installer.", nargs='+', default=[])
    ap.add_argument("--swi", help="Include the given SWI in the installer.")
    ap.add_argument("--work-dir", help="Set work directory and keep intermediates for debugging.")
    ap.add_argument("--verbose", '-v', help="Verbose output.", action='store_true')
    ap.add_argument("--out", help="Destination Filename")

    ops = ap.parse_args()
    installer = InstallerShar(ops.arch, ops.work_dir)

    if ops.arch == 'amd64':
        if ops.initrd is None:
            logger.error("--initrd must be used with architecture %s" % ops.arch)
            sys.exit(1)
        if ops.fit:
            logger.error("--fit cannot be used with architecture %s" % ops.arch)
            sys.exit(1)
    else:
        if ops.fit is None:
            logger.error("--fit must be used with architecture %s" % ops.arch)
            sys.exit(1)
        if ops.initrd:
            logger.error("--initrd cannot be used with architecture %s" % ops.arch)
            sys.exit(1)

    if ops.initrd:
        installer.add_initrd(*ops.initrd)
    if ops.fit:
        installer.add_fit(*ops.fit)

    installer.add_file(ops.boot_config)

    for f in ops.add_file:
        installer.add_file(f)

    for d in ops.add_dir:
        installer.add_dir(d)

    if ops.swi:
        installer.add_swi(ops.swi)

    iname = os.path.abspath(ops.out)
    installer.build(iname)
    logger.info("installer: %s" % iname)
