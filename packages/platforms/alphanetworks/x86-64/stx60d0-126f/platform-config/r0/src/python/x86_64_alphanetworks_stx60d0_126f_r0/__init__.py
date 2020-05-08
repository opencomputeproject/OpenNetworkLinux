from onl.platform.base import *
from onl.platform.alphanetworks import *

class OnlPlatform_x86_64_alphanetworks_stx60d0_126f_r0(OnlPlatformAlphaNetworks,
                                              OnlPlatformPortConfig_8x1_8x10):
    PLATFORM='x86-64-alphanetworks-stx60d0-126f-r0'
    MODEL="STX60D0-126F"
    SYS_OBJECT_ID=".6040.8"

    def baseconfig(self):
        ########### initialize I2C bus 0 ###########
        self.new_i2c_devices([

            # 8A34002 @MB
            ('8a34002', 0x58, 0),

            # ONIE EEPROM @MB
            ('fm24c512d', 0x56, 0),
			
            # PCA9539#2 @MB
            ('pca9539', 0x75, 0),
			
            # initialize multiplexer (PCA9548 #0)
            ('pca9548', 0x70, 0),		

            ])

        ########### initialize I2C bus PCA9548 #0 ###########
        self.new_i2c_devices(
            [
            # CFG EEPROM @MB
            ('fm24c512d', 0x51, 1),

            # initialize multiplexer (PCA9548 #2)
            ('pca9548', 0x72, 3),

            # PCA9539#1 @MB
            ('pca9539', 0x76, 3),

            # PCA9539#3 @MB
            ('pca9539', 0x77, 3),
			
            # MAC @MB
            ('bcm88272', 0x47, 4),

            # PSU #0
            ('psu', 0x58, 5),
            ('psu', 0x50, 5),
            # PSU #1
            ('psu', 0x58, 6),
            ('psu', 0x50, 6),
			
            # initialize multiplexer (PCA9548 #1)
            ('pca9548', 0x72, 7),

            # TCA6424#0 @MB
            ('tca6424', 0x22, 7),
			
            # PCA9539#0 @MB
            ('pca9539', 0x76, 8),
			
            ])

        ########### initialize I2C bus PCA9548 #2 ###########
        self.new_i2c_devices(
            [
			# SFP 1
            ('SFP', 0x50, 9),

			# SFP 2
            ('SFP', 0x50, 10),

			# SFP 3
            ('SFP', 0x50, 11),

			# SFP 4
            ('SFP', 0x50, 12),
			
			# SFP 5
            ('SFP', 0x50, 13),
			
			# SFP 6
            ('SFP', 0x50, 14),
			
			# SFP 7
            ('SFP', 0x50, 15),
			
			# SFP 8
            ('SFP', 0x50, 16),
            
            ])
			
        ########### initialize I2C bus PCA9548 #1 ###########
        self.new_i2c_devices(
            [
			# SFP+ 1
            ('SFP+', 0x50, 17),

			# SFP+ 2
            ('SFP+', 0x50, 18),

			# SFP+ 3
            ('SFP+', 0x50, 19),

			# SFP+ 4
            ('SFP+', 0x50, 20),
			
			# SFP+ 5
            ('SFP+', 0x50, 21),
			
			# SFP+ 6
            ('SFP+', 0x50, 22),
			            
            # reserved
            
            ])

        return True
