from onl.platform.base import *
from onl.platform.delta import *
import os.path
import subprocess

class OnlPlatform_x86_64_delta_agc032_r0(OnlPlatformDelta,
                                              OnlPlatformPortConfig_32x400):
    PLATFORM='x86-64-delta-agc032-r0'
    MODEL="AGC032"
    SYS_OBJECT_ID=".032.0"


    def baseconfig(self):
        return True

