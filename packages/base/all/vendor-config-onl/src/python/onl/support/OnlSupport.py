#!/usr/bin/python
from shutil import copytree, ignore_patterns, copy, make_archive, rmtree
import logging
import errno
import os
import subprocess
import time

from onl.platform.current import OnlPlatform

copy_dir_list = ['/etc',
                 '/var/log',
                 '/mnt/onl']

dump_command_list = ['onl-sysconfig',
                     'lsmod',
                     'ifconfig',
                     'dmesg',
                     'uptime',
                     'df',
                     'blkid',
                     'onlpdump',
                     'onl-platform-show',
                     ['ntpdc -p', 'ntpdc.result'],
                     ['ps auxww', 'ps.result'],
                     ]


class OnlSupport(object):
    def __init__(self, base_root="/tmp/", debug=False):
        self.base_root = base_root
        self.dir_name = "onl-support-" + OnlPlatform().PLATFORM
        self.dir_root = self.base_root + self.dir_name
        self.command_root = self.dir_root + "/support/"
        self.tarfile_name = self.dir_name + ".tar.gz"
        self.tarfile_root = self.base_root + self.tarfile_name
        self.debug = debug
        self.logger = self.logging(debug)

    def logging(self, debug=False):
        logger = logging.getLogger('onl-support')

        if debug is True:
            logger.setLevel(logging.DEBUG)
        else:
            logger.setLevel(logging.INFO)
        ch = logging.StreamHandler()
        logger.addHandler(ch)
        return logger

    def copy_dir(self, src_dir):
        dest = self.dir_root + src_dir
        try:
            copytree(src_dir, dest, symlinks=True, ignore=ignore_patterns('alternatives', '*.deb', '*.swi'))
        except OSError as e:
            if e.errno == errno.ENOTDIR:
                copy(src_dir, dest)
            else:
                self.logger.debug('Directory not copied. Error: {}'.format(e))

    def dump_command(self, command, output_filename=None):
        # Create directory if not exists
        if not os.path.exists(self.command_root):
            os.makedirs(self.command_root)

        # Defined filename
        if output_filename:
            dest = self.command_root + output_filename  # Create new filename
        else:
            dest = self.command_root + command  # Filename using command name

        # Output result
        try:
            log_file = open(dest, "w+")
            command = command.split()  # str() split to `list`
            proc = subprocess.Popen(command,
                                    stdout=log_file,
                                    stderr=log_file)
            while proc.poll() is None:
                time.sleep(0.5)
                proc.poll()
            log_file.close()
        except Exception as e:
            self.logger.debug('Failed to open file: {}'.format(e))

    def archive(self):
        self.logger.info("Archiving Tarfile...")
        archive_path = make_archive(base_name=self.dir_name, format='gztar', base_dir=self.dir_root,
                                    root_dir=self.base_root)
        self.logger.info("Support tarball created: {}".format(archive_path))

    def dump(self):
        self.logger.info("Dumping Log...")
        for dir_name in copy_dir_list:
            self.copy_dir(dir_name)
            self.logger.debug('Dump directory {} '.format(dir_name))

        for command in dump_command_list:
            if isinstance(command, str):
                self.dump_command(command)
            elif isinstance(command, list):
                self.dump_command(command[0], command[1])
            self.logger.debug('Dump command {} '.format(command))

    def clean(self):
        rmtree(self.dir_root)
        self.logger.debug('Clean {} directory'.format(self.dir_root))

    def main(self):
        self.dump()
        self.archive()
        if self.debug is False:
            self.clean()
