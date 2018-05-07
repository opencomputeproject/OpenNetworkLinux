#!/usr/bin/python
############################################################
#
# This script generates a repository skeleton for a new
# Network Operating System based on ONL.
#
############################################################
import os
import sys
import argparse
import inspect
import logging

logging.basicConfig()
logger = logging.getLogger("onl-nos-create")
logger.setLevel(logging.INFO)

ap = argparse.ArgumentParser("onl-nos-create");

ap.add_argument('--root', help="The root directory for the new NOS repository.", required=True)
ap.add_argument("--name", help="The name of this NOS.", required=True)
ap.add_argument('--prefix', help="Package prefix for this NOS.", required=True)
ap.add_argument('--arches', help="The architectures to include in this new NOS.", nargs='+', choices=['amd64', 'powerpc', 'armel', 'arm64'], required=True)
ap.add_argument('--debian', help="The debian distributions to incldue in this new NOS.", nargs='+', choices=['jessie', 'stretch'], required=True)
ap.add_argument('--copyright', help="The copyright to use for debian packages.", required=True)
ap.add_argument('--maintainer', help="The maintainer to use for debian packages.", required=True)
ap.add_argument('--version', help="The version to use for debian packages.", default="1.0.0")
ap.add_argument('--changelog', help="Initial changelog for debian packages.", default="Initial.")
ap.add_argument('--support', help="The support to use for debian packages.", required=True)
ap.add_argument("--csr-C", help="CSR Generation: C", required=True)
ap.add_argument("--csr-ST", help="CSR Generation: S", required=True)
ap.add_argument("--csr-O", help="CSR Generation: O", required=True)
ap.add_argument("--csr-localityName", help="CSR Generation: localityName", required=True)
ap.add_argument("--csr-commonName", help="CSR Generation: commonName", required=True)
ap.add_argument("--csr-organizationUnitName", help="CSR Generation: organizationUnitName", required=True)
ap.add_argument("--csr-emailAddress", help="CSR Generation: emailAddress", required=True)
ap.add_argument("--file", help="Output file object to stdout.")
ap.add_argument("--list-files", help="Show all files.", action='store_true')
ap.add_argument("--write-files", help="Write out all files.", action='store_true')
ap.add_argument("--overwrite", help="Overwrite existing files.", action='store_true')
ap.add_argument("--dry", "-n", help="Dry run.", action='store_true')

class NOSFile(object):
    path=None
    template=None

    def default_path(self):
        #
        # The file path is implicit in the classname.
        #
        clsname = self.__class__.__name__.split('_')

        replacements = {
            'APKG' : 'APKG.yml',
            'PKG'  : 'PKG.yml',
            'gitignore' : '.gitignore',
            'bootconfig' : 'boot-config',
        }

        clsname = map(lambda t: replacements.get(t, t), clsname)
        return os.path.join(*clsname)


    def __init__(self, root, keywords, path=None, template=None):
        self.root = root
        self.keywords = keywords
        self.keywords['MTOP'] = "$(%(PREFIX)s)" % keywords
        self.keywords['TOP'] = "$%(PREFIX)s" % keywords

        path = path if path else self.path
        path = path if path else self.default_path()
        template = template if template else self.template

        if template.startswith('\n'):
            template = template[1:]
        if not template.endswith('\n'):
            template = template + '\n'

        self.epath = path % self.keywords
        try:
            self.etemplate = template % self.keywords
        except:
            logger.error("Error evaluating template.")
            for line in template.split('\n'):
                logger.error("Running: '%s'" % line)
                logger.error(line % keywords)
            raise

    def write(self, stdout=False, overwrite=False, dry=False):
        if stdout:
            print self.etemplate
        else:
            abspath = os.path.join(self.root, self.epath)
            if not os.path.isdir(os.path.dirname(abspath)):
                os.makedirs(os.path.dirname(abspath))

            if os.path.exists(abspath):
                if not overwrite:
                    logger.debug("SKIP  %s" % abspath)
                    return
                else:
                    logger.info("OVER  %s" % abspath)
            else:
                logger.info("WRITE %s" % abspath)

            if not dry:
                with open(abspath, "w") as f:
                    f.write(self.etemplate)

    @classmethod
    def is_file(klass):
        return klass.template

class PKGFile(NOSFile):

    def __init__(self, root, keywords):
        common = """version: %(version)s
    copyright: %(copyright)s
    maintainer: %(maintainer)s
    changelog: %(changelog)s
    support:   %(support)s""" % keywords
        keywords['COMMON'] = common

        common_nv = """copyright: %(copyright)s
    maintainer: %(maintainer)s
    changelog: %(changelog)s
    support:   %(support)s""" % keywords

        keywords['COMMON_NV'] = common_nv

        NOSFile.__init__(self, root, keywords)

class CommonFile(PKGFile):
    pass

