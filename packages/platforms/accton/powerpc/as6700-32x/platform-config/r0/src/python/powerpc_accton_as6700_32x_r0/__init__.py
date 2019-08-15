#!/usr/bin/python
############################################################
#
############################################################
from onl.platform.base import *
from onl.platform.accton import *

class OnlPlatform_powerpc_accton_as6700_32x_r0(OnlPlatformAccton,
                                               OnlPlatformPortConfig_32x40):
    PLATFORM='powerpc-accton-as6700-32x-r0'
    MODEL="AS6700-32X"
    SYS_OBJECT_ID=".6700.32"

    def baseconfig(self):
        with open("/etc/default/watchdog", 'a') as f:
            f.write("run_watchdog=0\n");
        return True



