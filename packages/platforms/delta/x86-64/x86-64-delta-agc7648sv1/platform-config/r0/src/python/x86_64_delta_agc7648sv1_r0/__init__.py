from onl.platform.base import *
from onl.platform.delta import *

class OnlPlatform_x86_64_delta_agc7648sv1_r0(OnlPlatformDelta,
                                              OnlPlatformPortConfig_32x100):
    PLATFORM='x86-64-delta-agc7648sv1-r0'
    MODEL="AGC7648SV1"
    SYS_OBJECT_ID=".7648.1"


    def baseconfig(self):
        #Remove and rescan bus
        os.system("echo 1 >/sys/bus/i2c/devices/i2c-0/firmware_node/physical_node/remove")
        os.system("echo 1 > /sys/bus/pci/rescan")

        #insert tmp401(tmp431/tmp432) module
        self.insmod('tmp401')

        #insert platform module
        self.insmod('delta_agc7648sv1_platform')

        #Insert psu module
        self.insmod('dni_agc7648sv1_psu')

        #insert fan module
        self.insmod('dni_emc2305')

        #insert qsfp mosule
        self.insmod('optoe')

        return True

