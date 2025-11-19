import commands
import time
from itertools import chain
from onl.platform.base import *
from onl.platform.accton import *

init_ipmi_dev = [
    'echo "remove,kcs,i/o,0xca2" > /sys/module/ipmi_si/parameters/hotmod',
    'echo "add,kcs,i/o,0xca2" > /sys/module/ipmi_si/parameters/hotmod'
]

# ONL may boot more faster than BMC, so retries are needed.
# But if the waiting time runs out, there may be something wrong with BMC,
# then ONL gives up waiting.
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
        time.sleep(interval)

    return (False, ATTEMPTS * interval)

def init_ipmi_oem_cmd():
    attempts = ATTEMPTS
    interval = INTERVAL

    while attempts:
        # to see if BMC is scanning 
        status, output = commands.getstatusoutput('ipmitool raw 0x34 0x95')
        if status:
            attempts -= 1
            time.sleep(interval)
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
            time.sleep(interval - elapsed_dev)
            continue

        (status, elapsed_oem) = init_ipmi_oem_cmd()
        if status is not True:
            time.sleep(interval - elapsed_dev - elapsed_oem)
            continue

        print('IPMI dev interface is ready.')
        return True

    print('Failed to initialize IPMI dev interface')
    return False

class OnlPlatform_x86_64_accton_as9737_32db_r0(OnlPlatformAccton,
                                               OnlPlatformPortConfig_32x400_1x10_1x1):
    PLATFORM='x86-64-accton-as9737-32db-r0'
    MODEL="AS9737-32DB"
    SYS_OBJECT_ID=".9737.32"

    def modprobe(self, module, required=True, params={}):
        cmd = "modprobe %s" % module
        subprocess.check_call(cmd, shell=True)

    def baseconfig(self):
        if init_ipmi() is not True:
            return False

        self.modprobe('optoe')
        self.modprobe('at24')
        self.insmod('accton_ipmi_intf')

        for m in [ 'i2c-ocores', 'fpga', 'mux', 'cpld', 'fan', 'psu', 'thermal', 'sys', 'leds' ]:
            self.insmod("x86-64-accton-as9737-32db-%s.ko" % m)

        ########### initialize I2C bus 0 ###########
        self.new_i2c_devices(
            [
                # initialize multiplexer (PCA9548)
                ('as9737_32db_mux', 0x77, 0),

                # initialize CPLDs
                ('as9737_32db_cpld2', 0x61, 35),
                ('as9737_32db_cpld3', 0x62, 36),

                # EEPROM
                #('24c02', 0x56, 0),
                ]
            )

        # initialize SFP devices
        for port in range(1, 17):
            subprocess.call('echo 0 > /sys/bus/i2c/devices/35-0061/module_reset_%d' % (port), shell=True)

        for port in range(17, 33):
            subprocess.call('echo 0 > /sys/bus/i2c/devices/36-0062/module_reset_%d' % (port), shell=True)

        sfp_bus = [
             1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16,
            17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32,
            33, 34
        ]

        for port in range(1, len(sfp_bus)+1):
            self.new_i2c_device('optoe3' if (port <= 32) else 'optoe2', 0x50, sfp_bus[port-1])
            subprocess.call('echo port%d > /sys/bus/i2c/devices/%d-0050/port_name' % (port, sfp_bus[port-1]), shell=True)

        return True
