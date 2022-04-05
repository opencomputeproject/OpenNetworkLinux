from onl.platform.base import *
from onl.platform.accton import *
from time import sleep

class OnlPlatform_x86_64_accton_as7315_30x_r0(OnlPlatformAccton,
                                              OnlPlatformPortConfig_2x100_8x25_16x10_4x1):
    PLATFORM='x86-64-accton-as7315-30x-r0'
    MODEL="AS7315-30X"
    SYS_OBJECT_ID=".7315.30"

    def baseconfig(self):
        os.system("modprobe i2c-ismt")
        os.system("modprobe pmbus_core")
        self.insmod('optoe')
        self.insmod("dps850")
        for m in [ 'fpga', 'cpld', 'sys', 'fan', 'fan-eeprom', 'psu', 'leds' ]:
            self.insmod("x86-64-accton-as7315-30x-%s" % m)

        ########### initialize I2C bus 0 ###########
        self.new_i2c_devices(
            [
                # initialize multiplexer (PCA9548)
                ('pca9548', 0x72, 0), # i2c  2-9
                ('pca9548', 0x70, 2), # i2c 10-17
                ('pca9548', 0x71, 3), # i2c 18-25
                ('pca9548', 0x73, 4), # i2c 26-33
                ]
            )

        # initialize SFP devices
        for port in range(1, 25):
            self.new_i2c_device('optoe2', 0x50, port+9)
            subprocess.call('echo port%d > /sys/bus/i2c/devices/%d-0050/port_name' % (port, port+9), shell=True)

        # initialize QSFP devices
        for port in range(25, 27):
            self.new_i2c_device('optoe1', 0x50, port-19)
            subprocess.call('echo port%d > /sys/bus/i2c/devices/%d-0050/port_name' % (port, port-19), shell=True)

        ########### initialize I2C bus 1 ###########
        self.new_i2c_devices(
            [
                # initialize multiplexer (PCA9548)
                ('pca9548', 0x76, 1),  # i2c 34-41
                ('pca9548', 0x74, 35), # i2c 42-49

                # initialize UCD90160
                ('ucd90160', 0x34, 43),

                # initialize CPLD
                ('as7315_30x_fpga', 0x64, 39), # i2c 50-53
                ('as7315_30x_cpld', 0x63, 40),
                ('as7315_30x_fan',  0x66, 41),
                ('as7315_fan_eeprom', 0x50, 41),

                # initiate PSU-1 AC Power
                ('as7315_30x_psu1',  0x53, 45),
                ('dps850',  0x5B, 45),

                # initiate PSU-2 AC Power
                ('as7315_30x_psu2',  0x50, 44),
                ('dps850',  0x58, 44),

                # initiate IDPROM
                ('as7315_30x_sys', 0x56, 1),
                ]
            )

        # sleep for a while to make sure the mux channels of fpga are created
        sleep(0.1)
        self.new_i2c_devices(
            [
                # inititate LM75
                ('lm75', 0x49, 51),
                ('lm75', 0x4a, 52),
                ('lm75', 0x4c, 53),
                ]
            )

        # Write sensors.conf for ucd90160
        lines = """
        bus "i2c-35" "i2c-27-mux (chan_id 1)"
            chip "ucd90160-i2c-*-34"
                label in1  "H_VCC12_MON"
                label in2  "VCC12_MON"
                label in3  "USB_VCC5P0_MON"
                label in4  "VCC5P0_MON"
                label in5  "VDD3P3_MON"
                label in6  "VDD1P8_MON"
                label in7  "VDD1P0_MON"
                label in8  "SFP_3P3_MON"
                label in9  "VDD1P0_ROV_MON"
                label in10 "VPP_DDR2P5_MON"
                label in11 "VDD1P2_DDR_MON"
                label in12 "VDD0P6_MON"
                label in13 "Vmonitor_3V3_MON"
                label in14 "FPGA_2V5_MON"
                label in15 "FPGA_1V2_MON"
                label in16 "MANG_2P5_MON"
        """
        with open("/etc/sensors.d/sensors.conf1","w+") as f:
            f.write(lines)

        return True
