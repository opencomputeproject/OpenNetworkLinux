from onl.platform.base import *
from onl.platform.delta import *
import os.path
import subprocess

class OnlPlatform_x86_64_delta_agc7646slv1b_r0(OnlPlatformDelta, OnlPlatformPortConfig_46x10_6x100):
    PLATFORM='x86-64-delta-agc7646slv1b-r0'
    MODEL="AGC7646SLV1B"
    SYS_OBJECT_ID=".7648.1"

    def baseconfig(self):
        #Check BMC monitor status
        bmc_mon_status = subprocess.check_output('ipmitool raw 0x38 0x1a 0x00', shell=True)
        if bmc_mon_status == ' 00\n':
            os.system("ipmitool raw 0x38 0x0a 1")

        #Remove and rescan bus
        os.system("i2cset -y 0 0x31 0x14 0xfd")
        os.system("echo 1 > /sys/bus/i2c/devices/i2c-0/firmware_node/physical_node/remove")
        os.system("echo 1 > /sys/bus/pci/rescan")

        #Insert tmp401(tmp431/tmp432) module
        os.system('modprobe tmp401')

        #Insert platform module
        self.insmod('delta_agc7646slv1b_platform')

        #Insert psu module
        self.insmod('dni_agc7646slv1b_psu')

        #Insert fan module
        self.insmod('dni_emc2305')

        #Insert qsfp mosule
        self.insmod('optoe')

        #Restore BMC Monitor status
        if bmc_mon_status == ' 00\n':
            os.system("ipmitool raw 0x38 0x0a 0")
        elif bmc_mon_status == ' 01\n':
            os.system("ipmitool raw 0x38 0x0a 1")

        #Prevent onlpd and onlp-snmpd access i2c peripherals
        os.system("i2cset -y -f 0 0x31 0x14 0xfc")

        return True

