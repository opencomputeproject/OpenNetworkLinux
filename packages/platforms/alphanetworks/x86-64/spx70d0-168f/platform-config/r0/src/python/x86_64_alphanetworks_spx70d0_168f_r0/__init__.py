from onl.platform.base import *
from onl.platform.alphanetworks import *

class OnlPlatform_x86_64_alphanetworks_spx70d0_168f_r0(OnlPlatformAlphaNetworks,
                                              OnlPlatformPortConfig_16x10_6x25_2x100):
    PLATFORM='x86-64-alphanetworks-spx70d0-168f-r0'
    MODEL="SPX70D0-168F"
    SYS_OBJECT_ID=".6040.24"

    def baseconfig(self):
        ########### initialize I2C bus 0 ###########
        self.new_i2c_devices([

            # TPS53647 @MB
            ('tps53647', 0x65, 0),

            # TPS53667 @MB
            ('tps53667', 0x66, 0),

            # 8A34002 @MB
            ('8a34002', 0x5B, 0),

            # ONIE EEPROM @MB (FM24C128A)
            ('fm24c128a', 0x56, 0),
			
            # initialize multiplexer (PCA9548 #0)
            ('pca9548', 0x70, 0),

            # UCD90120A @MB
            ('ucd90120a', 0x34, 0),

            # Power CPLD @MB (5M1270ZF256C5N)
            ('powercpld', 0x5F, 0),

            # FRU EEPROM @MB (FM24C512D)
            ('fm24c512d', 0x57, 0),

            # Thermal Ambient @MB (TMP1075 #0)
            ('tmp1075', 0x48, 0),

            # Thermal Hot Spot @MB (TMP1075 #1)
            ('tmp1075', 0x49, 0),

            # Thermal sensor @MB (TMP435 #0)
            ('tmp435', 0x4D, 0),

            ])

        ########### initialize I2C bus PCA9548 #0 ###########
        self.new_i2c_devices(
            [
            # CFG EEPROM @MB (AT24C02D)
            ('at24c02d', 0x50, 1),

            # PCA9539#1 @MB
            ('pca9539', 0x75, 1),

            # PCA9539#0 @MB
            ('pca9539', 0x76, 1),
                      
            # initialize multiplexer (PCA9548 #1)
            ('pca9548', 0x71, 2),

            # PCA9555#0 @MB
            ('pca9555', 0x20, 2),

            # PCA9555#1 @MB
            ('pca9555', 0x21, 2),

            # initialize multiplexer (PCA9548 #2)
            ('pca9548', 0x72, 2),

            # PCA9555#2 @MB
            ('pca9555', 0x22, 2),

            # PCA9555#3 @MB
            ('pca9555', 0x23, 2),

            # initialize multiplexer (PCA9548 #3)
            ('pca9548', 0x73, 2),

            # PCA9555#4 @MB
            ('pca9555', 0x24, 2),

            # PCA9555#5 @MB
            ('pca9555', 0x25, 2),
                      
            # MAC @MB
            ('bcm88470', 0x47, 4),

            # PSU #0
            ('psu', 0x58, 5),

            # PSU #1
            ('psu', 0x59, 5),
			
            ])

        ########### initialize I2C bus PCA9548 #1 ###########
        self.new_i2c_devices(
            [
            # QSFP28 1 uplink
            ('spx70d0_qsfp1', 0x50, 9),

            # QSFP28 2 uplink
            ('spx70d0_qsfp2', 0x50, 10),

            # SFP28 3 downlink
            ('spx70d0_sfp28_3', 0x50, 11),

            # SFP28 4 downlink
            ('spx70d0_sfp28_4', 0x50, 12),

            # SFP28 5 downlink
            ('spx70d0_sfp28_5', 0x50, 13),

            # SFP28 6 downlink
            ('spx70d0_sfp28_6', 0x50, 14),

            # SFP28 7 downlink
            ('spx70d0_sfp28_7', 0x50, 15),

            # SFP28 8 downlink
            ('spx70d0_sfp28_8', 0x50, 16),
            
            ])

        ########### initialize I2C bus PCA9548 #2 ###########
        self.new_i2c_devices(
            [
            # SFP+ 9 downlink
            ('spx70d0_sfpp9', 0x50, 17),
            
            # SFP+ 10 downlink
            ('spx70d0_sfpp10', 0x50, 18),

            # SFP+ 11 downlink
            ('spx70d0_sfpp11', 0x50, 19),

            # SFP+ 12 downlink
            ('spx70d0_sfpp12', 0x50, 20),

            # SFP+ 13 downlink
            ('spx70d0_sfpp13', 0x50, 21),

            # SFP+ 14 downlink
            ('spx70d0_sfpp14', 0x50, 22),

            # SFP+ 15 downlink
            ('spx70d0_sfpp15', 0x50, 23),

            # SFP+ 16 downlink
            ('spx70d0_sfpp16', 0x50, 24),
            
            ])

        ########### initialize I2C bus PCA9548 #3 ###########
        self.new_i2c_devices(
            [
            # SFP+ 17 downlink
            ('spx70d0_sfpp17', 0x50, 25),

            # SFP+ 18 downlink
            ('spx70d0_sfpp18', 0x50, 26),

            # SFP+ 19 downlink
            ('spx70d0_sfpp19', 0x50, 27),

            # SFP+ 20 downlink
            ('spx70d0_sfpp20', 0x50, 28),

            # SFP+ 21 downlink
            ('spx70d0_sfpp21', 0x50, 29),

            # SFP+ 22 downlink
            ('spx70d0_sfpp22', 0x50, 30),

            # SFP+ 23 downlink
            ('spx70d0_sfpp23', 0x50, 31),

            # SFP+ 24 downlink
            ('spx70d0_sfpp24', 0x50, 32),
            
            ])

        return True
