#!/usr/bin/python
############################################################
#
# Flat Image Tree Generator
#
############################################################
import subprocess
import yaml
import tempfile

import os, sys
toolsdir = os.path.dirname(os.path.abspath(__file__))
onldir = os.path.dirname(toolsdir)
pydir = os.path.join(onldir, "packages/base/all/vendor-config-onl/src/python")
sys.path.append(pydir)
import onl.YamlUtils

from onlpm import *
pm = defaultPm()

class Image(object):
    """Base ITS Image Class"""

    def __init__(self, type_, data, compression='none'):

        self.type = type_
        self.compression = compression
        self.load = None
        self.entry = None
        self.os = None

        if type(data) == str:
            if ',' in data:
                pkg, fname = [x.strip() for x in data.split(',')]
            else:
                pkg, fname = None, data
        elif type(data) == list:
            pkg, fname = data
        elif type(data) == dict:
            fname = data['=']
            pkg = data.get('package', None)
        else:
            raise ValueError("invalid image specifier: %s" % repr(data))

        if pkg is not None:
            pm.require(pkg, force=False, build_missing=True)
            self.data = pm.opr.get_file(pkg, fname)
        else:
            self.data = data

        try:
            self.name = os.path.basename(fname)
        except:
            import pdb
            pdb.set_trace()
            raise
        self.description = self.name


    def wl(self, line, indent="        "):
        self.handle.write("%s%s\n" % (indent, line))

    def start_image(self, f):
        self.handle = f
        self.wl("""%s {""" % self.name)
        self.wl("""    description = "%s";""" % self.description)
        self.wl("""    type = "%s";""" % self.type)
        self.wl("""    data = /incbin/("%s");""" % self.data)
        self.wl("""    arch = "%s";""" % ("arm" if ops.arch == 'armel' else ops.arch))
        self.wl("""    compression = "%s";""" % self.compression)
        if self.os:
            self.wl("""    os = %s;""" % self.os)
        if self.load:
            self.wl("""    load = %s;""" % self.load)
        if self.entry:
            self.wl("""    entry = %s;""" % self.entry)

    def include_hashes(self,f):
        self.wl("""    hash@1 {""")
        self.wl("""        algo = "crc32";""")
        self.wl("""    };""")

    def end_image(self, f):
        self.wl("""};""")


class KernelImage(Image):
    """Kernel image entry"""

    def __init__(self, fdata, arch):
        Image.__init__(self, "kernel", fdata, compression='gzip')
        self.os = '"linux"'

        # Fixme -- thse should be parameterized
        if arch == 'powerpc':
            self.load = "<0x0>"
            self.entry = "<0x0>"
        elif arch == 'armel':
            self.load = "<0x61008000>"
            self.entry = "<0x61008000>"
        elif arch == 'arm64':
            self.load = "<0x80080000>"
            self.entry = "<0x80080000>"

    def write(self, f):
        self.start_image(f)
        self.include_hashes(f)
        self.end_image(f)

class InitrdImage(Image):
    """Initrd image entry"""

    def __init__(self, fdata, arch):
        Image.__init__(self, "ramdisk", fdata, compression='gzip')

        # Fixme -- thse should be parameterized
        if arch == 'powerpc':
            self.load = "<0x1000000>"
            self.entry ="<0x1000000>"
        elif arch == 'armel':
            self.load = "<0x0000000>"
            self.entry ="<0x0000000>"
        elif arch == 'arm64':
            self.load = "<0x0000000>"
            self.entry ="<0x0000000>"

        self.os = '"linux"'

    def write(self, f):
        self.start_image(f)
        self.include_hashes(f)
        self.end_image(f)

class DtbImage(Image):
    """DTB Image Entry"""

    def __init__(self, fdata):
        Image.__init__(self, "flat_dt", fdata, compression="none")

    def write(self, f):
        self.start_image(f)
        self.end_image(f)


