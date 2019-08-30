from onl.platform.base import *
from onl.platform.alphanetworks import *

class OnlPlatform_x86_64_alphanetworks_stx60d0_062f_r0(OnlPlatformAlphanetworks,
                                              OnlPlatformPortConfig_8x1_8x10):
    PLATFORM='x86-64-alphanetworks-stx60d0-062f-r0'
    MODEL="STX60D0-062F"
    SYS_OBJECT_ID=".6040.8"

    def baseconfig(self):
        ########### initialize I2C bus 0 ###########
        self.new_i2c_devices([

            # LMK05318 @MB
            ('lmk05318', 0x66, 0),

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
			
            # PCA9539#0 @MB
            ('pca9539', 0x76, 8),
			
            ])

        ########### initialize I2C bus PCA9548 #1 ###########
        self.new_i2c_devices(
            [
			# SFP+ 1
            ('SFP+', 0x50, 9),

			# SFP+ 2
            ('SFP+', 0x50, 10),

			# SFP+ 1 uplink
            ('SFP+', 0x50, 11),

			# SFP+ 2 uplink
            ('SFP+', 0x50, 12),
			
            # PCA9539#1 @MB
            ('pca9539', 0x76, 13),
            
            # reserved
            
            ])

        return True
