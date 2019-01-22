from onl.platform.base import *
from onl.platform.delta import *
import os.path

class OnlPlatform_x86_64_delta_agc7648sv1_r0(OnlPlatformDelta,
                                              OnlPlatformPortConfig_32x100):
    PLATFORM='x86-64-delta-agc7648sv1-r0'
    MODEL="AGC7648SV1"
    SYS_OBJECT_ID=".7648.1"


    def baseconfig(self):
        #Remove and rescan bus
        os.system("echo 1 > /sys/bus/i2c/devices/i2c-0/firmware_node/physical_node/remove")
        os.system("echo 1 > /sys/bus/pci/rescan")

        #Insert gpio_pca953x module
        os.system('modprobe gpio_pca953x')

        #Insert tmp401(tmp431/tmp432) module
        os.system('modprobe tmp401')

        #Insert platform module
        self.insmod('delta_agc7648sv1_platform')

        #Insert psu module
        self.insmod('dni_agc7648sv1_psu')

        #Insert fan module
        self.insmod('dni_emc2305')

        #Insert qsfp mosule
        self.insmod('optoe')

        #Fantray present(fan1:499, fan2:498, fan3:497, fan4:496)
        os.system("echo 496 > /sys/class/gpio/export")
        os.system("echo 497 > /sys/class/gpio/export")
        os.system("echo 498 > /sys/class/gpio/export")
        os.system("echo 499 > /sys/class/gpio/export")

        return True