class ArchFile(PKGFile):
    arches = [ 'amd64', 'powerpc', 'armel', 'arm64' ]

    def __init__(self, root, keywords, arch):
        keywords['arch'] = arch
        self.arch = arch
        PKGFile.__init__(self, root, keywords)

    def default_path(self):
        path = NOSFile.default_path(self)
        return path.replace('arch', self.arch)

class ArchFileUboot(ArchFile):
    arches = [ 'arm64', 'armel', 'powerpc' ]
    def default_path(self):
        path = ArchFile.default_path(self)
        return path.replace('archuboot', self.arch)

class ArchFileGrub(ArchFile):
    arches = [ 'amd64' ]
    def default_path(self):
        path = ArchFile.default_path(self)
        return path.replace('archgrub', self.arch)

class OnlPkgMakefile(PKGFile):
    template="""include $(ONL)/make/pkg.mk"""

class OnlSubdirsMakefile(PKGFile):
    template="""include $(ONL)/make/subdirs.mk"""


class gitignore(CommonFile):
    template="""
# Everything in the REPO directory is ignored by default.

# Build products
BUILD/
dependmodules.x
*.deb
*.cpio.gz
*.sqsh
*.pyc
*.pyo

# Package cache and lock files
.lock
.PKGs.cache*

# Local module manifest.
.manifest.mk
RELEASE/

.bash_history
.buildroot-ccache

# temporary files
*~
.#*
"""

class make_config(CommonFile):
    path='make/config.mk'
    template="""
include $(ONL)/make/config.mk
#
# Custom configuration here
#
"""

class make_gitignore(CommonFile):
    template="""versions"""

class make_arch_config(ArchFile):
    path='make/config.%(arch)s.mk'
    template="""
include $(ONL)/make/config.%(arch)s.mk
include %(MTOP)s/make/config.mk
"""

class packages_Makefile(CommonFile,OnlPkgMakefile):
    pass

class packages_base_Makefile(CommonFile,OnlPkgMakefile):
    pass

class packages_base_any_initrd_loader_APKG(CommonFile):
    template = """
############################################################
#
# %(PREFIX)s Loader Package Template
#
# Requires: ARCH
#
############################################################
prerequisites:
  packages: [ "onl-loader-initrd:$ARCH" ]

packages:
  - name: %(prefix)s-loader-initrd
    arch: $ARCH
    %(COMMON)s
    summary: %(PREFIX)s Loader Initrd CPIO ($ARCH)

    files:
      builds/%(prefix)s-loader-initrd-$ARCH.cpio.gz : $$PKG_INSTALL/
      builds/manifest.json :                    $$PKG_INSTALL/

"""


class packages_base_any_initrd_loader_builds_Makefile(CommonFile):
    template="""
############################################################
#
# %(PREFIX)s  Loader initrd build template.
#
############################################################
ifndef ARCH
  $(error $$ARCH must be set)
endif

ROOT := root
TARGET := %(prefix)s-loader-initrd-$(ARCH).cpio.gz
.PHONY: $(TARGET)

$(TARGET):
\t$(ONLPM) --copy-file onl-loader-initrd:$(ARCH) onl-loader-initrd-$(ARCH).cpio.gz .
\t$(ONLPM) --copy-file onl-loader-initrd:$(ARCH) manifest.json .
\t$(ONL)/tools/sjson.py --inout manifest.json --kj version %(MTOP)s/make/versions/version-%(prefix)s.json
\tsudo rm -rf $(ROOT) && mkdir $(ROOT)
\tsudo mkdir -p $(ROOT)/etc/%(prefix)s/loader && sudo cp manifest.json $(ROOT)/etc/%(prefix)s/loader
\tsudo cp %(MTOP)s/make/versions/version-%(prefix)s.json $(ROOT)/etc/%(prefix)s/loader/versions.json
\tsudo cp %(MTOP)s/make/versions/version-%(prefix)s.sh   $(ROOT)/etc/%(prefix)s/loader/versions.sh
\tsudo cp %(MTOP)s/make/versions/version-%(prefix)s.mk   $(ROOT)/etc/%(prefix)s/loader/versions.mk
\t$(ONLPM) --sudo --force --extract-dir %(prefix)s-sysconfig:all $(ROOT)
\t$(ONLPM) --sudo --force --extract-dir %(prefix)s-loader-files:all $(ROOT)
\tsudo $(ONL)/tools/cpiomod.py --cpio onl-loader-initrd-$(ARCH).cpio.gz --add-directory $(ROOT) --out $@
\tsudo rm -rf $(ROOT) onl-loader-initrd-$(ARCH).cpio.gz

"""

class packages_base_arch_initrd_loader_builds_Makefile(ArchFile):
    template="""
include %(MTOP)s/make/config.%(arch)s.mk
include %(MTOP)s/packages/base/any/initrd/loader/builds/Makefile
"""

class packages_base_arch_initrd_loader_builds_gitignore(ArchFile):
    template="""manifest.json"""

class packages_base_arch_initrd_loader_PKG(ArchFile):
    template="""
!include %(TOP)s/packages/base/any/initrd/loader/APKG.yml ARCH=%(arch)s
"""

