from onl.platform.base import *
from onl.platform.mellanox import *

class OnlPlatform_x86_64_mlnx_mqm8700_r0(OnlPlatformMellanox,
                                               OnlPlatformPortConfig_32x100):
    PLATFORM='x86-64-mlnx-mqm8700-r0'
    MODEL="MQM8700"
    SYS_OBJECT_ID=".8700.1"

    def baseconfig(self):
        # load modules
        import os
        # necessary if there are issues with the install
        # os.system("/usr/bin/apt-get install")
        os.system("/etc/mlnx/mlnx-hw-management start")
        self.syseeprom_export();
        return True
