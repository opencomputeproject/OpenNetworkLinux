from onl.platform.base import *
from onl.platform.delta import *

class OnlPlatform_x86_64_delta_ag9032v2a_r0(OnlPlatformDelta,
                                              OnlPlatformPortConfig_32x100):
    PLATFORM='x86-64-delta-ag9032v2a-r0'
    MODEL="AG9032V2A"
    SYS_OBJECT_ID=".9032.2.1"


    def baseconfig(self):
        #Check cpld default data
        os.system("i2cset -y 0 0x31 0x14 0xfd")
        os.system("echo 1 > /sys/bus/i2c/devices/i2c-0/firmware_node/physical_node/remove")
        os.system("echo 1 > /sys/bus/pci/rescan")

        #insert sfp module
        self.insmod('delta_ag9032v2a_platform')

        #Insert psu module
        self.insmod('dni_ag9032v2a_psu')

        #insert fan module
        self.insmod('dni_emc2305')

        #insert qsfp mosule
        self.insmod('optoe')

        return True

