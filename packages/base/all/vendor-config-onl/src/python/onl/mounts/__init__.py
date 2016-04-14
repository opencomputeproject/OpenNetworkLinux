#!/usr/bin/python
import os
import sys
import platform
import subprocess
import logging
import time
import json
import yaml

class OnlMountManager(object):
    def __init__(self, mdata, logger):

        if os.path.exists(mdata):
            mdata = yaml.load(open(mdata, "r"));

        self.mdata = mdata
        self.logger = logger

        # Needed to avoid ugly warnings from fsck
        if not os.path.exists('/etc/mtab'):
            open("/etc/mtab", 'w').close();


    def checkmount(self, directory):
        with open("/proc/mounts") as f:
            return directory in f.read()

    def mount(self, device, directory, mode='r', timeout=5):
        self.logger.debug("Mounting %s -> %s %s" % (device, directory, mode))
        try:
            subprocess.check_call("mount -%s %s %s" % (mode, device, directory), shell=True)
        except subrocess.CalledProcessError, e:
            self.logger("Mount failed: '%s'" % e.output)
            return False

        # If requested, wait for the mount to complete.
        while(timeout > 0):
            if self.checkmount(directory):
                break
            time.sleep(1)
            timeout-=1

        if self.checkmount(directory):
            self.logger.info("%s is now mounted @ %s" % (device, directory))
            return True
        else:
            self.logger.info("%s failed to report in /proc/mounts." % (directory))



    def mountall(self, all_=False, fsck=None, timeout=5):
        for (k, v) in self.mdata['mounts'].iteritems():

            #
            # Make the mount point for future use.
            #
            if not os.path.isdir(v['dir']):
                self.logger.debug("Make directory '%s'..." % v['dir'])
                os.makedirs(v['dir'])

            #
            # Get the partition device.
            # The timeout logic is here to handle waiting for the
            # block devices to arrive at boot.
            #
            while timeout > 0:
                try:
                    v['device'] = subprocess.check_output("blkid -L %s" % k, shell=True).strip()
                    break
                except subprocess.CalledProcessError:
                    self.logger.debug("Block label %s does not yet exist..." % k)
                    time.sleep(1)
                    timeout -= 1

            if 'device' not in v:
                self.logger.error("Timeout waiting for block label %s after %d seconds." % (k, timeout))
                continue;

            self.logger.debug("%s @ %s" % (k, v['device']))

            #
            # If its currently mounted we should unmount first.
            #
            if self.checkmount(v['device']):
                self.logger.info("%s is currently mounted." % (k))
                try:
                    out = subprocess.check_output("umount %s" % v['device'], shell=True)
                    self.logger.info("%s now unmounted." % (k))
                except subprocess.CalledProcessError,e:
                    self.logger.error("Could not unmount %s @ %s: " % (k, v['device'], e.output))
                    continue
            #
            # FS Checks
            #
            if fsck is not None:
                # Override fsck setting with given value
                self.logger.debug("Overriding fsck settings for %s with %s" % (k, fsck))
                v['fsck'] = fsck

            if v.get('fsck', False):
                try:
                    self.logger.info("Running fsck on %s [ %s ]..." % (k, v['device']))
                    cmd = "fsck.ext4 -p %s" % (v['device'])
                    self.logger.debug(cmd)
                    try:
                        out = subprocess.check_output(cmd, shell=True)
                        self.logger.info("%s [ %s ] is clean." % (v['device'], k))
                    except subprocess.CalledProcessError, e:
                        self.logger.error("fsck failed: %s" % e.output)
                except subprocess.CalledProcessError, e:
                    # Todo - recovery options
                    raise


            if all_:
                v['mount'] = 'w'

            mount = v.get('mount', None)
            if mount:
                if mount in ['r', 'w']:
                    self.mount(v['device'], v['dir'], mode=mount, timeout=v.get('timeout', 5))
                else:
                    self.logger("Mount %s has an invalid mount mode (%s)" % (k, mount))


    @staticmethod
    def main():
        import argparse

        logging.basicConfig()

        ap = argparse.ArgumentParser(description="ONL Mount Manager.");
        ap.add_argument("--mtab", default="/etc/mtab.yml")
        ap.add_argument("--rw", action='store_true')
        ap.add_argument("--verbose", "-v", action='store_true')
        ap.add_argument("--quiet", "-q", action='store_true')

        ops = ap.parse_args();

        logger = logging.getLogger("initmounts")
        if ops.verbose:
            logger.setLevel(logging.DEBUG)
        elif ops.quiet:
            logger.setLevel(logging.ERROR)
        else:
            logger.setLevel(logging.INFO)

        mm = OnlMountManager(ops.mtab, logger)
        if ops.rw:
            mm.mountall(all_=True, fsck=False)
        else:
            mm.mountall()
