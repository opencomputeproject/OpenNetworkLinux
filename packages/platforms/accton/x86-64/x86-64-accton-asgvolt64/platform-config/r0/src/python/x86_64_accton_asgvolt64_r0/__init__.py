from onl.platform.base import *
from onl.platform.accton import *

class OnlPlatform_x86_64_accton_asgvolt64_r0(OnlPlatformAccton,
                                             OnlPlatformPortConfig_20x100):
    PLATFORM='x86-64-accton-asgvolt64-r0'
    MODEL="ASGVOLT64"
    SYS_OBJECT_ID=".volt.64"

    def baseconfig(self):
        #self.insmod('ym2651y')
        self.insmod('optoe')
        for m in [ 'cpld', 'fan', 'psu', 'leds', 'thermal' ]:
            self.insmod("x86-64-accton-asgvolt64-%s.ko" % m)

        ########### initialize I2C bus 0 ###########
        self.new_i2c_devices([
            # initialize root(one-level) multiplexer (PCA9548)
            ('pca9548', 0x77, 0),
            # initiate (two-level) multiplexer (PCA9548)
            ('pca9548', 0x70, 1),
            # initiate (three-level) multiplexer (PCA9548)
            ('pca9548', 0x72, 16),
            
            # initiate (four-level) multiplexer (PCA9548) (for 25g-sfp)
            ('pca9548', 0x75, 19),
            # initiate (four-level) multiplexer (PCA9548) (for 8 pca mux to xfp)
            ('pca9548', 0x75, 22),
            
            # initiate (five-level) multiplexer (PCA9548) (for xfp)
            ('pca9548', 0x76, 33),
            ('pca9548', 0x76, 34),
            ('pca9548', 0x76, 35),
            ('pca9548', 0x76, 36),
            ('pca9548', 0x76, 37),
            ('pca9548', 0x76, 38),
            ('pca9548', 0x76, 39),
            ('pca9548', 0x76, 40),
            
            ('asgvlot64_cpld1', 0x60, 9),
            ('asgvlot64_cpld2', 0x61, 10),
            ('asgvlot64_cpld3', 0x62, 11)
        ])
       
        # initialize XFP port 1~64
        for port in range(41, 105):
            self.new_i2c_device('optoe2', 0x50, port)
            subprocess.call('echo port%d > /sys/bus/i2c/devices/%d-0050/port_name' % (port-40, port), shell=True)
            
        # initialize QSFP port 1~2 (port_name=port65~66)
        for port in range(20, 22):
            self.new_i2c_device('optoe1', 0x50, port)
            self.new_i2c_device('optoe1', 0x50, port)
            subprocess.call('echo port%d > /sys/bus/i2c/devices/%d-0050/port_name' % (port+45, port), shell=True)        
        
        # initialize SFP port 1~8 (port_name=port67~74)
        for port in range(25, 33):
            self.new_i2c_device('optoe2', 0x50, port)
            subprocess.call('echo port%d > /sys/bus/i2c/devices/%d-0050/port_name' % (port+41, port), shell=True)
        # initiate IDPROM
        self.new_i2c_device('24c02', 0x57, 0)
        

        return True

