from onl.platform.base import *
from onl.platform.accton import *

class OnlPlatform_x86_64_accton_csp9250_r0(OnlPlatformAccton,
                                              OnlPlatformPortConfig_48x25_6x100):

    PLATFORM='x86-64-accton-csp9250-r0'
    MODEL="CSP9250"
    SYS_OBJECT_ID=".9250.54"

    def baseconfig(self):
        self.insmod('ym2651y')
        for m in [ 'cpld', 'sfp', 'leds' ]:
            self.insmod("x86-64-accton-csp9250-%s.ko" % m)

        ########### initialize I2C bus 0 ###########

        # initiate multiplexer (PCA9548)
        self.new_i2c_devices(
            [
                # initiate multiplexer (PCA9548)
                ('pca9548', 0x71, 0),
                ('pca9548', 0x70, 0),
                ('pca9548', 0x72, 0),
                ('pca9548', 0x73, 0),
                ('pca9548', 0x74, 0),
                ('pca9548', 0x75, 0),
                ('pca9548', 0x76, 0),
                ('pca9548', 0x77, 0),
                ]
            )
       
        self.new_i2c_devices([
            # initialize CPLD
            ('accton_i2c_cpld', 0x60, 1),
            ('accton_i2c_cpld', 0x62, 2),
            ('accton_i2c_cpld', 0x64, 3),
            ])
        
        # initialize QSFP port 1~54
        self.new_i2c_devices(
            [
                ('csp9250_sfp1', 0x50, 17),
                ('csp9250_sfp2', 0x50, 18),
                ('csp9250_sfp3', 0x50, 19),
                ('csp9250_sfp4', 0x50, 20),
                ('csp9250_sfp5', 0x50, 21),
                ('csp9250_sfp6', 0x50, 22),
                ('csp9250_sfp7', 0x50, 23),
                ('csp9250_sfp8', 0x50, 24),                
                
                ('csp9250_sfp9' , 0x50, 25),                
                ('csp9250_sfp10', 0x50, 26),
                ('csp9250_sfp11', 0x50, 27),
                ('csp9250_sfp12', 0x50, 28),
                ('csp9250_sfp13', 0x50, 29),
                ('csp9250_sfp14', 0x50, 30),
                ('csp9250_sfp15', 0x50, 31),
                ('csp9250_sfp16', 0x50, 32),
                                
                ('csp9250_sfp17', 0x50, 33),
                ('csp9250_sfp18', 0x50, 34),                
                ('csp9250_sfp19', 0x50, 35),
                ('csp9250_sfp20', 0x50, 36),
                ('csp9250_sfp21', 0x50, 37),
                ('csp9250_sfp22', 0x50, 38),
                ('csp9250_sfp23', 0x50, 39),
                ('csp9250_sfp24', 0x50, 40),
                                
                ('csp9250_sfp25', 0x50, 41),
                ('csp9250_sfp26', 0x50, 42),
                ('csp9250_sfp27', 0x50, 43),                
                ('csp9250_sfp28', 0x50, 44),
                ('csp9250_sfp29', 0x50, 45),
                ('csp9250_sfp30', 0x50, 46),
                ('csp9250_sfp31', 0x50, 47),
                ('csp9250_sfp32', 0x50, 48),
                
                ('csp9250_sfp33', 0x50, 49),                
                ('csp9250_sfp34', 0x50, 50),
                ('csp9250_sfp35', 0x50, 51),
                ('csp9250_sfp36', 0x50, 52),                
                ('csp9250_sfp37', 0x50, 53),
                ('csp9250_sfp38', 0x50, 54),
                ('csp9250_sfp39', 0x50, 55),
                ('csp9250_sfp40', 0x50, 56),
                ('csp9250_sfp41', 0x50, 57),                
                ('csp9250_sfp42', 0x50, 58),
                
                ('csp9250_sfp43', 0x50, 59),
                ('csp9250_sfp44', 0x50, 60),
                ('csp9250_sfp45', 0x50, 61),                
                ('csp9250_sfp46', 0x50, 62),
                ('csp9250_sfp47', 0x50, 63),
                ('csp9250_sfp48', 0x50, 64),
                                
                ('csp9250_sfp49', 0x50, 9),
                ('csp9250_sfp50', 0x50, 10),
                ('csp9250_sfp51', 0x50, 11),
                ('csp9250_sfp52', 0x50, 12),
                ('csp9250_sfp53', 0x50, 13),
                ('csp9250_sfp54', 0x50, 14),
            ]
        )
        
        return True


