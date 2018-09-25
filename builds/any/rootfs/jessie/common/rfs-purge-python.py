#!/usr/bin/python

"""rfs-purge-python

"""

import sys, os
import glob
import distutils.sysconfig
import shutil

def do_glob_rm(*els):
    p2 = os.path.join(*els)
    for p in glob.glob(p2):
        os.unlink(p)

def do_rmdir(*els):
    p2 = os.path.join(*els)
    if os.path.isdir(p2):
        shutil.rmtree(p2)

dynload = distutils.sysconfig.get_config_var('DESTSHARED')
lib = distutils.sysconfig.get_python_lib(False)

do_glob_rm(dynload, '_codecs_*.so')
do_glob_rm(dynload, '*audio*.so')
do_glob_rm(dynload, '_bsddb*.so')

do_glob_rm(lib, "encodings", "cp*")
do_glob_rm(lib, "encodings", "big*")
do_glob_rm(lib, "encodings", "shift*")
do_glob_rm(lib, "encodings", "mac*")
do_glob_rm(lib, "encodings", "iso*")
do_glob_rm(lib, "encodings", "euc*")

do_rmdir(lib, 'lib-tk')
do_rmdir(lib, "multiprocessing")
do_rmdir(lib, "distutils")
do_rmdir(lib, "lib2to3")
do_rmdir(lib, "test")
do_rmdir(lib, "unittest")
do_rmdir(lib, "bsddb")
do_rmdir(lib, "pydoc_data")
do_rmdir(lib, "wsgiref")
do_rmdir(lib, "sqlite3")
do_rmdir(lib, "xml")
do_rmdir(lib, "hotshot")
