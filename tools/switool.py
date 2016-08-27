#!/usr/bin/python
############################################################
import argparse
import sys
import os
import zipfile
import json
import apt_inst
import onlu

logger = onlu.init_logging('switool')

class OnlSwitchImage(object):

    def __init__(self, fname, mode):
        self.fname = fname
        self.mode = mode
        self.zipfile = zipfile.ZipFile(fname, mode=mode)
        self.manifest = None

    def add(self, fname, arcname=None, compressed=True):
        self.zipfile.write(fname, arcname=arcname, compress_type = zipfile.ZIP_DEFLATED if compressed else zipefile.ZIP_STORED)

    def add_rootfs(self, rootfs_sqsh):
        self.add(rootfs_sqsh)

    def add_manifest(self, manifest):
        self.add(manifest, arcname="manifest.json")

    def get_manifest(self):
        if 'manifest.json' in self.zipfile.namelist():
            return json.load(self.zipfile.open('manifest.json'))
        else:
            return None

    def get_arch(self):
        return self.get_manifest()['arch']

    def get_platforms(self):
        p = self.get_manifest()['platforms']
        if type(p) is list:
            return p
        else:
            return p.split(',')

    def get_contents(self):
        return self.zipfile.namelist()

############################################################

ap = argparse.ArgumentParser(description="SWI Tool")
ap.add_argument('--create', action='store_true', help='Create new SWI.')
ap.add_argument('--overwrite', action='store_true', help='Overwrite existing file.')
ap.add_argument('--rootfs', help='Root SquashFS File')
ap.add_argument('--manifest', help='SWI Manifest file.')
ap.add_argument('--add-files', help='Add additional files.', default=[], nargs='+')
ap.add_argument("--contents", help='Show SWI contents.', action='store_true')
ap.add_argument("--platforms", help='Show SWI contents.', action='store_true')
ap.add_argument('swi', help='SWI image name.')

ops = ap.parse_args()

if os.path.exists(ops.swi):
    if ops.create and not ops.overwrite:
        logger.critical("File '%s' exists." % ops.swi)
        sys.exit(1)

swi = None

if ops.create or ops.overwrite:
    if not ops.rootfs:
        logger.critical("Rootfs required to create new SWI.")
        sys.exit(1)

    if not ops.manifest:
        logger.critical("Manifest required to create new SWI.")
        sys.exit(1)

    swi = OnlSwitchImage(ops.swi, 'w')
    swi.add_rootfs(ops.rootfs)
    swi.add_manifest(ops.manifest)
    for f in ops.add_files:
        swi.add(f, arcname=f)

if swi is None:

    if not os.path.exists(ops.swi):
        logger.critical("SWI file %s does not exist." % ops.swi)
        sys.exit(1)

    swi = OnlSwitchImage(ops.swi, 'r')

if ops.contents:
    print " ".join(swi.get_contents())

if ops.platforms:
    print " ".join(swi.get_platforms())
