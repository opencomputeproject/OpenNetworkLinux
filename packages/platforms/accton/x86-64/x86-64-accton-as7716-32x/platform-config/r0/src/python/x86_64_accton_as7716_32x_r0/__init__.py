from onl.platform.base import *
from onl.platform.accton import *

class OnlPlatform_x86_64_accton_as7716_32x_r0(OnlPlatformAccton,
                                              OnlPlatformPortConfig_32x100):
    PLATFORM='x86-64-accton-as7716-32x-r0'
    MODEL="AS7716-32X"
    SYS_OBJECT_ID=".7716.32"

    def baseconfig(self):
        self.insmod('optoe')
        self.insmod('ym2651y')
        self.insmod('accton_i2c_cpld')
        for m in [ 'fan', 'cpld1', 'psu', 'leds' ]:
            self.insmod("x86-64-accton-as7716-32x-%s.ko" % m)

        ########### initialize I2C bus 0 ###########
        self.new_i2c_devices([
                # initialize multiplexer (PCA9548)
                ('pca9548', 0x77, 0),

                # initiate leaf multiplexer (PCA9548)
                ('pca9548', 0x76, 1),

                # initiate chassis fan
                ('as7716_32x_fan', 0x66, 9),

                # inititate LM75
                ('lm75', 0x48, 10),
                ('lm75', 0x49, 10),
                ('lm75', 0x4a, 10),

                #initiate CPLD
                ('as7716_32x_cpld1', 0x60, 11),
                ('accton_i2c_cpld', 0x62, 12),
                ('accton_i2c_cpld', 0x64, 13),

                # initiate leaf multiplexer (PCA9548)
                ('pca9548', 0x71, 2),

                # initiate PSU-1
                ('as7716_32x_psu1', 0x53, 18),
                ('ym2651', 0x5b, 18),

                # initiate PSU-2
                ('as7716_32x_psu2', 0x50, 17),
                ('ym2651', 0x58, 17),

                # initiate leaf multiplexer (PCA9548)
                ('pca9548', 0x72, 2),
                ('pca9548', 0x73, 2),
                ('pca9548', 0x74, 2),
                ('pca9548', 0x75, 2),

                # initialize QSFP port 1-32
                ('optoe1', 0x50, 25),
                ('optoe1', 0x50, 26),
                ('optoe1', 0x50, 27),
                ('optoe1', 0x50, 28),
                ('optoe1', 0x50, 29),
                ('optoe1', 0x50, 30),
                ('optoe1', 0x50, 31),
                ('optoe1', 0x50, 32),
                ('optoe1', 0x50, 33),
                ('optoe1', 0x50, 34),
                ('optoe1', 0x50, 35),
                ('optoe1', 0x50, 36),
                ('optoe1', 0x50, 37),
                ('optoe1', 0x50, 38),
                ('optoe1', 0x50, 39),
                ('optoe1', 0x50, 40),
                ('optoe1', 0x50, 41),
                ('optoe1', 0x50, 42),
                ('optoe1', 0x50, 43),
                ('optoe1', 0x50, 44),
                ('optoe1', 0x50, 45),
                ('optoe1', 0x50, 46),
                ('optoe1', 0x50, 47),
                ('optoe1', 0x50, 48),
                ('optoe1', 0x50, 49),
                ('optoe1', 0x50, 50),
                ('optoe1', 0x50, 51),
                ('optoe1', 0x50, 52),
                ('optoe1', 0x50, 53),
                ('optoe1', 0x50, 54),
                ('optoe1', 0x50, 55),
                ('optoe1', 0x50, 56),
                #('24c02', 0x56, 0),
                ])

        subprocess.call('echo port9 > /sys/bus/i2c/devices/25-0050/port_name', shell=True)
        subprocess.call('echo port10 > /sys/bus/i2c/devices/26-0050/port_name', shell=True)
        subprocess.call('echo port11 > /sys/bus/i2c/devices/27-0050/port_name', shell=True)
        subprocess.call('echo port12 > /sys/bus/i2c/devices/28-0050/port_name', shell=True)
        subprocess.call('echo port1 > /sys/bus/i2c/devices/29-0050/port_name', shell=True)
        subprocess.call('echo port2 > /sys/bus/i2c/devices/30-0050/port_name', shell=True)
        subprocess.call('echo port3 > /sys/bus/i2c/devices/31-0050/port_name', shell=True)
        subprocess.call('echo port4 > /sys/bus/i2c/devices/32-0050/port_name', shell=True)
        subprocess.call('echo port6 > /sys/bus/i2c/devices/33-0050/port_name', shell=True)
        subprocess.call('echo port5 > /sys/bus/i2c/devices/34-0050/port_name', shell=True)
        subprocess.call('echo port8 > /sys/bus/i2c/devices/35-0050/port_name', shell=True)
        subprocess.call('echo port7 > /sys/bus/i2c/devices/36-0050/port_name', shell=True)
        subprocess.call('echo port13 > /sys/bus/i2c/devices/37-0050/port_name', shell=True)
        subprocess.call('echo port14 > /sys/bus/i2c/devices/38-0050/port_name', shell=True)
        subprocess.call('echo port15 > /sys/bus/i2c/devices/39-0050/port_name', shell=True)
        subprocess.call('echo port16 > /sys/bus/i2c/devices/40-0050/port_name', shell=True)
        subprocess.call('echo port17 > /sys/bus/i2c/devices/41-0050/port_name', shell=True)
        subprocess.call('echo port18 > /sys/bus/i2c/devices/42-0050/port_name', shell=True)
        subprocess.call('echo port19 > /sys/bus/i2c/devices/43-0050/port_name', shell=True)
        subprocess.call('echo port20 > /sys/bus/i2c/devices/44-0050/port_name', shell=True)
        subprocess.call('echo port25 > /sys/bus/i2c/devices/45-0050/port_name', shell=True)
        subprocess.call('echo port26 > /sys/bus/i2c/devices/46-0050/port_name', shell=True)
        subprocess.call('echo port27 > /sys/bus/i2c/devices/47-0050/port_name', shell=True)
        subprocess.call('echo port28 > /sys/bus/i2c/devices/48-0050/port_name', shell=True)
        subprocess.call('echo port29 > /sys/bus/i2c/devices/49-0050/port_name', shell=True)
        subprocess.call('echo port30 > /sys/bus/i2c/devices/50-0050/port_name', shell=True)
        subprocess.call('echo port31 > /sys/bus/i2c/devices/51-0050/port_name', shell=True)
        subprocess.call('echo port32 > /sys/bus/i2c/devices/52-0050/port_name', shell=True)
        subprocess.call('echo port21 > /sys/bus/i2c/devices/53-0050/port_name', shell=True)
        subprocess.call('echo port22 > /sys/bus/i2c/devices/54-0050/port_name', shell=True)
        subprocess.call('echo port23 > /sys/bus/i2c/devices/55-0050/port_name', shell=True)
        subprocess.call('echo port24 > /sys/bus/i2c/devices/56-0050/port_name', shell=True)
        return True