class packages_base_arch_initrd_loader_Makefile(ArchFile,OnlPkgMakefile):
    pass

class packages_base_arch_initrd_Makefile(ArchFile,OnlPkgMakefile):
    pass

class packages_base_any_hw_hw(CommonFile):
    path='packages/base/any/hw/hw.c'
    template="""
/** This is simply an example of building a local package into your NOS */
#include <stdio.h>

int
main(int argc, char* argv[])
{
    printf("%(PREFIX)s Hello, World!\\n");
    return 0;
}
"""

class packages_base_any_hw_APKG(CommonFile):
    template="""
packages:
  - name: %(prefix)s-hw
    arch: $ARCH
    %(COMMON)s
    summary: %(PREFIX)s Hello World
    files:
      builds/%(prefix)s-hw : /usr/bin/
"""

class packages_base_all_initrd_loaderfiles_Makefile(CommonFile,OnlPkgMakefile):
    path='packages/base/all/initrd/loader-files/Makefile'

class packages_base_all_initrd_loaderfiles_PKG(CommonFile):
    path='packages/base/all/initrd/loader-files/PKG.yml'
    template="""
packages:
  - name: %(prefix)s-loader-files
    arch: all
    %(COMMON)s
    summary: %(PREFIX)s Loader Files

    files:
      - src/lib : /lib

"""

class packages_base_all_initrd_loaderfiles_src_lib_bootcustom(CommonFile):
    path='packages/base/all/initrd/loader-files/src/lib/boot-custom'
    template="""
############################################################
#
# Included by the ONL boot1 script to perform
# custom preparation prior to switching root.
#
############################################################
cp -R /etc/%(prefix)s /newroot/etc

if [ -d /var/run/udhcpc ]; then
   cp -R /var/run/udhcpc /newroot/etc/%(prefix)s/loader
fi
"""

class packages_base_all_initrd_loaderfiles_src_lib_customize(CommonFile):
    path='packages/base/all/initrd/loader-files/src/lib/customize.sh'
    template="""
LOADER_SYSTEM_NAME="%(name)s"
LOADER_LOADER_NAME="Loader"
LOADER_PROMPT="loader# "
ONL_UDHCPC_VENDOR="%(PREFIX)s"

if [ -f /etc/%(prefix)s/loader/versions.sh ]; then
    . /etc/%(prefix)s/loader/versions.sh
fi
"""

class packages_base_all_sysconfig_PKG(CommonFile):
    template="""
packages:
  - name: %(prefix)s-sysconfig
    %(COMMON)s
    arch: all
    summary: %(PREFIX)s Sysconfig Package

    files:
      src/%(prefix)s.yml :   /etc/onl/sysconfig/01-%(prefix)s.yml

"""

class packages_base_all_sysconfig_Makefile(OnlPkgMakefile,CommonFile):
    pass

class packages_base_all_sysconfig_src(CommonFile):
    path='packages/base/all/sysconfig/src/%(prefix)s.yml'
    template="""
############################################################
#
# %(PREFIX)s System Configuration
#
############################################################
installer:
  menu_name: %(PREFIX)s
  os_name: %(name)s
  grub:
    - $PLATFORM.cpio.gz
    - %(prefix)s-loader-initrd-$PARCH.cpio.gz
  fit:
    - $PLATFORM.itb
    - %(prefix)s-loader-fit.itb

upgrade:

  system:
    auto: force

  onie:
    auto: advisory
    package:
      dir: /lib/platform-config/current/%(prefix)s/upgrade/onie

  firmware:
    auto: advisory
    package:
      dir: /lib/platform-config/current/%(prefix)s/upgrade/firmware

  loader:
    auto: force
    versions: /etc/%(prefix)s/loader/versions.json
    package:
      dir: /etc/%(prefix)s/upgrade/$PARCH
      grub:
        - $PLATFORM.cpio.gz
        - %(prefix)s-loader-initrd-$PARCH.cpio.gz
      fit:
        - $PLATFORM.itb
        - %(prefix)s-loader-fit.itb

pki:
  key:
    name: key.pem
    len:  2048
  cert:
    name: certificate
    csr:
      fields:
        C: %(csr_C)s
        ST: %(csr_ST)s
        O: %(csr_O)s
        localityName: %(csr_localityName)s
        commonName: %(csr_commonName)s
        organizationalUnitName: %(csr_organizationUnitName)s
        emailAddress: %(csr_emailAddress)s
      cdays: 3600

"""

class packages_base_arch_hw_Makefile(ArchFile,OnlPkgMakefile):
    pass

class packages_base_arch_hw_PKG(ArchFile):
    template="""
!include %(TOP)s/packages/base/any/hw/APKG.yml ARCH=%(arch)s
"""

class packages_base_arch_hw_builds_Makefile(ArchFile):
    template="""
include %(MTOP)s/make/config.%(arch)s.mk
include %(MTOP)s/packages/base/any/hw/builds/Makefile

"""

