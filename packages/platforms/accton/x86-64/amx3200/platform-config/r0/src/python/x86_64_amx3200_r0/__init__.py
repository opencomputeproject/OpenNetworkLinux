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
sled_1_is_present = 1
sled_2_is_present = 1

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

class OnlPlatform_x86_64_amx3200_r0(OnlPlatformAccton,
                                             OnlPlatformPortConfig_24x400_2x10):
    PLATFORM='x86-64-amx3200-r0'
    MODEL="AMX3200"
    SYS_OBJECT_ID="amx3200"

    def modprobe(self, module, required=True, params={}):
        cmd = "modprobe %s" % module
        subprocess.check_call(cmd, shell=True)

    def baseconfig(self):
        if init_ipmi() is not True:
            return False

        self.modprobe('i2c-ismt')
        self.modprobe('optoe')
        self.modprobe('at24')

        for m in ['mux', 'fpga', 'fan', 'thermal', 'leds', 'psu', 'sled','sfp','sys']:
            self.insmod("x86-64-amx3200-%s" % m)

        self.new_i2c_devices(
            [
                # initiate IDPROM
                ('24c02', 0x52, 1),
                # initialize multiplexer (CPLD)
                ('amx3200_mux', 0x61, 1),
            ]
        )
        sled_1_is_present = commands.getoutput('cat /sys/bus/platform/devices/amx3200_sled/sled_1_present')
        sled_1_power_good = commands.getoutput('cat /sys/bus/platform/devices/amx3200_sled/sled_1_all_power_good')
        #if SLED1 is present
        if sled_1_is_present == '1' and sled_1_power_good == '1':
            self.new_i2c_devices(
                [
                    #SLED 1
                    # initialize multiplexer (PCA9548)
                    ('pca9548', 0x70, 4),
                    ('24c02', 0x53, 4),
                    ('tmp435', 0x4C, 6),
                    # initialize multiplexer (PCA9548)
                    ('pca9548', 0x71, 11),
                ]
            )
            # initialize QSFP devices
            for port in range(14, 22):
                self.new_i2c_device('optoe1', 0x50, port)

        sled_2_is_present = commands.getoutput('cat /sys/bus/platform/devices/amx3200_sled/sled_2_present')
        sled_2_power_good = commands.getoutput('cat /sys/bus/platform/devices/amx3200_sled/sled_2_all_power_good')
        #if SLED2 is present
        if sled_2_is_present == '1' and sled_2_power_good == '1':
            self.new_i2c_devices(
                [
                    #SLED 2
                    # initialize multiplexer (PCA9548)
                    ('pca9548', 0x70, 5),
                    ('24c02', 0x53, 5),
                    ('tmp435', 0x4C, 6+16*int(sled_1_is_present)),
                    # initialize multiplexer (PCA9548)
                    ('pca9548', 0x71, 11+16*int(sled_1_is_present)),
                    ]
                )
            # initialize QSFP devices
            for port in range(14+16*int(sled_1_is_present), 22+16*int(sled_1_is_present)):
                self.new_i2c_device('optoe1', 0x50, port)

        return True
