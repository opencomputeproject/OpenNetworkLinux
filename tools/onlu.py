#!/usr/bin/python
############################################################
#
# Common utilities for the ONL python tools.
#
############################################################
import logging
import subprocess
from collections import Iterable
import sys
import os
import fcntl

logger = None

# Cheap colored terminal logging. Fixme.
def color_logging():
    if sys.stderr.isatty():
        logging.addLevelName( logging.WARNING, "\033[1;31m%s\033[1;0m" % logging.getLevelName(logging.WARNING))
        logging.addLevelName( logging.ERROR, "\033[1;41m%s\033[1;0m" % logging.getLevelName(logging.ERROR))


def init_logging(name, lvl=logging.DEBUG):
    global logger
    logging.basicConfig()
    logger = logging.getLogger(name)
    logger.setLevel(lvl)
    color_logging()
    return logger


############################################################
#
# Log and execute system commands
#
def execute(args, sudo=False, chroot=None, ex=None):

    if type(args) is str:
        # Must be executed through the shell
        shell=True
    else:
        shell = False

    if chroot and os.geteuid() != 0:
        # Must be executed under sudo
        sudo = True

    if chroot:
        if type(args) is str:
            args = "chroot %s %s" % (chroot, args)
        elif type(args) in (list,tuple):
            args = ['chroot', chroot] + list(args)

    if sudo:
        if type(args) is str:
            args = "sudo %s" % (args)
        elif type(args) in (list, tuple):
            args = [ 'sudo' ] + list(args)


    logger.debug("Executing:%s", args)

    try:
        subprocess.check_call(args, shell=shell)
        return 0
    except subprocess.CalledProcessError, e:
        if ex:
            raise ex
        return e.returncode


# Flatten lists if string lists
def sflatten(coll):
    for i in coll:
            if isinstance(i, Iterable) and not isinstance(i, basestring):
                for subc in sflatten(i):
                    if subc:
                        yield subc
            else:
                yield i



def gen_salt():
    # return an eight character salt value
    salt_map = './' + string.digits + string.ascii_uppercase + \
        string.ascii_lowercase
    rand = random.SystemRandom()
    salt = ''
    for i in range(0, 8):
        salt += salt_map[rand.randint(0, 63)]
    return salt



############################################################
#
# Delete a user
#
def userdel(username):
    # Can't use the userdel command because of potential uid 0 in-user problems while running ourselves
    for line in fileinput.input('/etc/passwd', inplace=True):
        if not line.startswith('%s:' % username):
            print line,
    for line in fileinput.input('/etc/shadow', inplace=True):
        if not line.startswith('%s:' % username):
            print line,

############################################################
#
# Add a user
#
def useradd(username, uid, password, shell, deleteFirst=True):
    args = [ 'useradd', '--non-unique', '--shell', shell, '--home-dir', '/root',
             '--uid', '0', '--gid', '0', '--group', 'root' ]

    if deleteFirst:
        userdel(username)

    if password:
        epassword=crypt.crypt(password, '$1$%s$' % gen_salt());
        args = args + ['-p', epassword]

    args.append(username)

    cc(args);

    if password is None:
        cc(('passwd', '-d', username))

    logger.info("user %s password %s", username, password)


class Lock(object):
    """File Locking class."""

    def __init__(self, filename):
        self.filename = filename
        self.handle = open(filename, 'w')

    def take(self):
        # logger.debug("taking lock %s" % self.filename)
        fcntl.flock(self.handle, fcntl.LOCK_EX)
        # logger.debug("took lock %s" % self.filename)

    def give(self):
        fcntl.flock(self.handle, fcntl.LOCK_UN)
        # logger.debug("released lock %s" % self.filename)

    def __enter__(self):
        self.take()

    def __exit__(self ,type, value, traceback):
        self.give()

    def __del__(self):
        self.handle.close()

