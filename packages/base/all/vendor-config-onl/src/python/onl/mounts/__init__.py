#!/usr/bin/python
import os
import sys
import platform
import subprocess
import logging
import time
import json
import yaml
import tempfile
import shutil

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
                    self.logger.error("Could not unmount %s @ %s: %s" % (k, v['device'], e.output))
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



############################################################
#
# Fix this stuff
#
############################################################
class ServiceMixin(object):

    def _execute(self, cmd, root=False, ex=True):
        self.logger.debug("Executing: %s" % cmd)
        if root is True and os.getuid() != 0:
            cmd = "sudo " + cmd
        try:
            subprocess.check_call(cmd, shell=True)
        except Exception, e:
            if ex:
                self.logger.error("Command failed: %s" % e)
                raise
            else:
                return e.returncode

    def _raise(self, msg, klass):
        self.logger.critical(msg)
        raise klass(msg)

class DataMount(ServiceMixin):

    def __init__(self, partition, logger=None):
        self.partition = partition
        self.logger = logger

        self.mountpoint = None
        self.mounted = False

        if os.path.isabs(partition) and not os.path.exists(partition):
            # Implicitly a bind mount. It may not exist yet, so create it
            os.makedirs(partition)

        if os.path.exists(partition):
            # Bind mount
            self.device = None
        else:
            self.device = subprocess.check_output("blkid | grep %s | awk '{print $1}' | tr -d ':'" % self.partition, shell=True).strip()
            if self.device is None or len(self.device) is 0:
                self._raise("Data partition %s does not exist." % self.partition,
                            RuntimeError)
        self.logger.debug("device is %s" % self.device)

    def _mount(self):
        if self.device:
            self._execute("mount %s %s" % (self.device, self.mountdir()), root=True)
        else:
            self._execute("mount --bind %s %s" % (self.partition,
                                                  self.mountdir()), root=True)
        self.mounted = True

    def _umount(self):
        mounted, self.mounted = self.mounted, False
        if mounted:
            self._execute("umount %s" % self.mountpoint, root=True)
        mountpoint, self.mountpoint = self.mountpoint, None
        if mountpoint and os.path.exists(mountpoint):
            self.logger.debug("+ /bin/rmdir %s", mountpoint)
            os.rmdir(mountpoint)

    def __enter__(self):
        self._mount()
        return self

    def __exit__(self, type_, value, traceback):
        self._umount()

    def mountdir(self):
        if self.mountpoint is None:
            self.mountpoint = tempfile.mkdtemp(prefix="pki-", suffix=".d")
            self.logger.debug("mountpoint is %s" % self.mountpoint)
        return self.mountpoint


class OnlDataStore(ServiceMixin):

    # Data partition containing the persistant store
    DATA_PARTITION='ONL-CONFIG'

    # Persistant directory on DATA_PARTITION
    P_DIR=None

    # Runtime directory in the root filesystem
    R_DIR=None

    def __init__(self, logger=None):

        if logger is None:
            logging.basicConfig()
            logger = logging.getLogger(str(self.__class__))
            logger.setLevel(logging.WARN)

        self.logger = logger

        self.mount = DataMount(self.DATA_PARTITION, logger=self.logger)

        if self.P_DIR is None:
            raise AttributeError("P_DIR must be set in the derived class.")

        if self.R_DIR is None:
            raise ValueError("R_DIR must be set in the derived class.")

        # The R_DIR is accessed here
        self.r_dir = self.R_DIR

        self.logger.debug("persistant dir: %s" % self.p_dir)
        self.logger.debug("   runtime dir: %s" % self.r_dir)

    @property
    def p_dir(self):
        return os.path.join(self.mount.mountdir(), self.P_DIR)

    def _sync_dir(self, src, dst):
        self.logger.debug("Syncing store from %s -> %s" % (src, dst))
        if os.path.exists(dst):
            shutil.rmtree(dst)

        if not os.path.exists(src):
            os.makedirs(src)

        shutil.copytree(src, dst)

    def init_runtime(self):
        with self.mount:
            self._sync_dir(self.p_dir, self.r_dir)

    def commit_runtime(self):
        with self.mount:
            self._sync_dir(self.r_dir, self.p_dir)

    def diff(self):
        with self.mount:
            rv = self._execute("diff -rNq %s %s" % (self.p_dir, self.r_dir), ex=False)
        return rv == 0

    def ls(self):
        with self.mount:
            self._execute("cd %s && find ." % (self.p_dir))

    def rm(self, filename):
        with self.mount:
            os.unlink(os.path.join(self.p_dir, filename))
        os.unlink(os.path.join(r_dir, filename))