class packages_base_any_hw_builds_Makefile(CommonFile):
    template="""
all:
\t$(CROSS_COMPILER)gcc -o %(prefix)s-hw %(MTOP)s/packages/base/any/hw/hw.c
"""

class packages_base_arch_hw_builds_gitignore(ArchFile):
    template="""%(prefix)s-hw"""

class packages_base_arch_Makefile(ArchFile, OnlPkgMakefile):
    pass

class REPO_Makefile(CommonFile, OnlSubdirsMakefile):
    pass

class REPO_gitignore(CommonFile):
    template="""
extracts
*.deb
Packages
Packages.gz
"""

class tools_vi(CommonFile):
    path='tools/%(prefix)svi.py'
    template="""
import subprocess

class OnlVersionImplementation(object):

    PRODUCTS = [
        {
            "id" : "%(PREFIX)s",
#            "version": "Your poduct version here.""
            }
        ]

    def __init__(self):
        if 'version' in self.PRODUCTS[0]:
            # Release builds have a specific version.
            self.release = True
        else:
            # The current branch is used as the release version.
            self.release = False
            cmd = ('git', 'rev-parse', '--abbrev-ref', 'HEAD')
            branch = subprocess.check_output(cmd).strip()
            self.PRODUCTS[0]['version'] = branch

    def V_OS_NAME(self, data):
        return "%(name)s"

    def V_BUILD_SHA1(self, data):
        return data['build_sha1']

    def V_BUILD_SHORT_SHA1(self, data):
        return self.V_BUILD_SHA1(data)[0:7]

    def V_BUILD_TIMESTAMP(self, data):
        return data['build_timestamp']

    def V_FNAME_BUILD_TIMESTAMP(self, data):
        return self.V_BUILD_TIMESTAMP(data).replace(':', '')

    def V_BUILD_ID(self, data):
        return "{}-{}".format(self.V_BUILD_TIMESTAMP(data), self.V_BUILD_SHORT_SHA1(data))

    def V_FNAME_BUILD_ID(self, data):
        return "{}-{}".format(self.V_FNAME_BUILD_TIMESTAMP(data), self.V_BUILD_SHORT_SHA1(data))

    def V_PRODUCT_ID_VERSION(self, data):
        return data['product']['version']

    def V_VERSION_ID(self, data):
        return "%(PREFIX)s-{}".format(self.V_PRODUCT_ID_VERSION(data))

    def V_FNAME_VERSION_ID(self, data):
        return self.V_VERSION_ID(data)

    def V_PRODUCT_VERSION(self, data):
        return "%(PREFIX)s-{}".format(self.V_PRODUCT_ID_VERSION(data))

    def V_FNAME_PRODUCT_VERSION(self, data):
        return "%(PREFIX)s-{}".format(self.V_PRODUCT_ID_VERSION(data))

    def V_VERSION_STRING(self, data):
        return "{} {}, {}".format(self.V_OS_NAME(data), self.V_VERSION_ID(data), self.V_BUILD_ID(data))

    def V_RELEASE_ID(self, data):
        return "{},{}".format(self.V_VERSION_ID(data), self.V_BUILD_ID(data))

    def V_FNAME_RELEASE_ID(self, data):
        return "{}-{}".format(self.V_VERSION_ID(data), self.V_FNAME_BUILD_ID(data))

    def V_SYSTEM_COMPATIBILITY_VERSION(self, data):
        return "2"

    def V_ISSUE(self, data):
        if self.release:
            return "{} {}".format(self.V_OS_NAME(data), self.V_VERSION_ID(data))
        else:
            return self.V_VERSION_STRING(data)
"""

class tools_gitignore(CommonFile):
    template="""*.pyc"""

class packages_base_amd64_upgrade_PKG(ArchFileGrub):
    template="""
prerequisites:
    packages:
      - onl-kernel-3.16-lts-x86-64-all:amd64
      - onl-kernel-4.9-lts-x86-64-all:amd64
      - onl-kernel-4.14-lts-x86-64-all:amd64
      - %(prefix)s-loader-initrd:amd64

packages:
  - name: %(prefix)s-upgrade
    arch: amd64
    %(COMMON)s
    summary: %(PREFIX)s Upgrade Package

    files:
      builds/files : /etc/%(prefix)s/upgrade/amd64
"""

class packages_base_amd64_upgrade_builds_gitignore(ArchFileGrub):
    template="""files"""

class packages_base_amd64_upgrade_Makefile(ArchFileGrub,OnlPkgMakefile):
    pass