class FlatImageTree(object):
    """Generates a FIT .its file"""

    def __init__(self, description):
        self.kernels = []
        self.dtbs = []
        self.initrds = []
        self.description = description
        self.configurations = {}

    def add_initrd(self, initrd):
        self.initrds.append(initrd)

    def add_kernel(self, kernel):
        self.kernels.append(kernel)

    def add_dtb(self, dtb):
        self.dtbs.append(dtb)

    def add_config(self, name, kernel, dtb, initrd=None):
        if name in self.configurations:
            raise KeyError("Configuration '%s' already exists." % name)

        self.add_kernel(kernel)
        self.add_dtb(dtb)
        if initrd:
            self.add_initrd(initrd)
        else:
            # Use the first initrd as the default.
            if len(self.initrds) == 0:
                raise ValueError("No default initrd available while adding configuration '%s'" % name)
            else:
                initrd = self.initrds[0]

        self.configurations[name] = (kernel, dtb, initrd)

    def add_dict(self, name, d):
        if name in d:
            d = d[name]

        if 'flat_image_tree' in d:
            d = d['flat_image_tree']

        kernel = d.get('kernel', None)
        if kernel is None:
            raise KeyError("Configuration for %s does not contain a kernel key." % name)

        dtb = d.get('dtb', None)
        if dtb is None:
            raise KeyError("Configuration for %s does not contain a dtb key." % name)

        initrd = d.get('initrd', None)

        sys.stderr.write("*** platform %s kernel %s\n"
                         % (name, kernel,))
        self.add_config(name, kernel, dtb, initrd)


    def add_yaml(self, name, fname, defaults=None):
        if defaults is not None:
            d = onl.YamlUtils.merge(defaults, fname)
        else:
            with open(fname) as fd:
                d = yaml.load(fd)
        self.add_dict(name, d)

    def add_platform_package(self, package):
        print package
        platform = package.replace(":%s" % ops.arch, "").replace("onl-platform-config-", "")

        vpkg = "onl-vendor-config-onl:all"
        pm.require(vpkg, force=False, build_missing=True)
        y1 = pm.opr.get_file(vpkg, "platform-config-defaults-uboot.yml")

        pm.require(package, force=False, build_missing=True)
        y2 = pm.opr.get_file(package, platform + '.yml')

        self.add_yaml(platform, y2, defaults=y1)

    def add_platform(self, platform):
        if (":%s" % ops.arch) in platform:
            self.add_platform_package(platform)
        else:
            self.add_platform_package("onl-platform-config-%s:%s" % (platform, ops.arch))

    def write(self, fname):
        with open(fname, "w") as f:
            self.writef(f)

    def writef(self, f):

        kdict = {}
        for k in self.kernels:
            ki = KernelImage(k, ops.arch)
            kdict[ki.name] = ki

        ddict = {}
        for d in self.dtbs:
            di = DtbImage(d)
            ddict[di.name] = di

        idict = {}
        for i in self.initrds:
            ii = InitrdImage(i, ops.arch)
            idict[ii.name] = ii


        f.write("""/* \n""")
        f.write(""" * %s\n""" % self.description)
        f.write(""" */\n""")
        f.write("""\n""")
        f.write("""/dts-v1/;\n""")
        f.write("""/ {\n""")
        f.write("""    description = "%s";\n""" % self.description)
        f.write("""    #address-cells = <0x1>;\n""")
        f.write("""\n""")
        f.write("""    images {\n\n""")

        f.write("""        /* Kernel Images */\n""")
        for k in kdict.values():
            k.write(f)

        f.write("""\n""")
        f.write("""        /* DTB Images */\n""")
        for d in ddict.values():
            d.write(f)

        f.write("""\n""")
        f.write("""        /* Initrd Images */\n""")
        for i in idict.values():
            i.write(f)

        f.write("""    };\n""")
        f.write("""    configurations {\n""")
        for (name, (kernel, dtb, initrd)) in self.configurations.iteritems():
            f.write("""        %s {\n""" % name)
            f.write("""            description = "%s";\n""" % name)
            f.write("""            kernel = "%s";\n""" % (KernelImage(kernel, ops.arch).name))
            f.write("""            ramdisk = "%s";\n""" % (InitrdImage(initrd, ops.arch).name))
            f.write("""            fdt = "%s";\n""" % (DtbImage(dtb).name))
            f.write("""        };\n\n""")
        f.write("""    };\n""")
        f.write("""};\n""")


############################################################

if __name__ == '__main__':
    import os
    import sys
    import argparse

    ap = argparse.ArgumentParser(description="UBoot Flat Image Tree Generator.")
    ap.add_argument("--initrd", nargs='+',       action='append', help="Add initrds.")
    ap.add_argument("--kernel", nargs='+',       action='append', help="Add kernels.")
    ap.add_argument("--dtb",    nargs='+',       action='append', help="Add dtbs.")
    ap.add_argument("--add-yaml", nargs='+',     action='append', help="Add a configuration from a yaml configuration file.")
    ap.add_argument("--add-platform", nargs='+', action='append', help="Add the given ONL platforms to the configuration.")
    ap.add_argument("--desc", nargs=1, help="Flat Image Tree description", default="ONL Flat Image Tree.")
    ap.add_argument("--itb", metavar='itb-file', help="Compile result to an image tree blob file.")
    ap.add_argument("--its", metavar='its-file', help="Write result to an image tree source file.")
    ap.add_argument("--arch", choices=['powerpc', 'armel', 'arm64'], required=True)
    ops=ap.parse_args()

    fit = FlatImageTree(ops.desc)
    initrd=None

    if ops.initrd:
        for ilist in ops.initrd:
            for initrd in ilist:
                fit.add_initrd(initrd)
                # hack for now
                initrd = initrd

    if ops.kernel:
        for klist in ops.kernel:
            for kernel in klist:
                fit.add_kernel(kernel)

    if ops.dtb:
        for dlist in ops.dtb:
            for dtb in dlist:
                fit.add_dtb(dtb)

    if ops.add_yaml:
        for ylist in ops.add_yaml:
            for y in ylist:
                fit.add_yaml(y)

    if ops.add_platform == [['all']]:
        ops.add_platform = [ pm.list_platforms(ops.arch) ]

    if ops.add_platform == [['initrd']]:
        # Add support for the platforms listed in the initrd's platform manifest
        (package,f) = initrd.split(':')
        pkg = package + ':' + ops.arch
        pm.require(pkg, force=False, build_missing=True)
        mfile = pm.opr.get_file(pkg, "manifest.json")
        manifest = json.load(open(mfile))
        ops.add_platform = [[ "%s" % p for p in manifest['platforms'] ]]


    if ops.add_platform:
        for plist in ops.add_platform:
            for platform in plist:
                fit.add_platform(platform)

    if ops.itb is None:
        if ops.its is None:
            fit.writef(sys.stdout)
        else:
            fit.write(ops.its)
    else:
        its = ops.its
        if its is None:
            its = tempfile.NamedTemporaryFile(delete=False)
            fit.writef(its)
            its.close()
            its = its.name
        else:
            fit.write(its)

        if os.system("mkimage -f %s %s" % (its, ops.itb)) != 0:
            raise RuntimeError("Device tree compilation failed. See %s." % its)

        if ops.its is None:
            os.unlink(its)
