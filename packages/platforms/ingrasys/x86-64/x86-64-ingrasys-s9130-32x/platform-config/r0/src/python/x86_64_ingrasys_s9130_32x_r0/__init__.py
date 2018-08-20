from onl.platform.base import *
from onl.platform.ingrasys import *
import os

class OnlPlatform_x86_64_ingrasys_s9130_32x_r0(OnlPlatformIngrasys):
    PLATFORM='x86-64-ingrasys-s9130-32x-r0'
    MODEL="S9130-32X"
    SYS_OBJECT_ID=".9130.32"
    
    def baseconfig(self):
                
        self.insmod("eeprom_mb")
        self.insmod("ingrasys_s9130_32x_psu")

        os.system("/lib/platform-config/current/onl/sbin/i2c_utils.sh i2c_init")

        return True
        
        