class packages_base_amd64_upgrade_builds_Makefile(ArchFileGrub):
    template="""
include %(MTOP)s/make/config.amd64.mk

# All amd64 kernels
KERNELS := $(shell $(ONLPM) --find-file onl-kernel-3.16-lts-x86-64-all:amd64 kernel-3.16-lts-x86_64-all) \\
           $(shell $(ONLPM) --find-file onl-kernel-4.9-lts-x86-64-all:amd64 kernel-4.9-lts-x86_64-all) \\
           $(shell $(ONLPM) --find-file onl-kernel-4.14-lts-x86-64-all:amd64 kernel-4.14-lts-x86_64-all)


# Loader initrd
INITRD := $(shell $(ONLPM) --find-file %(prefix)s-loader-initrd:amd64 %(prefix)s-loader-initrd-amd64.cpio.gz)
MANIFEST := $(shell $(ONLPM) --find-file %(prefix)s-loader-initrd:amd64 manifest.json)

all:
\tmkdir -p files
\tcp $(KERNELS) files
\tcp $(INITRD) files
\tcp $(MANIFEST) files

"""

class setup(CommonFile):
    path='setup.env'
    template="""
#!/bin/bash
############################################################
#
# The settings in this script are required
# and should be sourced into your local build shell.
#
############################################################

# The root of the %(PREFIX)s build tree is here
export %(PREFIX)s=$( cd "$(dirname "${BASH_SOURCE[0]}" )" && pwd)

# The root of the ONL tree is here
export ONL=%(TOP)s/sm/ONL

# Checkout ONL if necessary
if [ ! -f $ONL/LICENSE ]; then
    git submodule update --init sm/ONL
    # Versions and setup.
    (cd sm/ONL && . setup.env)
fi

# All package directories.
export ONLPM_OPTION_PACKAGEDIRS="$ONL/packages:$ONL/builds:%(TOP)s/packages:%(TOP)s/builds"

# Repo directory.
export ONLPM_OPTION_REPO="%(TOP)s/REPO"

# RELEASE directory.
export ONLPM_OPTION_RELEASE_DIR="%(TOP)s/RELEASE"

# The ONL build tools should be included in the local path:
export PATH="$ONL/tools/scripts:$ONL/tools:$PATH"

# Parallel Make Jobs
# Default parallel build settings
export ONL_MAKE_PARALLEL=-j16

# Export the current debian suite
export ONL_DEBIAN_SUITE=$(lsb_release -c -s)

export BUILDER_MODULE_DATABASE_ROOT=%(TOP)s

# Version files
$ONL/tools/make-versions.py --force --import-file=%(TOP)s/tools/%(prefix)svi --class-name=OnlVersionImplementation --output-dir %(TOP)s/make/versions
( . %(TOP)s/make/versions/version-%(prefix)s.sh && echo BuildID is $FNAME_BUILD_ID )

# Enable local post-merge githook
if [ ! -f %(TOP)s/.git/hooks/post-merge ]; then
    cp $ONL/tools/scripts/post-merge.hook %(TOP)s/.git/hooks/post-merge
fi

export ONL_SUBMODULE_UPDATED_SCRIPTS="%(TOP)s/tools/scripts/submodule-updated.sh:$ONL/tools/scripts/submodule-updated.sh"

# Update %(PREFIX)s REPO from ONL build-artifacts
cp -R $ONL/sm/build-artifacts/REPO/* %(TOP)s/REPO



"""

class builds_any_rootfs_APKG(CommonFile):
    template="""
variables:
  !include %(TOP)s/make/versions/version-%(prefix)s.yml

prerequisites:
  broken: true

packages:
  - name: %(prefix)s-rootfs
    summary: %(name)s Root Filesystem
    arch: $ARCH
    version: 0.$FNAME_RELEASE_ID
    %(COMMON_NV)s

    files:
      builds/rootfs-$ARCH.cpio.gz : $$PKG_INSTALL/
      builds/rootfs-$ARCH.sqsh : $$PKG_INSTALL/
      builds/manifest.json : $$PKG_INSTALL/

"""

class packages_base_any_fit_loader_builds_Makefile(CommonFile):
    template="""
ifndef ARCH
$(error $$ARCH must be set)
endif

.PHONY: %(prefix)s-loader-fit.itb %(prefix)s-loader-fit.its

%(prefix)s-loader-fit.itb:
\t$(ONL)/tools/flat-image-tree.py --initrd %(prefix)s-loader-initrd:$(ARCH),%(prefix)s-loader-initrd-$(ARCH).cpio.gz --arch $(ARCH) --add-platform initrd --itb $@
\t$(ONLPM) --copy-file %(prefix)s-loader-initrd:$(ARCH) manifest.json .

%(prefix)s-loader-fit.its:
    $(ONL)/tools/flat-image-tree.py --initrd %(prefix)s-loader-initrd:$(ARCH),%(prefix)s-loader-initrd-$(ARCH).cpio.gz --arch $(ARCH) --add-platform initrd --its $@

its: %(prefix)s-loader-fit.itspass
"""


class packages_base_any_fit_loader_APKG(CommonFile):
    template="""
prerequisites:
  packages:
    - %(prefix)s-loader-initrd:$ARCH

packages:
  - name: %(prefix)s-loader-fit
    arch: $ARCH
    %(COMMON)s
    summary: %(name)s FIT Loader Image for $ARCH

    files:
      builds/%(prefix)s-loader-fit.itb : /etc/%(prefix)s/upgrade/$ARCH/
      builds/manifest.json      : /etc/%(prefix)s/upgrade/$ARCH/


"""

