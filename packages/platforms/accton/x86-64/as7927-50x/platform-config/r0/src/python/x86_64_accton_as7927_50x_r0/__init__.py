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

class OnlPlatform_x86_64_accton_as7927_50x_r0(OnlPlatformAccton,
                                              OnlPlatformPortConfig_1x800_1x400_8X50_40X25):
    PLATFORM='x86-64-accton-as7927-50x-r0'
    MODEL="AS7927-50X"
    SYS_OBJECT_ID=".7927.50"

    def modprobe(self, module, required=True, params={}):
        cmd = "modprobe %s" % module
        subprocess.check_call(cmd, shell=True)

    def baseconfig(self):
        if init_ipmi() is not True:
            return False

        self.modprobe('optoe')
        self.modprobe('accton_ipmi_intf')

        for m in [ 'i2c-ocores', 'fpga', 'fan', 'psu', 'thermal', 'sys', 'leds']:
            self.insmod("x86-64-accton-as7927-50x-%s" % m)

        # initialize SFP devices
        for port in range(1, 49):
            subprocess.call('echo 1 > /sys/devices/platform/as7927_50x_fpga/module_efuse_%d' % (port), shell=True)
        for port in range(49, 51):
            subprocess.call('echo 1 > /sys/devices/platform/as7927_50x_fpga/module_enable_%d' % (port), shell=True)
            subprocess.call('echo 0 > /sys/devices/platform/as7927_50x_fpga/module_reset_%d' % (port), shell=True)
        #SFP 
        for port in range(1, 49):
            self.new_i2c_device('optoe2', 0x50, port)
        #QSFP-DD
        for port in range(49, 51):
            self.new_i2c_device('optoe3', 0x50, port)

        for port in range(1, 51):
            subprocess.call('echo port%d > /sys/bus/i2c/devices/%d-0050/port_name' % (port, port), shell=True)

        return True
