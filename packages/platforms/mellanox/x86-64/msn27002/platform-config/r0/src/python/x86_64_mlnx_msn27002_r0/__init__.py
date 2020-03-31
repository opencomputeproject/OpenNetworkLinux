from onl.platform.base import *
from onl.platform.mellanox import *

class OnlPlatform_x86_64_mlnx_msn27002_r0(OnlPlatformMellanox,
                                           OnlPlatformPortConfig_32x100):
    PLATFORM='x86-64-mlnx-msn27002-r0'
    MODEL="MSN27002"
    SYS_OBJECT_ID=".27002.1"

    def baseconfig(self):
        # load modules
        import os
        # necessary if there are issues with the install
        # os.system("/usr/bin/apt-get install")
        self.syseeprom_export();
        return True