class packages_base_arch_fit_Makefile(ArchFileUboot,OnlPkgMakefile):
    pass

class packages_base_arch_fit_loader_PKG(ArchFileUboot):
    template="""
!include %(TOP)s/packages/base/any/fit/loader/APKG.yml ARCH=%(arch)s
"""

class packages_base_arch_fit_loader_builds_gitignore(ArchFileUboot):
    template="""
kernel-*
*.itb
*.its
loader-initrd-powerpc
manifest.json
"""

class packages_base_arch_fit_loader_builds_Makefile(ArchFileUboot):
    template="""
include %(MTOP)s/make/config.%(arch)s.mk
include %(MTOP)s/packages/base/any/fit/loader/builds/Makefile
"""

class packages_base_arch_fit_loader_Makefile(ArchFileUboot,OnlPkgMakefile):
    pass

class builds_gitignore(CommonFile):
    template="""
*.swi
*.md5sum
"""

class builds_arch_Makefile(ArchFile):
    template="""
include $(ONL)/make/arch-build.mk
"""

class builds_arch_installer_Makefile(ArchFile,OnlPkgMakefile):
    pass

class builds_arch_installer_installed_Makefile(ArchFile,OnlPkgMakefile):
    pass


class builds_arch_installer_installed_PKG(ArchFile):
    template="""
!include %(TOP)s/builds/any/installer/APKG.yml ARCH=%(arch)s BOOTMODE=installed
"""

class builds_arch_installer_installed_builds_gitignore(ArchFile):
    template="""*INSTALLER"""

class builds_arch_installer_installed_builds_Makefile(ArchFile):
    template="""
BOOTMODE=INSTALLED
include %(MTOP)s/make/config.%(arch)s.mk
include %(MTOP)s/builds/any/installer/builds/Makefile
"""

class builds_arch_installer_installed_builds_bootconfig(ArchFile):
    template="""
NETDEV=ma1
BOOTMODE=INSTALLED
SWI=images::latest
"""

class builds_arch_installer_swi_Makefile(ArchFile,OnlPkgMakefile):
    pass

class builds_arch_installer_swi_PKG(ArchFile):
    template="""
!include %(TOP)s/builds/any/installer/APKG.yml ARCH=%(arch)s BOOTMODE=swi
"""

class builds_arch_installer_swi_builds_gitignore(ArchFile):
    template="""*INSTALLER"""

class builds_arch_installer_swi_builds_Makefile(ArchFile):
    template="""
BOOTMODE=SWI
include %(MTOP)s/make/config.%(arch)s.mk
include %(MTOP)s/builds/any/installer/builds/Makefile
"""


class builds_arch_installer_swi_builds_bootconfig(ArchFile):
    template="""
NETDEV=ma1
BOOTMODE=SWI
SWI=images::latest
"""

class builds_arch_rootfs_gitignore(ArchFile):
    template="""*.d/"""

class builds_arch_rootfs_Makefile(ArchFile,OnlPkgMakefile):
    pass

class builds_arch_rootfs_PKG(ArchFile):
    template="""
!include %(TOP)s/builds/any/rootfs/APKG.yml ARCH=%(arch)s
"""

class builds_arch_rootfs_builds_gitignore(ArchFile):
    template="""manifest.json"""

class builds_arch_rootfs_builds_Makefile(ArchFile):
    template="""
include %(MTOP)s/make/config.%(arch)s.mk

export PLATFORM_LIST=$(shell onlpm --list-platforms --arch %(arch)s --csv )

RFS_CONFIG := %(MTOP)s/builds/any/rootfs/rfs.yml
RFS_DIR := rootfs-%(arch)s.d
RFS_CPIO := rootfs-%(arch)s.cpio.gz
RFS_SQUASH := rootfs-%(arch)s.sqsh
RFS_MANIFEST := /etc/%(prefix)s/rootfs/manifest.json

include $(ONL)/make/rfs.mk
"""

class builds_arch_swi_Makefile(ArchFile,OnlPkgMakefile):
    pass

class builds_arch_swi_PKG(ArchFile):
    template="""
!include %(TOP)s/builds/any/swi/APKG.yml ARCH=%(arch)s
"""

class builds_arch_swi_builds_gitignore(ArchFile):
    template="""manifest.json"""

class builds_arch_swi_builds_Makefile(ArchFile):
    template="""
ROOTFS_PACKAGE := %(prefix)s-rootfs
include %(MTOP)s/make/config.%(arch)s.mk
include $(ONL)/make/swi.mk
"""

class builds_any_installer_gitignore(CommonFile):
    template="installer.sh.in"

