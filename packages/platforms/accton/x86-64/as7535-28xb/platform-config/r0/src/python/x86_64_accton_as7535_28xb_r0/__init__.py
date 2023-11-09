import commands
from itertools import chain
from onl.platform.base import *
from onl.platform.accton import *
from time import sleep

init_ipmi_dev = [
    'echo "remove,kcs,i/o,0xca2" > /sys/module/ipmi_si/parameters/hotmod',
    'echo "add,kcs,i/o,0xca2" > /sys/module/ipmi_si/parameters/hotmod']

ATTEMPTS = 5
INTERVAL = 3

def init_ipmi_dev_intf():
    attempts = ATTEMPTS
    interval = INTERVAL

    while attempts:
        if os.path.exists('/dev/ipmi0') or os.path.exists('/dev/ipmidev/0'):
            return (True, (ATTEMPTS - attempts) * interval)

        for i in range(0, len(init_ipmi_dev)):
            commands.getstatusoutput(init_ipmi_dev[i])

        attempts -= 1
        sleep(interval)

    return (False, ATTEMPTS * interval)

def init_ipmi_oem_cmd():
    attempts = ATTEMPTS
    interval = INTERVAL

    while attempts:
        status, output = commands.getstatusoutput('ipmitool raw 0x34 0x95')
        if status:
            attempts -= 1
            sleep(interval)
            continue

        return (True, (ATTEMPTS - attempts) * interval)

    return (False, ATTEMPTS * interval)

def init_ipmi():
    attempts = ATTEMPTS
    interval = 60

    while attempts:
        attempts -= 1

        (status, elapsed_dev) = init_ipmi_dev_intf()
        if status is not True:
            sleep(interval - elapsed_dev)
            continue

        (status, elapsed_oem) = init_ipmi_oem_cmd()
        if status is not True:
            sleep(interval - elapsed_dev - elapsed_oem)
            continue

        print('IPMI dev interface is ready.')
        return True

    print('Failed to initialize IPMI dev interface')
    return False

class OnlPlatform_x86_64_accton_as7535_28xb_r0(OnlPlatformAccton,
                                              OnlPlatformPortConfig_2x100_2x400_24x25):
    PLATFORM='x86-64-accton-as7535-28xb-r0'
    MODEL="AS7535-28XB"
    SYS_OBJECT_ID=".7535.28"

    def baseconfig(self):
        if init_ipmi() is not True:
            return False

        self.insmod('optoe')
        for m in [ 'sys', 'cpld', 'fan', 'psu', 'thermal', 'leds' ]:
            self.insmod("x86-64-accton-as7535-28xb-%s" % m)

        ########### initialize I2C bus 0 ###########
        self.new_i2c_devices(
            [
                # initialize multiplexer (PCA9548)
                ('pca9548', 0x77, 0), # i2c 1-8
                ('pca9548', 0x76, 1), # i2c 9-16
                ('pca9548', 0x73, 9), # i2c 17-24
                ('pca9548', 0x74, 17), # i2c 25-32
                ('pca9548', 0x74, 18), # i2c 33-40
                ('pca9548', 0x74, 19), # i2c 41-48

                # initialize CPLD
                ('as7535_28xb_cpld', 0x61, 12),
                ]
            )

        # initialize SFP devices
        for port in range(1, 5):
            subprocess.call('echo 0 > /sys/bus/i2c/devices/12-0061/module_reset_%d' % (port), shell=True)

        sfp_bus = [23, 21, 24, 22, 25, 26, 27, 28, 29, 30,
                   31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
                   41, 42, 43, 44, 45, 46, 47, 48]

        # initialize QSFP devices
        for port in range(1, len(sfp_bus)+1):
            self.new_i2c_device('optoe1' if (port<=4) else 'optoe2', 0x50, sfp_bus[port-1])
            subprocess.call('echo port%d > /sys/bus/i2c/devices/%d-0050/port_name' % (port, sfp_bus[port-1]), shell=True)

        return True
