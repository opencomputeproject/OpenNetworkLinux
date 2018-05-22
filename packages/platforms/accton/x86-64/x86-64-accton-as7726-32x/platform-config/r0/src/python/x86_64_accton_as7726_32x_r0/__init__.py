from onl.platform.base import *
from onl.platform.accton import *

class OnlPlatform_x86_64_accton_as7726_32x_r0(OnlPlatformAccton,
                                              OnlPlatformPortConfig_48x25_6x100):

    PLATFORM='x86-64-accton-as7726-32x-r0'
    MODEL="AS7726-32X"
    SYS_OBJECT_ID=".7726.32"

    def baseconfig(self):
        self.insmod('optoe')
        self.insmod('ym2651y')
        for m in [ 'cpld', 'fan', 'psu', 'leds' ]:
            self.insmod("x86-64-accton-as7726-32x-%s.ko" % m)

        ########### initialize I2C bus 0 ###########
        # initialize multiplexer (PCA9548)        
        self.new_i2c_device('pca9548', 0x77, 0)
        # initiate multiplexer (PCA9548)
        self.new_i2c_devices(
            [
                # initiate multiplexer (PCA9548)
                ('pca9548', 0x76, 1),
                ('pca9548', 0x72, 1),
                ('pca9548', 0x73, 1),
                ('pca9548', 0x74, 1),
                ('pca9548', 0x75, 1),
                ('pca9548', 0x71, 2)
                ]
            )
        
        self.new_i2c_devices([
            # initialize CPLD
            ('as7726_32x_cpld1', 0x60, 11),
            ('as7726_32x_cpld2', 0x62, 12),
            ('as7726_32x_cpld3', 0x64, 13),
            ])
        self.new_i2c_devices([
            # initiate fan
            ('as7726_32x_fan', 0x66, 54),
            ('lm75', 0x4c, 54),

            # inititate LM75
            ('lm75', 0x48, 55),
            ('lm75', 0x49, 55),
            ('lm75', 0x4a, 55),
            ('lm75', 0x4b, 55),
         ])

        self.new_i2c_devices([
            # initiate PSU-1
            ('as7726_32x_psu1', 0x53, 50),
            ('ym2651', 0x5B, 50),

            # initiate PSU-2
            ('as7726_32x_psu2', 0x50, 49),
            ('ym2651', 0x58, 49),
         ])

        # initialize QSFP port 1~8
        for port in range(1, 5):
            self.new_i2c_device('optoe1', 0x50, port+20)
            subprocess.call('echo port%d > /sys/bus/i2c/devices/%d-0050/port_name' % (port, port+20), shell=True)
            
        self.new_i2c_device('optoe1', 0x50, 26) 
        subprocess.call('echo port%d > /sys/bus/i2c/devices/%d-0050/port_name' % (5, 26), shell=True)
        self.new_i2c_device('optoe1', 0x50, 25) 
        subprocess.call('echo port%d > /sys/bus/i2c/devices/%d-0050/port_name' % (6, 25), shell=True)
        self.new_i2c_device('optoe1', 0x50, 28) 
        subprocess.call('echo port%d > /sys/bus/i2c/devices/%d-0050/port_name' % (7, 28), shell=True)
        self.new_i2c_device('optoe1', 0x50, 27) 
        subprocess.call('echo port%d > /sys/bus/i2c/devices/%d-0050/port_name' % (8, 27), shell=True)
        
        # initialize QSFP port 9~16    
        for port in range(9, 13):
            self.new_i2c_device('optoe1', 0x50, port+8)
            subprocess.call('echo port%d > /sys/bus/i2c/devices/%d-0050/port_name' % (port, port+8), shell=True)
        for port in range(13, 17):
            self.new_i2c_device('optoe1', 0x50, port+16)
            subprocess.call('echo port%d > /sys/bus/i2c/devices/%d-0050/port_name' % (port, port+16), shell=True)

        # initialize QSFP port 17~24
        for port in range(17, 21):
            self.new_i2c_device('optoe1', 0x50, port+16)
            subprocess.call('echo port%d > /sys/bus/i2c/devices/%d-0050/port_name' % (port, port+16), shell=True)
        for port in range(21, 25):
            self.new_i2c_device('optoe1', 0x50, port+24)
            subprocess.call('echo port%d > /sys/bus/i2c/devices/%d-0050/port_name' % (port, port+24), shell=True)

        # initialize QSFP port 25~32
        for port in range(25, 33):
            self.new_i2c_device('optoe1', 0x50, port+12)
            subprocess.call('echo port%d > /sys/bus/i2c/devices/%d-0050/port_name' % (port, port+12), shell=True)
        
        # initialize SFP port 33~34
        for port in range(33, 35):
            self.new_i2c_device('optoe1', 0x50, port-18)
            subprocess.call('echo port%d > /sys/bus/i2c/devices/%d-0050/port_name' % (port, port-18), shell=True)

        
       
        self.new_i2c_device('24c02', 0x56, 0)
        return True
