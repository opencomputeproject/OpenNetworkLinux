#!/usr/bin/python

"""onldump.py

Test harness for Python bindings.
"""

import sys
from ctypes import *

import logging

import onlp.onlp
import onlp.onlplib

logging.basicConfig()
logger = logging.getLogger("onlpdump")
logger.setLevel(logging.DEBUG)

onlp.onlp.onlp_init()

libonlp = onlp.onlp.libonlp
si = onlp.onlp.onlp_sys_info()
libonlp.onlp_sys_info_get(byref(si))

logger.info("hello")

import pdb
pdb.set_trace()

##libonlp.onlp_onie_show(byref(si.onie_info), byref(libonlp.aim_pvs_stdout))
libonlp.onlp_platform_dump(libonlp.aim_pvs_stdout,
                           (onlp.onlp.ONLP_OID_DUMP.RECURSE
                            | onlp.onlp.ONLP_OID_DUMP.EVEN_IF_ABSENT))

sys.exit(0)
