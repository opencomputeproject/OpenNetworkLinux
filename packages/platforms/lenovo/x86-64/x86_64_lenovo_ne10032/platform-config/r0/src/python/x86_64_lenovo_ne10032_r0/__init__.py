from onl.platform.base import *
from onl.platform.lenovo import *

class OnlPlatform_x86_64_lenovo_ne10032_r0(OnlPlatformLenovo,
                                              OnlPlatformPortConfig_32x100):
    PLATFORM='x86-64-lenovo-ne10032-r0'
    MODEL="NE10032"
    SYS_OBJECT_ID=".6010.32"

    def baseconfig(self):
        ########### initialize I2C bus 0 ###########
        self.new_i2c_devices([

            # ONIE EEPROM @MB
            ('24c128', 0x56, 0),
            
            # PowerCPLD @MB
            ('cpld', 0x5E, 0),

            # initialize multiplexer (PCA9548 #0)
            ('pca9548', 0x70, 0),

            # FRU EEROM @MB
            ('24c512', 0x51, 1),

            # CFG EEROM @MB
            ('24c02', 0x51, 2),

            # TMP75#1 (Hot Spot) #use lm75 will cause device busy, use other name
            ('thermal1', 0x4D, 3),

            # TMP75#0 (Ambient)  #use lm75 will cause device busy, use other name
            ('thermal2', 0x4C, 4),

            # initialize multiplexer (PCA9545 #0)
            ('pca9545', 0x71, 5),

            # IDT 5P49V5923A (Clock generator)
            (' ', 0x6A, 6),

            # initialize multiplexer (PCA9548 #1)
            ('pca9548', 0x72, 7),

            # SystemCPLD @MB
            ('cpld', 0x5F, 8),

            ])

        ########### initialize I2C bus PCA9545 #0 ###########
        self.new_i2c_devices(
            [
            # MGMT EEROM
            ('24c02', 0x51, 9),
            # PSU #0
            ('psu', 0x51, 10),
            ('psu', 0x59, 10),
            # PSU #1
            ('psu', 0x51, 11),
            ('psu', 0x59, 11),
            # reserved 
            ])

        ########### initialize I2C bus PCA9548 #1 ###########
        self.new_i2c_devices(
            [
            # IR3595
            ('ir3595', 0x09, 18),
            
            # reserved
            
            # initialize multiplexer (PCA9548 #1)
            ('pca9548', 0x73, 20),
            ])


        ########### initialize I2C bus PCA9548 #2 @Fan Expander ###########
        self.new_i2c_devices(
            [
            # Fan EEROM
            ('24c02', 0x57, 21),
            # Fan EEROM
            ('24c02', 0x57, 22),
            # Fan EEROM
            ('24c02', 0x57, 23),
            # Fan EEROM
            ('24c02', 0x57, 24),
            # Fan EEROM
            ('24c02', 0x57, 25),
            # Fan EEROM
            ('24c02', 0x57, 26),

            ])


        # initialize QSFP port 1~32
        self.new_i2c_devices([
                
            ('QSFP28', 0x50, 13),
            ('QSFP28', 0x50, 14),
            ('QSFP28', 0x50, 15),
            ('QSFP28', 0x50, 16),

            (' ', 0x68, 13),
            (' ', 0x68, 14),
            (' ', 0x68, 15),
            (' ', 0x68, 16),

            # Port CPLD0~3
            ('cpld', 0x5F, 13),
            ('cpld', 0x5F, 14),
            ('cpld', 0x5F, 15),
            ('cpld', 0x5F, 16),

            ])

        return True
