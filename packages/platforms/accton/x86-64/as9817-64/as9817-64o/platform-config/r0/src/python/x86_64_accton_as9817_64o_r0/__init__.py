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


class OnlPlatform_x86_64_accton_as9817_64o_r0(OnlPlatformAccton,
                                              OnlPlatformPortConfig_64x800_2x25):
    PLATFORM='x86-64-accton-as9817-64o-r0'
    MODEL="AS9817-64O"
    SYS_OBJECT_ID=".9817.64"

    def modprobe(self, module, required=True, params={}):
        cmd = "modprobe %s" % module
        subprocess.check_call(cmd, shell=True)

    def baseconfig(self):
        if init_ipmi() is not True:
            return False

        self.modprobe('optoe')
        self.modprobe('at24')

        for m in [ 'i2c-ocores', 'fpga', 'mux', 'fan', 'psu', 'thermal', 'sys', 'leds' ]:
            self.insmod("x86-64-accton-as9817-64-%s" % m)

        ########### initialize I2C bus 0 ###########
        self.new_i2c_devices(
            [
                # initialize multiplexer (PCA9548)
                ('as9817_64_mux', 0x78, 0),

                # initiate IDPROM
                ('24c02', 0x56, 67),
                ]
            )

        # initialize SFP devices
        for port in range(1, 65):
            subprocess.call('echo 0 > /sys/devices/platform/as9817_64_fpga/module_reset_%d' % (port), shell=True)

        sfp_bus = [
             1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16,
            17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32,
            33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48,
            49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64,
            65, 66
        ]

        for port in range(1, len(sfp_bus)+1):
            self.new_i2c_device('optoe3' if (port <= 64) else 'optoe2', 0x50, sfp_bus[port-1])
            subprocess.call('echo port%d > /sys/bus/i2c/devices/%d-0050/port_name' % (port, sfp_bus[port-1]), shell=True)

        return True
