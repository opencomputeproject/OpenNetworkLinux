from onl.platform.base import *
from onl.platform.accton import *

class OnlPlatform_x86_64_accton_minipack_r0(OnlPlatformAccton,
                                                OnlPlatformPortConfig_128x100):
    MODEL="Minipack"
    PLATFORM="x86-64-accton-minipack-r0"
    SYS_OBJECT_ID=".100.128.2"

    def baseconfig(self):
        self.insmod('optoe')
        
        ########### initialize I2C bus 1 ###########
        # initialize level 1 multiplexer (PCA9548)
        self.new_i2c_devices([
                ('pca9548', 0x70, 0),
                ('24c64', 0x57, 0),
                ])
                
        # initialize multiplexer for 8 PIMs
        for pim in range(1, 9):
            self.new_i2c_devices([
                ('pca9548', 0x72, pim),
                ('pca9548', 0x71, pim),
                ])

        # Initialize QSFP devices
        for port in range(1, 129):
            self.new_i2c_device('optoe1', 0x50, port+8)
            base = ((port-1)/8*8) + 9
            index = (port - 1) % 8
            index = 7 - index
            if (index%2):
                index = index -1
            else:
                index = index +1 
            bus = base + index 
            subprocess.call('echo port%d > /sys/bus/i2c/devices/%d-0050/port_name' % (port, bus), shell=True)

        return True