class builds_any_installer_APKG(CommonFile):
    template="""
variables:
  !include %(TOP)s/make/versions/version-%(prefix)s.yml

prerequisites:
  broken: true
  packages: [ "%(prefix)s-swi:$ARCH" ]

packages:
  - name: %(prefix)s-installer-$BOOTMODE
    summary: %(name)s $ARCH Installer
    arch: $ARCH
    version: 0.$FNAME_RELEASE_ID
    %(COMMON_NV)s

    files:
      builds/*INSTALLER        : $$PKG_INSTALL/
      builds/*.md5sum          : $$PKG_INSTALL/


release:
  - builds/*INSTALLER : $ARCH/
  - builds/*.md5sum   : $ARCH/
"""

class builds_any_installer_builds_Makefile(CommonFile):
    template="""
ifndef ARCH
$(error $$ARCH not set)
endif

ifndef BOOTMODE
$(error $$BOOTMODE not set)
endif

# Hardcoded to match ONL File naming conventions.
include %(MTOP)s/make/versions/version-%(prefix)s.mk
INSTALLER_NAME=$(FNAME_PRODUCT_VERSION)_ONL-OS_$(FNAME_BUILD_ID)_$(UARCH)_$(BOOTMODE)_INSTALLER

MKINSTALLER_OPTS := \
  --arch $(ARCH) \
  --boot-config boot-config \
  --swi %(prefix)s-swi:$(ARCH)


ifeq ($(ARCH_BOOT),uboot)
    MKINSTALLER_OPTS += --fit %(prefix)s-loader-fit:$(ARCH) %(prefix)s-loader-fit.itb
else ifeq ($(ARCH_BOOT),grub)
    MKINSTALLER_OPTS += --initrd %(prefix)s-loader-initrd:$(ARCH) %(prefix)s-loader-initrd-$(ARCH).cpio.gz
else
    $(error ARCH_BOOT=$(ARCH_BOOT) not recognized.)
endif

__installer:
\t$(ONL)/tools/mkinstaller.py $(MKINSTALLER_OPTS) --out $(INSTALLER_NAME)
\tmd5sum "$(INSTALLER_NAME)" | awk '{ print $$1 }' > "$(INSTALLER_NAME).md5sum"
"""


class builds_any_rootfs_APKG(CommonFile):
    template="""
variables:
  !include %(TOP)s/make/versions/version-%(prefix)s.yml

prerequisites:
  broken: true

packages:
  - name: %(prefix)s-rootfs
    summary: %(name)s Root Filesystem
    arch: $ARCH
    version: 0.$FNAME_RELEASE_ID
    %(COMMON_NV)s

    files:
      builds/rootfs-$ARCH.cpio.gz : $$PKG_INSTALL/
      builds/rootfs-$ARCH.sqsh : $$PKG_INSTALL/
      builds/manifest.json : $$PKG_INSTALL/
"""



class builds_any_swi_APKG(CommonFile):
    template="""
variables:
  !include %(TOP)s/make/versions/version-%(prefix)s.yml

prerequisites:
  broken: true
  packages: [ "%(prefix)s-rootfs:$ARCH" ]

packages:
  - name: %(prefix)s-swi
    summary: %(name)s Switch Image (All $ARCH) Platforms)
    arch: $ARCH
    version: 0.$FNAME_RELEASE_ID
    %(COMMON_NV)s

    files:
      builds/*.swi          : $$PKG_INSTALL/
      builds/*.md5sum       : $$PKG_INSTALL/
      builds/manifest.json  : $$PKG_INSTALL/

release:
  - builds/*.swi : $ARCH/
  - builds/*.md5sum : $ARCH/

"""

class builds_any_rootfs_all_packages(CommonFile):
    path='builds/any/rootfs/all-packages.yml'
    template="""
- %(prefix)s-sysconfig
- %(prefix)s-hw
"""


class builds_any_rootfs_archgrub_packages(ArchFileGrub):
    path="builds/any/rootfs/%(arch)s-packages.yml"
    template="""
- %(prefix)s-upgrade
"""

class builds_any_rootfs_archuboot_packages(ArchFileUboot):
    path="builds/any/rootfs/%(arch)s-packages.yml"
    template="""
- %(prefix)s-loader-fit
"""

