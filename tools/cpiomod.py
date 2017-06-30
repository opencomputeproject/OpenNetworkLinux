#!/usr/bin/python2
import sys
import os
import argparse
import subprocess
import shutil
import tempfile

ap = argparse.ArgumentParser(description="CPIO Modify Tool.")

ap.add_argument("--cpio", help="Input cpio gzip", required=True)
ap.add_argument("--add-directory", nargs='+', help="Add the given directory to the root of the cpio.", default=[])
ap.add_argument("--makedevs", nargs='+', help="Run makedevs", default=[])
ap.add_argument("--ls", action='store_true', help="List files in CPIO and exit.")
ap.add_argument("--out", help="New CPIO")

ops = ap.parse_args()

#
# This is all pretty hacky right now
#

class CpioManager(object):
    def __init__(self):
        pass

    def __denit__(self):
        self.close(None)

    def open(self, cpio):
        self.dir = tempfile.mkdtemp()
        if os.system("cd %s && gzip -dc %s | sudo cpio -id" % (
                self.dir, os.path.abspath(cpio))) != 0:
            raise Exception("Could not unpack cpio %s" % cpio)

    def add_directory(self, directory):
        if not os.path.isdir(directory):
            raise Exception("Directory %s does not exist" % directory)

        if os.system("tar -c --exclude '.*~' -C %s . | sudo tar -x -C %s" % (directory, self.dir)) != 0:
            raise Exception("Could not add directory %s" % directory)

    def makedevs(self, devfile):
        if os.system("sudo %s -d %s %s" % (os.path.join(os.getenv('SWITCHLIGHT'), "tools", "makedevs"), os.path.abspath(devfile), self.dir)) != 0:
            raise Exception("Could not run makedevs")

    def close(self, ncpio):
        if ncpio:
            os.system("cd %s && find . | sudo cpio -H newc -o | gzip -f > %s" % (self.dir, os.path.abspath(ncpio)))
        os.system("sudo rm -rf %s" % (self.dir))
        self.dir = None
        
    def list(self):
        os.system("cd %s && find . -exec ls -l {} \; " % (self.dir))

cm = CpioManager()
cm.open(ops.cpio)

if ops.ls:
    cm.list()
    sys.exit(0)

for d in ops.add_directory:
    cm.add_directory(d)

for md in ops.makedevs:
    cm.makedevs(md)

cm.close(ops.out)

        
        

                     
