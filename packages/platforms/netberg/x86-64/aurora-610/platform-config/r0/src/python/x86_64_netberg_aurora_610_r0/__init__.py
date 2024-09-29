from onl.platform.base import *
from onl.platform.netberg import *
import os
import sys
import subprocess


class OnlPlatform_x86_64_netberg_aurora_610_r0(OnlPlatformNetberg,
                                               OnlPlatformPortConfig_48x25_8x100):
    PLATFORM = 'x86-64-netberg-aurora-610-r0'
    MODEL = "AURORA610"
    SYS_OBJECT_ID = ".610.1"

    def baseconfig(self):
        os.system(
            "insmod /lib/modules/`uname -r`/kernel/drivers/gpio/gpio-ich.ko gpiobase=0")
        os.system("insmod /lib/modules/`uname -r`/kernel/drivers/mfd/lpc_ich.ko")
        self.insmod('i2c-gpio')
        self.insmod('net_platform')
        self.insmod('net_psoc')
        os.system("echo net_cpld 0x77 > /sys/bus/i2c/devices/i2c-0/new_device")
        self.insmod('net_cpld')
        self.insmod('swps')
        self.insmod('vpd')
        os.system(
            "/lib/platform-config/x86-64-netberg-aurora-610-r0/onl/healthstatus.sh &")
        self.insmod('optoe')

        # init SFP28 EEPROM
        for port in range(1, 49):
            self.new_i2c_device('optoe2', 0x50, port + 9)
            subprocess.call(
                'echo port%d > /sys/bus/i2c/devices/%d-0050/port_name' % (port, port + 9), shell=True)

        # init QSFP28 EEPROM
        for port in range(49, 57):
            self.new_i2c_device('optoe1', 0x50, port + 9)

        subprocess.call(
            'echo port50 > /sys/bus/i2c/devices/58-0050/port_name', shell=True)
        subprocess.call(
            'echo port49 > /sys/bus/i2c/devices/59-0050/port_name', shell=True)
        subprocess.call(
            'echo port52 > /sys/bus/i2c/devices/60-0050/port_name', shell=True)
        subprocess.call(
            'echo port51 > /sys/bus/i2c/devices/61-0050/port_name', shell=True)
        subprocess.call(
            'echo port54 > /sys/bus/i2c/devices/62-0050/port_name', shell=True)
        subprocess.call(
            'echo port53 > /sys/bus/i2c/devices/63-0050/port_name', shell=True)
        subprocess.call(
            'echo port56 > /sys/bus/i2c/devices/64-0050/port_name', shell=True)
        subprocess.call(
            'echo port55 > /sys/bus/i2c/devices/65-0050/port_name', shell=True)

        return True
