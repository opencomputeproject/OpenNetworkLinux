from onl.platform.base import *
from onl.platform.mellanox import *

class OnlPlatform_x86_64_mlnx_msb78002_r0(OnlPlatformMellanox,
                                           OnlPlatformPortConfig_32x40):
    PLATFORM='x86-64-mlnx-msb78002-r0'
    MODEL="MSB78002"
    SYS_OBJECT_ID=".2700.4"

    def baseconfig(self):
        # load modules
        import os
        # necessary if there are issues with the install
        # os.system("/usr/bin/apt-get install")
        self.syseeprom_export();
        return True
