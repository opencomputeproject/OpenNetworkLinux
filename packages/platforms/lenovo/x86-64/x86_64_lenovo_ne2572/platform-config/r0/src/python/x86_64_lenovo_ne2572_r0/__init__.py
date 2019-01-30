from onl.platform.base import *
from onl.platform.lenovo import *

class OnlPlatform_x86_64_lenovo_ne2572_r0(OnlPlatformLenovo,
                                              OnlPlatformPortConfig_32x100):
    PLATFORM='x86-64-lenovo-ne2572-r0'
    MODEL="NE2572"
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
        self.new_i2c_devices([
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
        self.new_i2c_devices([

            # initialize multiplexer (PCA9548 #3)
            ('pca9548', 0x74, 14),

            # initialize multiplexer (PCA9548 #4)
            ('pca9548', 0x75, 14),

            # initialize multiplexer (PCA9548 #5)
            ('pca9548', 0x76, 14),

            # initialize multiplexer (PCA9548 #6)
            ('pca9548', 0x74, 16),

            # initialize multiplexer (PCA9548 #7)
            ('pca9548', 0x75, 16),

            # initialize multiplexer (PCA9548 #8)
            ('pca9548', 0x76, 16),

            # initialize multiplexer (PCA9548 #9)
            ('pca9548', 0x74, 18),

            # IR3595
            ('ir3595', 0x09, 19),
            
            # initialize multiplexer (PCA9548 #2)
            ('pca9548', 0x73, 20),
            ])

        ########### initialize I2C bus PCA9548 #3 @Fan Expander ###########
        self.new_i2c_devices([
                
            ('SFP', 0x50, 21),
            ('SFP', 0x50, 22),
            ('SFP', 0x50, 23),
            ('SFP', 0x50, 24),
            ('SFP', 0x50, 25),
            ('SFP', 0x50, 26),
            ('SFP', 0x50, 27),
            ('SFP', 0x50, 28),

            ])

        ########### initialize I2C bus PCA9548 #4 @Fan Expander ###########
        self.new_i2c_devices([
                
            ('SFP', 0x50, 29),
            ('SFP', 0x50, 30),
            ('SFP', 0x50, 31),
            ('SFP', 0x50, 32),
            ('SFP', 0x50, 33),
            ('SFP', 0x50, 34),
            ('SFP', 0x50, 35),
            ('SFP', 0x50, 36),

            ])

        ########### initialize I2C bus PCA9548 #5 @Fan Expander ###########
        self.new_i2c_devices([
                
            ('SFP', 0x50, 37),
            ('SFP', 0x50, 38),
            ('SFP', 0x50, 39),
            ('SFP', 0x50, 40),
            ('SFP', 0x50, 41),
            ('SFP', 0x50, 42),
            ('SFP', 0x50, 43),
            ('SFP', 0x50, 44),

            ])

        ########### initialize I2C bus PCA9548 #6 @Fan Expander ###########
        self.new_i2c_devices([
                
            ('SFP', 0x50, 45),
            ('SFP', 0x50, 46),
            ('SFP', 0x50, 47),
            ('SFP', 0x50, 48),
            ('SFP', 0x50, 49),
            ('SFP', 0x50, 50),
            ('SFP', 0x50, 51),
            ('SFP', 0x50, 52),

            ])

        ########### initialize I2C bus PCA9548 #7 @Fan Expander ###########
        self.new_i2c_devices([
                
            ('SFP', 0x50, 53),
            ('SFP', 0x50, 54),
            ('SFP', 0x50, 55),
            ('SFP', 0x50, 56),
            ('SFP', 0x50, 57),
            ('SFP', 0x50, 58),
            ('SFP', 0x50, 59),
            ('SFP', 0x50, 60),

            ])

        ########### initialize I2C bus PCA9548 #8 @Fan Expander ###########
        self.new_i2c_devices([
                
            ('SFP', 0x50, 61),
            ('SFP', 0x50, 62),
            ('SFP', 0x50, 63),
            ('SFP', 0x50, 64),
            ('SFP', 0x50, 65),
            ('SFP', 0x50, 66),
            ('SFP', 0x50, 67),
            ('SFP', 0x50, 68),

            ])

        ########### initialize I2C bus PCA9548 #9 @Fan Expander ###########
        self.new_i2c_devices([
                
            ('QSFP', 0x50, 69),
            ('QSFP', 0x50, 70),
            ('QSFP', 0x50, 71),
            ('QSFP', 0x50, 72),
            ('QSFP', 0x50, 73),
            ('QSFP', 0x50, 74),

            ])

        ########### initialize I2C bus PCA9548 #2 @Fan Expander ###########
        self.new_i2c_devices([
            # Fan EEROM
            ('24c02', 0x57, 77),
            # Fan EEROM
            ('24c02', 0x57, 78),
            # Fan EEROM
            ('24c02', 0x57, 79),
            # Fan EEROM
            ('24c02', 0x57, 80),
            # Fan EEROM
            ('24c02', 0x57, 81),
            # Fan EEROM
            ('24c02', 0x57, 82),

            ])


        self.new_i2c_devices([
                
            (' ', 0x68, 69),
            (' ', 0x68, 70),
            (' ', 0x68, 71),
            (' ', 0x68, 72),
            (' ', 0x68, 73),
            (' ', 0x68, 74),

            # Port CPLD0~2
            ('cpld', 0x5B, 13),
            ('cpld', 0x5C, 15),
            ('cpld', 0x5D, 17),

            ])

        return True
