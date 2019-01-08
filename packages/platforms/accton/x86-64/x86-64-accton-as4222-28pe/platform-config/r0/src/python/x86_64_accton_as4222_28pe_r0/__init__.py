from onl.platform.base import *
from onl.platform.accton import *

class OnlPlatform_x86_64_accton_as4222_28pe_r0(OnlPlatformAccton,
                                              OnlPlatformPortConfig_48x25_6x100):

    PLATFORM='x86-64-accton-as4222-28pe-r0'
    MODEL="AS4222-28PE"
    SYS_OBJECT_ID=".4222.28"

    def baseconfig(self):
        self.insmod('optoe')        
        for m in [ 'cpld', 'leds' ]:
            self.insmod("x86-64-accton-as4222-28pe-%s.ko" % m)

        ########### initialize I2C bus 0 ###########
        # initialize multiplexer (PCA9548)        
        self.new_i2c_device('pca9548', 0x72, 0)
       
        self.new_i2c_devices([
            # initialize CPLD
            #initiate CPLD
            ('as4222_28pe_cpld', 0x60, 1)
            ])
        self.new_i2c_devices([
            # inititate LM75           
            ('lm75', 0x48, 9),
            ('lm75', 0x49, 9),
            ('lm75', 0x4a, 9),            
            ('lm75', 0x4b, 9),
         ])

        # initialize QSFP port 1~34
        for port in range(1, 5):
            self.new_i2c_device('optoe1', 0x50, port+4)
            subprocess.call('echo port%d > /sys/bus/i2c/devices/%d-0050/port_name' % (port+24, port+4), shell=True)
      
        self.new_i2c_device('24c02', 0x57, 3)
        return True
