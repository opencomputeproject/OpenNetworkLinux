#!/usr/bin/python2
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
import glob
from string import Template
import time

logger = None

class colors(object):

    RED=31
    GREEN=32
    YELLOW=33
    BLUE=34
    PURPLE=35
    CYAN=36
    REDB=41
    GREENB=42
    YELLOWB=43
    BLUEB=44
    PURPLEB=45
    CYANB=46

    @staticmethod
    def color(string, color):
        if sys.stderr.isatty():
            return "\033[1;%sm%s\033[1;0m" % (color, string)
        else:
            return string

# Adds a method for each color to the colors class
for attr in dir(colors):
    def colormethod(attr):
        def f(klass, string):
            return colors.color(string, getattr(colors, attr))
        return classmethod(f)

    if attr.isupper():
        setattr(colors, attr.lower(), colormethod(attr))




def color_logging():
    logging.addLevelName( logging.WARNING, colors.red(logging.getLevelName(logging.WARNING)))
    logging.addLevelName( logging.ERROR, colors.redb(logging.getLevelName(logging.ERROR)))

def init_logging(name, lvl=logging.DEBUG):
    global logger
    logging.basicConfig()
    logger = logging.getLogger(name)
    logger.setLevel(lvl)
    color_logging()
    return logger


class Profiler(object):

    ENABLED=True
    LOGFILE=None

    def __enter__(self):
        self.start = time.time()
        return self

    def __exit__(self, *exc):
        self.end = time.time()
        self.duration = self.end - self.start

    def log(self, operation, prefix=''):
        msg = "[profiler] %s%s : %s seconds (%s minutes)" % (prefix, operation, self.duration, self.duration / 60.0)
        if self.ENABLED:
            logger.info(colors.cyan(msg))
        if self.LOGFILE:
            with open(self.LOGFILE, "a") as f:
                f.write(msg + "\n")

############################################################
#
# Log and execute system commands
#
def execute(args, sudo=False, chroot=None, ex=None, env=False):

    if isinstance(args, basestring):
        # Must be executed through the shell
        shell = True
    else:
        shell = False

    if chroot and os.geteuid() != 0:
        # Must be executed under sudo
        sudo = True

    if chroot:
        if isinstance(args, basestring):
            args = "chroot %s %s" % (chroot, args)
        elif type(args) in (list,tuple):
            args = ['chroot', chroot] + list(args)

    if sudo:
        if isinstance(args, basestring):
            if env:
                args = "sudo -E %s" % (args)
            else:
                args = "sudo %s" % (args)
        elif type(args) in (list, tuple):
            if env:
                args = [ 'sudo', '-E', ] + list(args)
            else:
                args = [ 'sudo' ] + list(args)


    logger.debug("Executing:%s", args)

    rv = 0
    with Profiler() as profiler:
        try:
            subprocess.check_call(args, shell=shell)
            rv = 0
        except subprocess.CalledProcessError, e:
            if ex:
                raise ex
            rv = e.returncode
    profiler.log(args)
    return rv


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
        logger.debug("taking lock %s" % self.filename)
        fcntl.flock(self.handle, fcntl.LOCK_EX)
        logger.debug("took lock %s" % self.filename)

    def give(self):
        fcntl.flock(self.handle, fcntl.LOCK_UN)
        logger.debug("released lock %s" % self.filename)

    def __enter__(self):
        self.take()

    def __exit__(self ,type, value, traceback):
        self.give()

    def __del__(self):
        self.handle.close()


def filepath(absdir, relpath, eklass, required=True):
    """Return the absolute path for the given absdir/repath file."""
    p = None
    if os.path.isabs(relpath):
        p = relpath
    else:
        p = os.path.join(absdir, relpath)

    # Globs that result in a single file are allowed:
    g = glob.glob(p)
    if len(g) is 0:
        if required:
            raise eklass("'%s' did not match any files." % p)
    elif len(g) > 1:
            raise eklass("'%s' matched too many files %s" % (p, g))
    else:
        p = g[0]

    return p

def validate_src_dst_file_tuples(absdir, data, dstsubs, eklass, required=True):
    files = []
    if type(data) is dict:
        for (s,d) in data.iteritems():
            files.append((s,d))
    elif type(data) is list:
        for e in data:
            if type(e) is dict:
                for (s,d) in e.iteritems():
                    files.append((s,d))
            elif type(e) in [ list, tuple ]:
                if len(e) != 2:
                    raise eklass("Too many filenames: '%s'" % (e))
                else:
                    files.append((e[0], e[1]))
            else:
                raise eklass("File entry '%s' is invalid." % (e))

    #
    # Validate/process source files.
    # Process dst paths.
    #
    flist = []
    for f in files:
        src = filepath(absdir, f[0], eklass, required)
        if os.path.exists(src):
            t = Template(f[1])
            dst = t.substitute(dstsubs)
            flist.append((src, dst))
        elif required:
            raise eklass("Source file or directory '%s' does not exist." % src)
    return flist
