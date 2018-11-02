from onl.platform.base import *
from onl.platform.accton import *

class OnlPlatform_x86_64_accton_as9716_32d_r0(OnlPlatformAccton,
                                              OnlPlatformPortConfig_48x25_6x100):

    PLATFORM='x86-64-accton-as9716-32d-r0'
    MODEL="AS9716-32D"
    SYS_OBJECT_ID=".9716.32"

    def baseconfig(self):
        self.insmod('optoe')       
        self.insmod('cpr_4011_4mxx')
        for m in [ 'cpld', 'fan', 'psu', 'leds' ]:
            self.insmod("x86-64-accton-as9716-32d-%s.ko" % m)

        ########### initialize I2C bus 0 ###########
        # initialize multiplexer (PCA9548)        
        self.new_i2c_device('pca9548', 0x77, 0)
        # initiate multiplexer (PCA9548)
        self.new_i2c_devices(
            [
                # initiate multiplexer (PCA9548)                
                ('pca9548', 0x72, 1),
                ('pca9548', 0x76, 1),                
            ]
        )
        self.new_i2c_devices(
            [
                # initiate multiplexer (PCA9548)                
                ('pca9548', 0x72, 2),
                ('pca9548', 0x73, 2), 
                ('pca9548', 0x74, 2),
                ('pca9548', 0x75, 2),
                ('pca9548', 0x76, 2)
            ]
        )
        self.new_i2c_devices([
            # initialize CPLD
             #initiate CPLD
            ('as9716_32d_cpld1', 0x60, 19),            
            ('as9716_32d_cpld2', 0x61, 20),
            ('as9716_32d_cpld3', 0x62, 21),
            ])
        self.new_i2c_devices([
            # initiate fan
              # initiate chassis fan
            ('as9716_32d_fan', 0x66, 17),

            # inititate LM75           
            ('lm75', 0x48, 18),
            ('lm75', 0x49, 18),
            ('lm75', 0x4a, 18),
            ('lm75', 0x4b, 18),
            ('lm75', 0x4c, 18),
            ('lm75', 0x4e, 18),
            ('lm75', 0x4f, 18),
         ])

        self.new_i2c_devices([
            # initiate PSU-2
            ('as9716_32d_psu2', 0x53, 10),
            ('cpr_4011_4mxx', 0x5B, 10),           

            # initiate PSU-1
            ('as9716_32d_psu1', 0x50, 9),
            ('cpr_4011_4mxx', 0x58, 9),            
         ])

        # initialize QSFP port 1~34
        for port in range(1, 35):
            self.new_i2c_device('optoe1', 0x50, port+24)
            subprocess.call('echo port%d > /sys/bus/i2c/devices/%d-0050/port_name' % (port, port+20), shell=True)
       
      
        self.new_i2c_device('24c02', 0x56, 0)
        return True
