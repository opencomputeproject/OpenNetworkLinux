from onl.platform.base import *
from onl.platform.alphanetworks import *

class OnlPlatform_x86_64_alphanetworks_scg60d0_484t_r0(OnlPlatformAlphaNetworks,
                                              OnlPlatformPortConfig_48x1_4x25):
    PLATFORM='x86-64-alphanetworks-scg60d0-484t-r0'
    MODEL="SCG60D0-484T"
    SYS_OBJECT_ID=".6040.8"

    def baseconfig(self):
        self.insmod('scg60d0-484t_onie_eeprom')
        self.insmod('scg60d0-484t_psu')

        for m in [ 'pwr_cpld', 'fan', 'thermal', 'led' ]:
            self.insmod("scg60d0-484t_%s.ko" % m)

        ########### initialize I2C bus 0 ###########
        self.new_i2c_devices([

            # ONIE EEPROM @MB
            ('scg60d0_onie_eeprom', 0x56, 0),
            
            # PCA9539 #2 
            ('pca9539', 0x75, 0),

            # initialize multiplexer (PCA9548 #0)
            ('pca9548', 0x70, 0),		

            ])

        ########### initialize I2C bus PCA9548 #0 ###########
        self.new_i2c_devices(
            [

            # reserved
            # reserved
            # reserved
            # reserved

            # PSU #1
            ('24c02', 0x50, 5),
            ('scg60d0_psu', 0x58, 5),
            # PSU #0
            ('24c02', 0x50, 6),
            ('scg60d0_psu', 0x58, 6),
            
            # initialize multiplexer (PCA9548 #1)
            ('pca9548', 0x72, 7),
            # PCA9539 #1
            ('pca9539', 0x76, 7),

            # PCA9539 #0
            ('pca9539', 0x77, 8),

            # reserved
            
            ])

        ########### initialize I2C bus PCA9548 #1 ###########
        self.new_i2c_devices(
            [
            # SFP28 Port 1
            ('optoe2', 0x50, 9),
            # SFP28 Port 2 
            ('optoe2', 0x50, 10),
            # SFP28 Port 3
            ('optoe2', 0x50, 11),
            # SFP28 Port 4
            ('optoe2', 0x50, 12),

            # reserved
            # reserved
            # reserved
            # reserved
            
            ])        
        

        # initialize sys led (PCA9539#2 @MB) (0x75)
        subprocess.call('echo 496 > /sys/class/gpio/export', shell=True)
        subprocess.call('echo 497 > /sys/class/gpio/export', shell=True)
        subprocess.call('echo 498 > /sys/class/gpio/export', shell=True)
        subprocess.call('echo 499 > /sys/class/gpio/export', shell=True)

        subprocess.call('echo out > /sys/class/gpio/gpio496/direction', shell=True)
        subprocess.call('echo out > /sys/class/gpio/gpio497/direction', shell=True)
        subprocess.call('echo out > /sys/class/gpio/gpio498/direction', shell=True)
        subprocess.call('echo out > /sys/class/gpio/gpio499/direction', shell=True)

        #default green for SYSTEM led and FAN LED
        subprocess.call('echo 1 > /sys/class/gpio/gpio497/value', shell=True)
        subprocess.call('echo 1 > /sys/class/gpio/gpio499/value', shell=True)


        # initialize sys led (PCA9539#1 @MB) (0x76)
        subprocess.call('echo 480 > /sys/class/gpio/export', shell=True)
        subprocess.call('echo 481 > /sys/class/gpio/export', shell=True)
        subprocess.call('echo 482 > /sys/class/gpio/export', shell=True)
        subprocess.call('echo 483 > /sys/class/gpio/export', shell=True)
        subprocess.call('echo 484 > /sys/class/gpio/export', shell=True)
        subprocess.call('echo 485 > /sys/class/gpio/export', shell=True)
        subprocess.call('echo 486 > /sys/class/gpio/export', shell=True)
        subprocess.call('echo 487 > /sys/class/gpio/export', shell=True)
        subprocess.call('echo 488 > /sys/class/gpio/export', shell=True)
        subprocess.call('echo 489 > /sys/class/gpio/export', shell=True)
        subprocess.call('echo 490 > /sys/class/gpio/export', shell=True)
        subprocess.call('echo 491 > /sys/class/gpio/export', shell=True)
        subprocess.call('echo 492 > /sys/class/gpio/export', shell=True)
        subprocess.call('echo 493 > /sys/class/gpio/export', shell=True)
        subprocess.call('echo 494 > /sys/class/gpio/export', shell=True)
        subprocess.call('echo 495 > /sys/class/gpio/export', shell=True)

        subprocess.call('echo out > /sys/class/gpio/gpio480/direction', shell=True)
        subprocess.call('echo out > /sys/class/gpio/gpio484/direction', shell=True)
        subprocess.call('echo out > /sys/class/gpio/gpio488/direction', shell=True)
        subprocess.call('echo out > /sys/class/gpio/gpio492/direction', shell=True)

        #default TX_DISABLE = false
        subprocess.call('echo 0 > /sys/class/gpio/gpio480/value', shell=True)
        subprocess.call('echo 0 > /sys/class/gpio/gpio484/value', shell=True)
        subprocess.call('echo 0 > /sys/class/gpio/gpio488/value', shell=True)
        subprocess.call('echo 0 > /sys/class/gpio/gpio492/value', shell=True)

        
        return True
