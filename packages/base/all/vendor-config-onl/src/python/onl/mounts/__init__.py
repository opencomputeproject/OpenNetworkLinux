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

class MountManager(object):

    def __init__(self, logger):
        self.read_proc_mounts()
        self.logger = logger
        if self.logger is None:
            self.logger = logging.getLogger('onl:mounts')

    def read_proc_mounts(self):
        self.mounts = {}
        with open('/proc/mounts') as mounts:
            for line in mounts.readlines():
                (dev, dir_, type_, options, a, b) = line.split()
                self.mounts[dir_] = dict(dev=dev, mode=options.split(',')[0])

    def is_mounted(self, device, directory):
        self.read_proc_mounts()
        if directory in self.mounts and self.mounts[directory]['dev'] == device:
            return self.mounts[directory]
        return None

    def is_dev_mounted(self, device):
        self.read_proc_mounts()
        for (k, v) in self.mounts.iteritems():
            if v['dev'] == device:
                return True
        return False

    def mount(self, device, directory, mode='r', timeout=5):

        mountargs = [ str(mode) ]
        currentItems = [x for x in self.mounts.iteritems() if x[1]['dev'] == device]
        if currentItems:
            currentDirectory, current = currentItems[0]
            if current['mode'] == mode:
                # Already mounted as requested.
                self.logger.debug("%s already mounted @ %s with mode %s. Doing nothing." % (device, directory, mode))
                return True
            elif directory != currentDirectory:
                # Already mounted, at a different location (e.g. '/'), but not in the requested mode.
                self.logger.debug("%s mounted @ %s (%s) with mode %s. It will be remounted %s.",
                                  device, directory, currentDirectory, current['mode'], mode)
                mountargs.append('remount')
                directory = currentDirectory
            else:
                # Already mounted, but not in the requested mode.
                self.logger.debug("%s mounted @ %s with mode %s. It will be remounted %s." % (device, directory, current['mode'], mode))
                mountargs.append('remount')
        else:
            # Not mounted at all.
            self.logger.debug("%s not mounted @ %s. It will be mounted %s" % (device, directory, mode))

        try:
            p = device.find('ubi')
            if p < 0:
                cmd = "mount -o %s %s %s" % (','.join(mountargs), device, directory)
            else:
                cmd = "mount -o %s -t %s  %s %s" % (','.join(mountargs), 'ubifs', device, directory)

            self.logger.debug("+ %s" % cmd)
            subprocess.check_call(cmd, shell=True)
        except subprocess.CalledProcessError, e:
            self.logger.error("Mount failed: '%s'" % e.output)
            return False

        # If requested, wait for the mount to complete.
        while(timeout > 0):
            if self.is_mounted(device, directory):
                break
            time.sleep(1)
            timeout-=1

        current = self.is_mounted(device, directory)
        if current:
            self.logger.debug("%s is now mounted @ %s %s" % (device, directory, current['mode']))
            return True
        else:
            self.logger.error("%s failed to report in /proc/mounts." % (directory))

    def umount(self, device, directory):
        current = self.is_mounted(device, directory)
        if not current:
            self.logger.error("umount: %s is not mounted @ %s" % (device, directory))
            return False

        try:
            out = subprocess.check_output("umount %s" % directory, shell=True)
            self.logger.debug("%s @ %s has been unmounted." % (device, directory))
            self.read_proc_mounts()
            return True

        except subprocess.CalledProcessError,e:
            self.logger.error("Could not unmount %s @ %s: %s" % (device, directory, e.output))



class MountContext(object):
    def __init__(self, device, directory, mode, logger):
        self.mm = MountManager(logger)
        self.device = device
        self.directory = directory
        self.mode = mode

    def __enter__(self):
        self.status = self.mm.is_mounted(self.device, self.directory)
        self.mm.mount(self.device, self.directory, self.mode)
        return self

    def __exit__(self, eType, eValue, eTrace):
        if self.status:
            self.mm.mount(self.device, self.directory, self.status['mode'])
        else:
            self.mm.umount(self.device, self.directory)


