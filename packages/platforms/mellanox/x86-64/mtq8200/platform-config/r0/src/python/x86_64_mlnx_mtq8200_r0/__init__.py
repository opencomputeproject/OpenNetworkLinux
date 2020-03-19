from onl.platform.base import *
from onl.platform.mellanox import *

class OnlPlatform_x86_64_mlnx_mtq8200_r0(OnlPlatformMellanox,
                                           OnlPlatformPortConfig_32x100):
    PLATFORM='x86-64-mlnx-mtq8200-r0'
    MODEL="MTQ8200"
    SYS_OBJECT_ID=".8100.2"

    def baseconfig(self):
        # load modules
        import os
        # necessary if there are issues with the install
        # os.system("/usr/bin/apt-get install")
        self.syseeprom_export();
        return True
