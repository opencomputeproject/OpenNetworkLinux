from onl.platform.base import *
from onl.platform.accton import *

import commands

class OnlPlatform_x86_64_accton_as9726_32d_r0(OnlPlatformAccton,
                                              OnlPlatformPortConfig_32x400_2x10):

    PLATFORM='x86-64-accton-as9726-32d-r0'
    MODEL="AS9726-32D"
    SYS_OBJECT_ID=".9726.32"

    def baseconfig(self):
        self.insmod('optoe')
        self.insmod('accton_i2c_psu')
        for m in [ 'cpld', 'fan', 'psu', 'leds' ]:
            self.insmod("x86-64-accton-as9726-32d-%s.ko" % m)

        ########### initialize I2C bus 0 ###########
        # initialize multiplexer (PCA9548)        
        self.new_i2c_device('pca9548', 0x77, 0)
        # initiate multiplexer (PCA9548)
        self.new_i2c_devices(
            [
                # initiate multiplexer (PCA9548)                
                ('pca9548', 0x71, 1),
                ('pca9548', 0x72, 1),
                ('pca9548', 0x73, 1),
                ('pca9548', 0x74, 1),
                ('pca9548', 0x75, 1),
                ('pca9548', 0x76, 1)
            ]
        )

        self.new_i2c_devices([
            # initialize CPLD
             #initiate CPLD
            ('as9726_32d_fpga', 0x60, 1),            
            ('as9726_32d_cpld1', 0x61, 10),
            ('as9726_32d_cpld2', 0x62, 10),
            ])
        self.new_i2c_devices([
            # initiate fan
              # initiate chassis fan
            ('as9726_32d_fan', 0x66, 14),

            # inititate LM75           
            ('lm75', 0x48, 15),
            ('lm75', 0x49, 15),
            ('lm75', 0x4a, 15),
            ('lm75', 0x4b, 15),
            ('lm75', 0x4c, 15),
            ('lm75', 0x4f, 15),
         ])

        self.new_i2c_devices([
            # initiate PSU-2
            ('as9726_32d_psu2', 0x51, 9),
            ('acbel_fsh082', 0x59, 9),           

            # initiate PSU-1
            ('as9726_32d_psu1', 0x50, 9),
            ('acbel_fsh082', 0x58, 9),
         ])

        # initialize pca9548 idle_state in kernel 5.4.40 version
        subprocess.call('echo -2 | tee /sys/bus/i2c/drivers/pca954x/*-00*/idle_state > /dev/null', shell=True)

        # initialize QSFP port 1~32. SFP port 33~34
        for port in range(1, 35):
            if port <= 32 :
                self.new_i2c_device('optoe1', 0x50, port+16)
            else:
                self.new_i2c_device('optoe2', 0x50, port+16)
            
            subprocess.call('echo port%d > /sys/bus/i2c/devices/%d-0050/port_name' % (port, port+16), shell=True)

        self.new_i2c_device('24c02', 0x56, 13)
        
        return True