class OnlMountManager(object):
    def __init__(self, mdata="/etc/mtab.yml", logger=None):
        self.mm = MountManager(logger)

        if os.path.exists(mdata):
            mdata = yaml.load(open(mdata, "r"));

        self.mdata = mdata
        self.logger = logger if logger else logging.getLogger(self.__class__.__name__)

        # Needed to avoid ugly warnings from fsck
        if not os.path.exists('/etc/mtab'):
            os.system("ln -s /proc/mounts /etc/mtab")

        self.missing = None

    def init(self, timeout=5):

        for(k, v) in self.mdata['mounts'].iteritems():
            #
            # Get the partition device for the given label.
            # The timeout logic is here to handle waiting for the
            # block devices to arrive at boot.
            #
            t = timeout
            while t >= 0:
                try:
                    useUbiDev = False
                    try:
                        v['device'] = subprocess.check_output("blkid -L %s" % k, shell=True).strip()
                    except subprocess.CalledProcessError:
                        useUbiDev = True
                    if useUbiDev == False: break
                    output  = subprocess.check_output("ubinfo -d 0 -N %s" % k, shell=True).splitlines()
                    volumeId = None
                    device = None
                    for line in output:
                        line = line.strip()
                        p = line.find(':')
                        if p < 0:
                            self.logger.debug("Invaild ubinfo output %s" % line)

                        name, value = line[:p], line[p+1:].strip()
                        if 'Volume ID' in name:
                            p = value.find('(')
                            if p < 0:
                                self.logger.debug("Invalid Volume ID %s" % value)

                            volumeId = value[:p].strip()
                            
                            p = value.find('on')
                            if p < 0:
                                self.logger.debug("Invalid ubi devicde %s" % value)

                            device = value[p+2:-1].strip()

                        if 'Name' in name:
                            v['device'] = "/dev/" + device + "_" + volumeId

                        
                    break
                except subprocess.CalledProcessError:
                        self.logger.debug("Block label %s does not yet exist..." % k)
                        time.sleep(1)
                        t -= 1

            if 'device' not in v:
                self.logger.error("Timeout waiting for block label %s after %d seconds." % (k, timeout))
                self.missing = k
                return False

            #
            # Make the mount point for future use.
            #
            if not os.path.isdir(v['dir']):
                self.logger.debug("Make directory '%s'..." % v['dir'])
                os.makedirs(v['dir'])

            self.logger.debug("%s @ %s" % (k, v['device']))

    def __fsck(self, label, device):
        self.logger.info("Running fsck on %s [ %s ]..." % (label, device))
        cmd = "fsck.ext4 -p %s" % (device)
        self.logger.debug(cmd)
        try:
            out = subprocess.check_output(cmd, shell=True)
            self.logger.info("%s [ %s ] is clean." % (device, label))
            return True
        except subprocess.CalledProcessError, e:
            self.logger.error("fsck failed: %s" % e.output)
            return False

    def __label_entry(self, label, emsg=True):

        if label in self.mdata['mounts']:
            return self.mdata['mounts'][label]

        if emsg:
            self.logger.error("Label %s does not exist." % label)

        return None

    def validate_labels(self, labels):

        if type(labels) is str:
            labels = labels.split(',')
        elif type(labels) is dict:
            labels = [ labels.keys() ]
        elif type(labels) is list:
            pass
        else:
            raise ValueError("invalid labels argument.")

        if 'all' in labels:
            labels = filter(lambda l: l != 'all', labels) + self.mdata['mounts'].keys()

        rv = []
        for l in list(set(labels)):
            if self.__label_entry("ONL-%s" % l.upper(), False):
                rv.append("ONL-%s" % l.upper())
            elif self.__label_entry(l.upper(), False):
                rv.append(l.upper())
            elif self.__label_entry(l):
                rv.append(l)
            else:
                pass

        return rv;

    def fsck(self, labels, force=False):
        labels = self.validate_labels(labels)
        for label in labels:
            m = self.__label_entry(label)
            if force or m.get('fsck', False):
                if not self.mm.is_dev_mounted(m['device']):
                    self.__fsck(label, m['device'])
                else:
                    self.logger.error("%s (%s) is mounted." % (label, m['device']))


    def mount(self, labels, mode=None):
        labels = self.validate_labels(labels)
        for label in labels:
            m = self.__label_entry(label)
            mmode = mode
            if mmode is None:
                mmode = m.get('mount', False)
            if mmode:
                self.mm.mount(m['device'], m['dir'], mmode)



    def umount(self, labels, all_=False):
        labels = self.validate_labels(labels)
        for label in labels:
            m = self.__label_entry(label)
            if self.mm.is_mounted(m['device'], m['dir']):
                if all_ or m.get('mount', False) is False:
                    self.mm.umount(m['device'], m['dir'])



    ############################################################
    #
    # CLI Support
    #
    ############################################################
    @staticmethod
    def cmdMount(args, register=False):
        if register:
            p = args.add_parser('mount')
            p.add_argument("labels", nargs='+', metavar='LABEL')
            p.add_argument("--rw", help='Ignore the mtab setting and mount all labels read/write.', action='store_true')
            p.add_argument("--ro", help='Ignore the mtab setting and mount all labels read-only.', action='store_true')
            p.set_defaults(func=OnlMountManager.cmdMount)
        else:
            if args.rw:
                mode = 'rw'
            elif args.ro:
                mode = 'ro'
            else:
                mode = None

            o = OnlMountManager(args.mtab, args.logger)
            o.init()
            o.mount(args.labels, mode=mode)

    @staticmethod
    def cmdRw(args, register=False):
        if register:
            p = args.add_parser('rw')
            p.add_argument("label")
            p.add_argument("cmd", nargs='+')
            p.set_defaults(func=OnlMountManager.cmdRw)
        else:
            with OnlMountContextReadWrite(args.label, logger=None):
                rc = subprocess.call(" ".join(args.cmd), shell=True)
            sys.exit(rc)

    @staticmethod
    def cmdFsck(args, register=False):
        if register:
            p = args.add_parser('fsck')
            p.add_argument("labels", nargs='+', metavar='LABEL')
            p.add_argument("--force", help='Ignore the mtab setting and run fsck on given labels.', action='store_true')
            p.set_defaults(func=OnlMountManager.cmdFsck)
        else:
            o = OnlMountManager(args.mtab, args.logger)
            o.init()
            o.fsck(args.labels, args.force)

    @staticmethod
    def cmdUnmount(args, register=False):
        if register:
            p = args.add_parser('unmount')
            p.add_argument("labels", nargs='+', metavar='LABEL')
            p.add_argument("--all", help="Ignore the mtab setting and unmount all labels.", action='store_true')
            p.set_defaults(func=OnlMountManager.cmdUnmount)
        else:
            o = OnlMountManager(args.mtab, args.logger)
            o.init()
            o.umount(args.labels, args.all)

    @staticmethod
    def initCommands(parser):
        sp = parser.add_subparsers()
        for attr in dir(OnlMountManager):
            if attr.startswith('cmd'):
                getattr(OnlMountManager, attr)(sp, register=True)

    @staticmethod
    def main(name):
        import argparse

        logging.basicConfig()
        logger = logging.getLogger(name)

        ap = argparse.ArgumentParser(description="ONL Mount Manager.")
        ap.add_argument("--mtab", default="/etc/mtab.yml")
        ap.add_argument("--verbose", "-m", action='store_true')
        ap.add_argument("--quiet", "-q", action='store_true')

        OnlMountManager.initCommands(ap)
        ap.set_defaults(logger=logger)

        ops = ap.parse_args();

        if ops.verbose:
            logger.setLevel(logging.DEBUG)
        elif ops.quiet:
            logger.setLevel(logging.ERROR)
        else:
            logger.setLevel(logging.INFO)

        ops.func(ops)



class OnlMountContext(MountContext):
    def __init__(self, label, mode, logger):
        mm = OnlMountManager()
        mm.init()
        labels = mm.validate_labels(label)
        if not labels:
            raise ValueError("Label '%s' doesn't exist." % label)
        MountContext.__init__(self,
                              mm.mdata['mounts'][labels[0]]['device'],
                              mm.mdata['mounts'][labels[0]]['dir'],
                              mode,
                              logger)

class OnlMountContextReadOnly(OnlMountContext):
    def __init__(self, label, logger):
        OnlMountContext.__init__(self, label, "ro", logger)


class OnlMountContextReadWrite(OnlMountContext):
    def __init__(self, label, logger):
        OnlMountContext.__init__(self, label, "rw", logger)


class OnlOnieBootContext(MountContext):
    def __init__(self, mdir="/mnt/onie-boot", mode="rw", label="ONIE-BOOT", logger=None):
        try:
            device = subprocess.check_output("blkid -L %s" % label, shell=True).strip()
        except subprocess.CalledProcessError:
            self.logger.debug("Block label %s does not yet exist..." % label)
            raise
        if not os.path.exists(mdir):
            os.makedirs(mdir)
        MountContext.__init__(self, device, mdir, mode, logger)
