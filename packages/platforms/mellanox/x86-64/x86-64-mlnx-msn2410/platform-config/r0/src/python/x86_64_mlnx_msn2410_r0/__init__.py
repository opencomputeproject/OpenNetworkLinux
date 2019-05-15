from onl.platform.base import *
from onl.platform.mellanox import *
import os

class OnlPlatform_x86_64_mlnx_msn2410_r0(OnlPlatformMellanox,
                                               OnlPlatformPortConfig_32x100):
    PLATFORM='x86-64-mlnx-msn2410-r0'
    MODEL="SN2410"
    SYS_OBJECT_ID=".2410.1"

    def hw_management_start(self):
        for tool in [ '/etc/mlnx/mlnx-hw-management', '/usr/bin/hw-management.sh' ]:
            if os.path.exists(tool):
                print "Starting Mellanox HW Management..."
                os.system("%s start" % tool)
                return True
        return False

    def baseconfig(self):

        if not self.hw_management_start():
            print "Mellanox HW Management Package Missing."

        self.syseeprom_export();
        return True
