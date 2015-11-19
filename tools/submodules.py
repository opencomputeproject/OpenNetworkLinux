#!/usr/bin/python
############################################################
# <bsn.cl fy=2013 v=none>
#
#        Copyright 2013, 2014 BigSwitch Networks, Inc.
#
#
#
# </bsn.cl>
############################################################
#
# Submodule management.
#
############################################################
import os
import sys
import subprocess
import shutil
import logging

logging.basicConfig()
logger = logging.getLogger("submodules")
logger.setLevel(logging.INFO)


def check_call(cmd, *args, **kwargs):
    if type(cmd) == str:
        logger.debug("+ " + cmd)
    else:
        logger.debug("+ " + " ".join(cmd))
    return subprocess.check_call(cmd, *args, **kwargs)

def check_output(cmd, *args, **kwargs):
    if type(cmd) == str:
        logger.debug("+ " + cmd)
    else:
        logger.debug("+ " + " ".join(cmd))
    return subprocess.check_output(cmd, *args, **kwargs)


class OnlSubmoduleError(Exception):
    """General Package Error Exception"""

    def __init__(self, value):
        self.value = value

    def __str__(self):
        return repr(self.value)

class OnlSubmoduleManager(object):
    def __init__(self, root):
        if not os.path.exists(os.path.join(root, ".git")):
            raise OnlSubmoduleError("%s is not a git repository." % root)
        if not os.path.exists(os.path.join(root, ".gitmodules")):
            raise OnlSubmoduleError("The git repository %s does not contain any submodules." % root)

        self.root = root
        self.get_status()

    def get_status(self):
        self.status = {}
        try:
            for entry in check_output(['git', 'submodule', 'status'], cwd=self.root).split("\n"):
                data = entry.split()
                if len(data) >= 2:
                    self.status[data[1]] = data[0]
        except subprocess.CalledProcessError:
            raise OnlSubmoduleError("git command(s) failed")


    def validate(self, path):
        if not path in self.status:
            raise OnlSubmoduleError("Submodule %s does not exist in repository %s" % (path, self.root))

    def update(self, path, depth=None, recursive=False):
        self.validate(path)

        if depth:
            logger.debug("shallow clone depth=%d", int(depth))
            # Shallow clone first
            url = check_output(['git', 'config', '-f', '.gitmodules', '--get',
                                'submodule.' + path + '.url' ], cwd=self.root)
            url = url.rstrip('\n')
            args = [ 'git', 'clone', '--depth', depth, url, path ]
            try:
                check_call(args, cwd=self.root)
            except subprocess.CalledProcessError:
                raise OnlSubmoduleError("git error cloning module %s" % path)

        # full or partial update
        args = [ 'git', 'submodule', 'update', '--init' ]
        if recursive:
            args.append("--recursive")
        args.append(path)
        try:
            check_call(args, cwd=self.root)
        except subprocess.CalledProcessError:
            raise OnlSubmoduleError("git error updating module %s" % path)


        #
        # Run any repository-specific post-submodule-init scripts.
        #
        script = os.path.join(self.root, 'tools', 'scripts', 'submodule-updated.sh')
        if os.path.exists(script):
            try:
                check_call([script, path], cwd=self.root)
            except subprocess.CalledProcessError:
                # Target doesn't exists
                raise OnlSubmoduleError("The repository post-init script %s failed." % script)


    def require(self, path, depth=None, recursive=False):
        self.get_status()
        self.validate(path)
        if self.status[path][0] == '-':
            self.update(path, depth=depth, recursive=recursive)


if __name__ == '__main__':

    import argparse
    ap = argparse.ArgumentParser(description='Submodule Manager')

    ap.add_argument("root", help="The root of the git repository in which to operate.")
    ap.add_argument("path", help="The submodule path to initialize.")
    ap.add_argument("--depth", help="Shallow submodule clone to given depth.")
    ap.add_argument("--recursive", help="Recursive update.", action='store_true')

    ops = ap.parse_args()

    try:
        sm = OnlSubmoduleManager(ops.root)
        sm.require(ops.path)
    except OnlSubmoduleError, e:
        logger.error("%s" % e.value)

