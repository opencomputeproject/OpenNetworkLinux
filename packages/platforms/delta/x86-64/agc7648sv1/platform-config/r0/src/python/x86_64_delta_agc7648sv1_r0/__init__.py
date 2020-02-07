from onl.platform.base import *
from onl.platform.delta import *
import os.path
import subprocess

class OnlPlatform_x86_64_delta_agc7648sv1_r0(OnlPlatformDelta, OnlPlatformPortConfig_48x10_6x100):
    PLATFORM='x86-64-delta-agc7648sv1-r0'
    MODEL="AGC7648SV1"
    SYS_OBJECT_ID=".7648.1"


    def baseconfig(self):

        #Remove and rescan bus
        os.system("i2cset -y 0 0x31 0x14 0xfd")
        os.system("echo 1 > /sys/bus/i2c/devices/i2c-0/firmware_node/physical_node/remove")
        os.system("echo 1 > /sys/bus/pci/rescan")

        #Insert platform module
        self.insmod('delta_agc7648sv1_platform')

        #Insert qsfp mosule
        self.insmod('optoe')

        #Prevent onlpd and onlp-snmpd access i2c peripherals
        os.system("i2cset -y -f 0 0x31 0x14 0xfc")

        return True

