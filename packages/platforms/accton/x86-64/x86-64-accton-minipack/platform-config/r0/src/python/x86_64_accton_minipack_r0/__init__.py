from onl.platform.base import *
from onl.platform.accton import *

class OnlPlatform_x86_64_accton_minipack_r0(OnlPlatformAccton,
                                                OnlPlatformPortConfig_128x100):
    MODEL="Minipack"
    PLATFORM="x86-64-accton-minipack-r0"
    SYS_OBJECT_ID=".100.128.2"

    def baseconfig(self):
        self.insmod('optoe')
        self.insmod_platform()
        
        ########### initialize I2C bus 1 ###########
        # initialize level 1 multiplexer (PCA9548)
        self.new_i2c_devices([
                ('pca9548', 0x70, 1),
                ('24c64', 0x57, 1),
                ])
                
        # initialize multiplexer for 8 PIMs
        for pim in range(2, 10):
            self.new_i2c_devices([
                ('pca9548', 0x72, pim),
                ('pca9548', 0x71, pim),
                ])

        return True
        # Initialize QSFP devices
        for port in range(1, 129):
            base = ((port-1)/8*8) + 10
            index = (port - 1) % 8
            index = 7 - index
            if (index%2):
                index = index -1
            else:
                index = index +1 
            bus = base + index 
            self.new_i2c_device('optoe1', 0x50, bus)
            subprocess.call('echo port%d > /sys/bus/i2c/devices/%d-0050/port_name' % (port, bus), shell=True)

        return True