class builds_any_rootfs_rfs(CommonFile):
    path="builds/any/rootfs/rfs.yml"
    template="""
variables:
  !include %(TOP)s/make/versions/version-%(prefix)s.yml

Packages: &Packages
  - !include $ONL/builds/any/rootfs/$ONL_DEBIAN_SUITE/common/all-base-packages.yml
  - !include $ONL/builds/any/rootfs/$ONL_DEBIAN_SUITE/common/${ARCH}-base-packages.yml
  - !include $ONL/builds/any/rootfs/$ONL_DEBIAN_SUITE/common/${ARCH}-onl-packages.yml
  - !script  $ONL/tools/onl-platform-pkgs.py ${PLATFORM_LIST}
  - !include %(TOP)s/builds/any/rootfs/all-packages.yml
  - !include %(TOP)s/builds/any/rootfs/${ARCH}-packages.yml

Multistrap:
  General:
    arch: ${ARCH}
    cleanup: true
    noauth: true
    explicitsuite: false
    unpack: true
    debootstrap: Debian-Local Local-All Local-Arch ONL-Local
    aptsources: Debian ONL

  Debian:
    packages: *Packages
    source: http://${DEBIAN_MIRROR}
    suite: ${ONL_DEBIAN_SUITE}
    keyring: debian-archive-keyring
    omitdebsrc: true

  Debian-Local:
    packages: *Packages
    source: http://${APT_CACHE}${DEBIAN_MIRROR}
    suite: ${ONL_DEBIAN_SUITE}
    keyring: debian-archive-keyring
    omitdebsrc: true

  ONL:
    packages: *Packages
    source: http://apt.opennetlinux.org/debian
    suite: unstable
    omitdebsrc: true

  ONL-Local:
    packages: *Packages
    source: http://${APT_CACHE}apt.opennetlinux.org/debian
    suite: unstable
    omitdebsrc: true

  Local-All:
    source: ${ONLPM_OPTION_REPO}/${ONL_DEBIAN_SUITE}/packages/binary-all
    omitdebsrc: true

  Local-Arch:
    source: ${ONLPM_OPTION_REPO}/${ONL_DEBIAN_SUITE}/packages/binary-${ARCH}
    omitdebsrc: true

Configure:
  overlays:
    - $ONL/builds/any/rootfs/${ONL_DEBIAN_SUITE}/common/overlay

  update-rc.d:
    - 'faultd defaults'
    - 'onlpd defaults'
    - 'snmpd defaults'
    - 'onlp-snmpd defaults'
    - 'ssh defaults'
    - 'openbsd-inetd remove'
    - 'ntp remove'
    - 'nfs-common remove'
    - 'rpcbind remove'
    - 'motd remove'
    - 'mountall-bootclean.sh remove'
    - 'mountall.sh remove'
    - 'checkfs.sh remove'
    - 'mtab.sh remove'
    - 'checkroot-bootclean.sh remove'
    - 'checkroot.sh remove'
    - 'mountnfs-bootclean.sh remove'
    - 'mountnfs.sh remove'
    - 'lm-sensors remove'
    - 'netplug defaults'
    - 'watchdog defaults'
    - 'wd_keepalive remove'

  options:
    clean: True
    securetty: False
    ttys: False
    console: True
    PermitRootLogin: 'yes'

  users:
    root:
      password: %(prefix)s

  manifests:
    '/etc/onl/rootfs/manifest.json' :
      version : $ONL/make/versions/version-onl.json
      platforms : $PLATFORM_LIST

    '/etc/%(prefix)s/rootfs/manifest.json' :
      version : %(TOP)s/make/versions/version-%(prefix)s.json
      platforms : $PLATFORM_LIST
      keys:
        version :
          CONFIGURATION: RELEASE

  issue: $VERSION_STRING

  files:
    remove:
      - /etc/motd
"""

ALL_FILE_CLASSES = [ klass for klass in inspect.getmembers(sys.modules[__name__], inspect.isclass) if issubclass(klass[1], NOSFile) and klass[1].is_file() ]
ALL_COMMON_CLASSES = [ klass for klass in ALL_FILE_CLASSES if issubclass(klass[1], CommonFile) ]
ALL_ARCH_CLASSES = [ klass for klass in ALL_FILE_CLASSES if issubclass(klass[1], ArchFile) ]


if __name__ == '__main__':

    ops = ap.parse_args()
    variables = {}
    variables.update(vars(ops))
    variables['PREFIX'] = variables['prefix'].upper()

    # Create all required common and arch objects
    OBJECTS = []
    for klass in ALL_COMMON_CLASSES:
        OBJECTS.append(klass[1](ops.root, variables.copy()))

    for arch in ops.arches:
        for klass in ALL_ARCH_CLASSES:
            if arch in klass[1].arches:
                OBJECTS.append(klass[1](ops.root, variables.copy(), arch))


    for distro in ops.debian:
        OBJECTS.append(NOSFile(ops.root, variables.copy(),
                               path='REPO/%s/Makefile' % distro,
                               template='include $(ONL)/make/repo-suite.mk'))

        OBJECTS.append(NOSFile(ops.root, variables.copy(),
                               path='REPO/%s/packages/binary-all/Makefile' % distro,
                               template='include $(ONL)/make/repo.mk'))
        for arch in ops.arches:
            OBJECTS.append(NOSFile(ops.root, variables.copy(),
                                   path='REPO/%s/packages/binary-%s/Makefile' % (distro, arch),
                                   template="include $(ONL)/make/repo.mk"))



    OBJECTS = sorted(OBJECTS,key=lambda entry: entry.epath)

    for obj in OBJECTS:
        if ops.list_files:
            print "%-60s" % (obj.epath)
        if ops.write_files:
            obj.write(overwrite=ops.overwrite, dry=ops.dry)
