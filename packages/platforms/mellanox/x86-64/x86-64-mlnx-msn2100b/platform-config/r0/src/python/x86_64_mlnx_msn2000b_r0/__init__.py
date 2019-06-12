from onl.platform.base import *
from onl.platform.mellanox import *

class OnlPlatform_x86_64_mlnx_msn2100b_r0(OnlPlatformMellanox,
                                           OnlPlatformPortConfig_32x40):
    PLATFORM='x86-64-mlnx-msn2100b-r0'
    MODEL="MSN2100B"
    SYS_OBJECT_ID=".2100.2"

    def baseconfig(self):
        # load modules
        import os
        # necessary if there are issues with the install
        # os.system("/usr/bin/apt-get install")
        self.syseeprom_export();
        return True
