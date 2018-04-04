from onl.platform.base import *
from onl.platform.accton import *

class OnlPlatform_x86_64_accton_as5916_54xm_r0(OnlPlatformAccton,
                                              OnlPlatformPortConfig_48x10_6x40):
    PLATFORM='x86-64-accton-as5916-54xm-r0'
    MODEL="AS5916-54XM"
    SYS_OBJECT_ID=".5916.54"

    def baseconfig(self):
        self.insmod('optoe')
        self.insmod("ym2651y")
        for m in [ "cpld", "psu", "fan", "leds" ]:
            self.insmod("x86-64-accton-as5916-54xm-%s" % m)

        ########### initialize I2C bus 0 ###########
        self.new_i2c_devices([
                # initialize multiplexer (PCA9548)
                ('pca9548', 0x77, 0),

                # initiate IDPROM
                ('24c02', 0x56, 0),
                ])

        self.new_i2c_devices([
                # initialize multiplexer (PCA9548)
                ('pca9548', 0x76, 1),
                
                # initiate chassis fan
                ('as5916_54xm_fan', 0x66, 9),

                # inititate LM75
                ('lm75', 0x48, 10),
                ('lm75', 0x49, 10),
                ('lm75', 0x4a, 10),
                ('lm75', 0x4b, 10),

                # initialize CPLDs
                ('as5916_54xm_cpld1', 0x60, 11),
                ('as5916_54xm_cpld2', 0x62, 12),
                ]
            )

        self.new_i2c_devices(
            [
                # initialize multiplexer (PCA9548)
                ('pca9548', 0x74, 1),
                
                # initiate PSU-1 AC Power
                ('as5916_54xm_psu1', 0x53, 18),
                ('ym2651', 0x5b, 18),

                # initiate PSU-2 AC Power
                ('as5916_54xm_psu2', 0x50, 17),
                ('ym2651', 0x58, 17),

                ]
            )

        self.new_i2c_devices(
            [
                # initialize multiplexer (PCA9548)
                ('pca9548', 0x70, 1),
            ]
            )
        # initialize QSFP devices
        for port in range(49, 55):
            self.new_i2c_device('optoe1', 0x50, port-24)
            subprocess.call('echo port%d > /sys/bus/i2c/devices/%d-0050/port_name' % (port, port-24), shell=True)

        ########### initialize I2C bus 1 ###########

        # initialize multiplexer (PCA9548) for SFP ports        self.new_i2c_devices(
        self.new_i2c_devices(
            [
                ('pca9548', 0x76, 2),
                ('pca9548', 0x70, 33),
                ('pca9548', 0x71, 34),
                ('pca9548', 0x72, 35),
                ('pca9548', 0x73, 36),
                ('pca9548', 0x74, 37),
                ('pca9548', 0x75, 38),
                ]
            )

        # initialize SFP devices
        for port in range(1, 49):
            self.new_i2c_device('optoe2', 0x50, port+40)
            subprocess.call('echo port%d > /sys/bus/i2c/devices/%d-0050/port_name' % (port, port+40), shell=True)

        return True

