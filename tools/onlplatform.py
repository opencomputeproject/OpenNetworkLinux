#!/usr/bin/python

"""onlplatform.py

Extract install file requirements from the platform YAML file and/or
the platform package metadata.
"""

import sys, os
import itertools
import argparse

toolsdir = os.path.dirname(os.path.abspath(__file__))
sys.path.append(toolsdir)

onldir = os.path.dirname(toolsdir)
onlpydir = os.path.join(onldir, "packages/base/all/vendor-config-onl/src/python")
sys.path.append(onlpydir)

import onl.YamlUtils

from onlpm import *
# glob import is required here so pickle load load properly

pm = defaultPm()

ap = argparse.ArgumentParser("ONL Platform Data Extractor.")
ap.add_argument("platform", help="Platform name")
ap.add_argument("arch", help="Architecture")
ap.add_argument("key", help="Lookup key.")
ops = ap.parse_args()

def extractKey(platform, arch, key):

    pkg = "onl-platform-config-%s:%s" % (platform, arch,)
    basename = "%s.yml" % platform
    pm.require(pkg, force=False, build_missing=False)
    platformConfigPath = pm.opr.get_file(pkg, basename)

    if arch in ('amd64',):
        pkg = "onl-vendor-config-onl:all"
        basename = "platform-config-defaults-x86-64.yml"
        subkey = 'grub'
    else:
        pkg = "onl-vendor-config-onl:all"
        basename = "platform-config-defaults-uboot.yml"
        subkey = 'flat_image_tree'
    pm.require(pkg, force=False, build_missing=False)
    defaultConfigPath = pm.opr.get_file(pkg, basename)

    platformConf = onl.YamlUtils.merge(defaultConfigPath, platformConfigPath)
    resource = platformConf[platform][subkey][key]
    if type(resource) == dict:
        pkg = resource['package']
        basename = resource['=']
    else:
        pkg, sep, basename = resource.partition(',')
        if not sep:
            raise ValueError("resource missing package declaration: %s" % resource)
        pkg = pkg.strip()
        basename = basename.strip()
    pm.require(pkg, force=False, build_missing=False)
    resourcePath = pm.opr.get_file(pkg, basename)
    return resourcePath

def extractVendor(platform, arch):
    pkg = "onl-platform-config-%s:%s" % (platform, arch,)
    l = pm.opr.lookup_all(pkg)
    if not l:
        raise SystemExit("cannot find package %s:%s"
                         % (platform, arch,))
    l = [x for x in pm.package_groups if pkg in x]
    l = list(itertools.chain(*[x.prerequisite_packages() for x in l]))
    l = [x for x in l if x.startswith('onl-vendor-config-')]
    return "\n".join(l)

if ops.key in ('kernel', 'initrd', 'dtb', 'itb',):
    print extractKey(ops.platform, ops.arch, ops.key)
    sys.exit(0)

if ops.key == 'vendor':
    print extractVendor(ops.platform, ops.arch)
    sys.exit(0)

raise SystemExit("invalid key %s" % ops.key)
