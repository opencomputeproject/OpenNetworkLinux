from onl.platform.base import *
from onl.platform.ingrasys import *
import os

class OnlPlatform_x86_64_ingrasys_s9230_64x_r0(OnlPlatformIngrasys):
    PLATFORM='x86-64-ingrasys-s9230-64x-r0'
    MODEL="S9230-64X"
    SYS_OBJECT_ID=".9230.64"
    
    def baseconfig(self):
                
        self.insmod("eeprom_mb")
        self.insmod("ingrasys_s9230_64x_psu")
        self.insmod("ingrasys_s9230_64x_i2c_cpld")
        
        os.system("/lib/platform-config/current/onl/sbin/i2c_utils.sh i2c_init")

        return True
        
        
