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

class OnlPlatform_x86_64_accton_as9926_24db_r0(OnlPlatformAccton,
                                               OnlPlatformPortConfig_24x400_2x10):
    PLATFORM='x86-64-accton-as9926-24db-r0'
    MODEL="AS9926-24DB"
    SYS_OBJECT_ID=".9926.24"

    def baseconfig(self):
        if init_ipmi() is not True:
            return False

        self.insmod('optoe')

        for m in [ 'fan', 'psu', 'leds', 'sfp', 'sys', 'thermal' ]:
            self.insmod("x86-64-accton-as9926-24db-%s.ko" % m)

        ########### initialize I2C bus 0 ###########
        self.new_i2c_devices(
            [
                # initialize multiplexer (PCA9548)
                ('pca9548', 0x77, 0),
                ('pca9548', 0x72, 2),
                ('pca9548', 0x73, 2),
                ('pca9548', 0x74, 2),
                ('pca9548', 0x76, 2),
                ]
            )

        for port in range(1, 25):
            subprocess.call('echo 0 > /sys/devices/platform/as9926_24db_sfp/module_reset_%d' % port, shell=True)

        # initialize QSFP devices
        for port in range(1, 25):
            self.new_i2c_device('optoe1', 0x50, port+8)

        # initialize SFP devices
        for port in range(25, 27):
            self.new_i2c_device('optoe2', 0x50, port+8)

        # set port name
        for port in range(1, 27):
            subprocess.call('echo port%d > /sys/bus/i2c/devices/%d-0050/port_name' % (port, port+8), shell=True)


        return True
