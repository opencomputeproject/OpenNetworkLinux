#!/usr/bin/python
############################################################
#
############################################################
from onl.platform.base import *
from onl.platform.accton import *

class OnlPlatform_powerpc_accton_as6700_32x_r0(OnlPlatformAccton):

    def model(self):
        return "AS6700-32X"

    def platform(self):
        return 'powerpc-accton-as6700-32x-r0'

    def baseconfig(self):
        with open("/etc/default/watchdog", 'a') as f:
            f.write("run_watchdog=0\n");
        return True

    def sys_oid_platform(self):
        return ".6700.32"

