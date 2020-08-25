from onl.platform.base import *
from onl.platform.mellanox import *

class OnlPlatform_x86_64_mlnx_msn4600_r0(OnlPlatformMellanox,
                                               OnlPlatformPortConfig_64x200):
    PLATFORM='x86-64-mlnx-msn4600-r0'
    MODEL="MSN4600"
    SYS_OBJECT_ID=".4600.1"

    def baseconfig(self):
        # load modules
        import os
        # necessary if there are issues with the install
        # os.system("/usr/bin/apt-get install")
        self.syseeprom_export();
        return True
