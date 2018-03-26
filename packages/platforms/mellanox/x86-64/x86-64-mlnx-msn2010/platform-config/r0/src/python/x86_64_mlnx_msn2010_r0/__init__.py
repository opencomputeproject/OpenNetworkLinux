from onl.platform.base import *
from onl.platform.mellanox import *

class OnlPlatform_x86_64_mlnx_msn2010_r0(OnlPlatformMellanox,
                                               OnlPlatformPortConfig_32x100):
    PLATFORM='x86-64-mlnx-msn2010-r0'
    MODEL="SN2010"
    SYS_OBJECT_ID=".2010.1"

    def baseconfig(self):
        # load modules
        import os
        # necessary if there are issues with the install
        # os.system("/usr/bin/apt-get install")
        os.system("/etc/mlnx/mlnx-hw-management start")
        self.syseeprom_export();
        return True
