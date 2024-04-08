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

class OnlPlatform_x86_64_accton_as9947_36xkb_r0(OnlPlatformAccton,
                                              OnlPlatformPortConfig_24x400_2x10):
    PLATFORM='x86-64-accton-as9947-36xkb-r0'
    MODEL="AS9947-36XKB"
    SYS_OBJECT_ID=".9947.36"

    def modprobe(self, module, required=True, params={}):
        cmd = "modprobe %s" % module
        subprocess.check_call(cmd, shell=True)

    def baseconfig(self):
        if init_ipmi() is not True:
            return False

        self.modprobe('optoe')
        self.modprobe('at24')

        for m in [ 'i2c-ocores', 'fpga', 'fan', 'psu', 'thermal', 'sys', 'leds','sfp']:
            self.insmod("x86-64-accton-as9947-36xkb-%s" % m)

        # initialize SFP devices
        for port in range(5, 41):
            subprocess.call('echo 0 > /sys/devices/platform/as9947_36xkb_sfp/module_reset_%d' % (port), shell=True)


        for port in range(1, 5):
            self.new_i2c_device('optoe2', 0x50, port)

        for port in range(5, 29):
            self.new_i2c_device('optoe1', 0x50, port)

        for port in range(29, 41):
            self.new_i2c_device('optoe3', 0x50, port)


        for port in range(1, 41):
            subprocess.call('echo port%d > /sys/bus/i2c/devices/%d-0050/port_name' % (port, port), shell=True)

        return True
