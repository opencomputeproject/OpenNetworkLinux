from onl.platform.base import *
from onl.platform.accton import *

class OnlPlatform_x86_64_accton_as4625_30p_r0(OnlPlatformAccton,
                                              OnlPlatformPortConfig_24x1_6x10):

    PLATFORM='x86-64-accton-as4625-30p-r0'
    MODEL="AS4625-30P"
    SYS_OBJECT_ID=".4625.30.1"

    def baseconfig(self):
        os.system("modprobe i2c-ismt")
        os.system("modprobe at24")
        self.insmod('optoe')
        self.insmod('ym2651y')

        for m in [ 'cpld', 'fan', 'leds', 'psu' ]:
            self.insmod("x86-64-accton-as4625-30p-%s.ko" % m)

        ########### initialize I2C bus 0, bus 1 ###########
        self.new_i2c_devices([

            #initiate CPLD
            ('as4625_cpld1', 0x64, 0),

            # initialize multiplexer (PCA9548)
            ('pca9548', 0x70, 1),
            ('pca9548', 0x71, 1)
            ])

        self.new_i2c_devices([
            # inititate LM75
            ('lm75', 0x4a, 3),
            ('lm75', 0x4d, 3),
            ('lm75', 0x4f, 3)
            ])

        self.new_i2c_devices([
            # initiate PSU-1
            ('as4625_30p_psu1', 0x50, 8),
            ('ym2651', 0x58, 8),
            # initiate PSU-2
            ('as4625_30p_psu2', 0x51, 9),
            ('ym2651', 0x59, 9),
            ])

        # initialize pca9548 idle_state in kernel 5.4.40 version
        subprocess.call('echo -2 | tee /sys/bus/i2c/drivers/pca954x/*-00*/idle_state > /dev/null', shell=True)

        # initialize SFP port 25~30
        for port in range(25, 31):
            self.new_i2c_device('optoe2', 0x50, port-15)
            subprocess.call('echo port%d > /sys/bus/i2c/devices/%d-0050/port_name' % (port, port-15), shell=True)

        # initialize the LOC led to off
        subprocess.call('echo 0 > /sys/class/leds/as4625_led::loc/brightness', shell=True)

        self.new_i2c_device('24c02', 0x51, 7)
        return True
