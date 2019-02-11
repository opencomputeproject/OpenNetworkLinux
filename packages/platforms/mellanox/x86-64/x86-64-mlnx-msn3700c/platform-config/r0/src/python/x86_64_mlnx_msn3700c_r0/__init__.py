from onl.platform.base import *
from onl.platform.mellanox import *

class OnlPlatform_x86_64_mlnx_msn3700c_r0(OnlPlatformMellanox,
                                           OnlPlatformPortConfig_32x40):
    PLATFORM='x86-64-mlnx-msn3700c-r0'
    MODEL="MSN3700C"
    SYS_OBJECT_ID=".3700.2"

    def baseconfig(self):
        # load modules
        import os
        # necessary if there are issues with the install
        # os.system("/usr/bin/apt-get install")
        self.syseeprom_export();
        return True
