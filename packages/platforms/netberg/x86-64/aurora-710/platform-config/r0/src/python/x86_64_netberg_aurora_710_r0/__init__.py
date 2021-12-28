
from onl.platform.base import *
from onl.platform.netberg import *
import os
import sys
import subprocess


def msg(s, fatal=False):
    sys.stderr.write(s)
    sys.stderr.flush()
    if fatal:
        sys.exit(1)


class OnlPlatform_x86_64_netberg_aurora_710_r0(OnlPlatformNetberg):
    PLATFORM = 'x86-64-netberg-aurora-710-r0'
    MODEL = "NBA710"
    SYS_OBJECT_ID = ".710.1"

    def baseconfig(self):
        self.insmod('optoe')

        # vid to mac vdd value mapping
        vdd_val_array = (0.85, 0.82, 0.77, 0.87, 0.74, 0.84, 0.79, 0.89)
        # vid to rov reg value mapping
        rov_reg_array = (0x24, 0x21, 0x1C, 0x26, 0x19, 0x23, 0x1E, 0x28)

        self.insmod("eeprom_mb")
        # init SYS EEPROM devices
        self.new_i2c_device('mb_eeprom', 0x55, 0)

        os.system("modprobe w83795")
        os.system("modprobe eeprom")

        ########### initialize I2C bus 0 ###########
        # init PCA9548
        self.new_i2c_devices(
            [
                ('pca9548', 0x70, 0),
                ('pca9548', 0x71, 1),
                ('pca9548', 0x71, 2),
                ('pca9548', 0x71, 3),
                ('pca9548', 0x71, 4),
                ('pca9548', 0x71, 7),
                ('pca9548', 0x76, 0),
            ]
        )

        # init PCA9545
        self.new_i2c_device('pca9545', 0x72, 0)

        # Golden Finger to show CPLD
        os.system("i2cget -y 44 0x74 2")

        # Reset BMC Dummy Board
        os.system("i2cset -y -r 0 0x26 4 0x00")
        os.system("i2cset -y -r 0 0x26 5 0x00")
        os.system("i2cset -y -r 0 0x26 2 0x3F")
        os.system("i2cset -y -r 0 0x26 3 0x1F")
        os.system("i2cset -y -r 0 0x26 6 0xC0")
        os.system("i2cset -y -r 0 0x26 7 0x00")

        # CPU Baord
        os.system("i2cset -y -r 0 0x77 6 0xFF")
        os.system("i2cset -y -r 0 0x77 7 0xFF")

        # init SMBUS1 ABS
        os.system("i2cset -y -r 5 0x20 4 0x00")
        os.system("i2cset -y -r 5 0x20 5 0x00")
        os.system("i2cset -y -r 5 0x20 6 0xFF")
        os.system("i2cset -y -r 5 0x20 7 0xFF")

        os.system("i2cset -y -r 5 0x21 4 0x00")
        os.system("i2cset -y -r 5 0x21 5 0x00")
        os.system("i2cset -y -r 5 0x21 6 0xFF")
        os.system("i2cset -y -r 5 0x21 7 0xFF")

        os.system("i2cset -y -r 5 0x22 4 0x00")
        os.system("i2cset -y -r 5 0x22 5 0x00")
        os.system("i2cset -y -r 5 0x22 6 0xFF")
        os.system("i2cset -y -r 5 0x22 7 0xFF")

        os.system("i2cset -y -r 5 0x23 4 0x00")
        os.system("i2cset -y -r 5 0x23 5 0x00")
        os.system("i2cset -y -r 5 0x23 2 0xCF")
        os.system("i2cset -y -r 5 0x23 3 0xF0")
        os.system("i2cset -y -r 5 0x23 6 0xCF")
        os.system("i2cset -y -r 5 0x23 7 0xF0")

        # init SFP
        os.system("i2cset -y -r 5 0x27 4 0x00")
        os.system("i2cset -y -r 5 0x27 5 0x00")
        os.system("i2cset -y -r 5 0x27 2 0x00")
        os.system("i2cset -y -r 5 0x27 3 0x00")
        os.system("i2cset -y -r 5 0x27 6 0xCF")
        os.system("i2cset -y -r 5 0x27 7 0xF0")

        # set ZQSFP LP_MODE = 0
        os.system("i2cset -y -r 6 0x20 4 0x00")
        os.system("i2cset -y -r 6 0x20 5 0x00")
        os.system("i2cset -y -r 6 0x20 2 0x00")
        os.system("i2cset -y -r 6 0x20 3 0x00")
        os.system("i2cset -y -r 6 0x20 6 0x00")
        os.system("i2cset -y -r 6 0x20 7 0x00")

        os.system("i2cset -y -r 6 0x21 4 0x00")
        os.system("i2cset -y -r 6 0x21 5 0x00")
        os.system("i2cset -y -r 6 0x21 2 0x00")
        os.system("i2cset -y -r 6 0x21 3 0x00")
        os.system("i2cset -y -r 6 0x21 6 0x00")
        os.system("i2cset -y -r 6 0x21 7 0x00")

        # set ZQSFP RST = 1
        os.system("i2cset -y -r 6 0x22 4 0x00")
        os.system("i2cset -y -r 6 0x22 5 0x00")
        os.system("i2cset -y -r 6 0x22 2 0xFF")
        os.system("i2cset -y -r 6 0x22 3 0xFF")
        os.system("i2cset -y -r 6 0x22 6 0x00")
        os.system("i2cset -y -r 6 0x22 7 0x00")

        os.system("i2cset -y -r 6 0x23 4 0x00")
        os.system("i2cset -y -r 6 0x23 5 0x00")
        os.system("i2cset -y -r 6 0x23 2 0xFF")
        os.system("i2cset -y -r 6 0x23 3 0xFF")
        os.system("i2cset -y -r 6 0x23 6 0x00")
        os.system("i2cset -y -r 6 0x23 7 0x00")

        # init Host GPIO
        os.system("i2cset -y -r 0 0x74 4 0x00")
        os.system("i2cset -y -r 0 0x74 5 0x00")
        os.system("i2cset -y -r 0 0x74 2 0x0F")
        os.system("i2cset -y -r 0 0x74 3 0xDF")
        os.system("i2cset -y -r 0 0x74 6 0x08")
        os.system("i2cset -y -r 0 0x74 7 0x1F")

        # init Board ID
        os.system("i2cset -y -r 51 0x27 4 0x00")
        os.system("i2cset -y -r 51 0x27 5 0x00")
        os.system("i2cset -y -r 51 0x27 6 0xFF")
        os.system("i2cset -y -r 51 0x27 7 0xFF")

        # init Board ID of Dummy BMC Board
        os.system("i2cset -y -r 0 0x24 4 0x00")
        os.system("i2cset -y -r 0 0x24 5 0x00")
        os.system("i2cset -y -r 0 0x24 6 0xFF")
        os.system("i2cset -y -r 0 0x24 7 0xFF")

        # init PSU I/O (BAREFOOT_IO_EXP_PSU_ID)
        os.system("i2cset -y -r 0 0x25 4 0x00")
        os.system("i2cset -y -r 0 0x25 5 0x00")
        os.system("i2cset -y -r 0 0x25 2 0x00")
        os.system("i2cset -y -r 0 0x25 3 0x1D")
        os.system("i2cset -y -r 0 0x25 6 0xDB")
        os.system("i2cset -y -r 0 0x25 7 0x03")

        # init FAN I/O (BAREFOOT_IO_EXP_FAN_ID)
        os.system("i2cset -y -r 59 0x20 4 0x00")
        os.system("i2cset -y -r 59 0x20 5 0x00")
        os.system("i2cset -y -r 59 0x20 2 0x11")
        os.system("i2cset -y -r 59 0x20 3 0x11")
        os.system("i2cset -y -r 59 0x20 6 0xCC")
        os.system("i2cset -y -r 59 0x20 7 0xCC")

        # init Fan
        # select bank 0
        os.system("i2cset -y -r 56 0x2F 0x00 0x80")
        # enable FANIN 1-8
        os.system("i2cset -y -r 56 0x2F 0x06 0xFF")
        # disable FANIN 9-14
        os.system("i2cset -y -r 56 0x2F 0x07 0x00")
        # select bank 2
        os.system("i2cset -y -r 56 0x2F 0x00 0x82")
        # set PWM mode in FOMC
        os.system("i2cset -y -r 56 0x2F 0x0F 0x00")

        # init VOLMON
        os.system("i2cset -y -r 56 0x2F 0x00 0x80")
        os.system("i2cset -y -r 56 0x2F 0x01 0x1C")
        os.system("i2cset -y -r 56 0x2F 0x00 0x80")
        os.system("i2cset -y -r 56 0x2F 0x02 0xFF")
        os.system("i2cset -y -r 56 0x2F 0x03 0x50")
        os.system("i2cset -y -r 56 0x2F 0x04 0x0A")
        os.system("i2cset -y -r 56 0x2F 0x00 0x80")
        os.system("i2cset -y -r 56 0x2F 0x01 0x1D")
        self.new_i2c_device('w83795adg', 0x2F, 56)

        # init Fan Speed
        os.system("echo 120 > /sys/class/hwmon/hwmon1/device/pwm1")
        os.system("echo 120 > /sys/class/hwmon/hwmon1/device/pwm2")

        # init Temperature
        self.new_i2c_devices(
            [
                # ASIC Coretemp and Front MAC
                ('lm86', 0x4C, 41),

                # CPU Board
                ('tmp75', 0x4F, 0),

                # Near PSU1
                ('tmp75', 0x48, 41),

                # Rear MAC
                ('tmp75', 0x4A, 41),

                # Near Port 32
                ('tmp75', 0x4B, 41),

                # Near PSU2
                ('tmp75', 0x4D, 41),
            ]
        )

        # init GPIO, ABS Port 0-15
        self.new_i2c_device('pca9535', 0x20, 5)
        for i in range(496, 512):
            os.system("echo %d > /sys/class/gpio/export" % i)
            os.system("echo 1 > /sys/class/gpio/gpio%d/active_low" % i)

        # init GPIO, ABS Port 16-31
        self.new_i2c_device('pca9535', 0x21, 5)
        for i in range(480, 496):
            os.system("echo %d > /sys/class/gpio/export" % i)
            os.system("echo 1 > /sys/class/gpio/gpio%d/active_low" % i)

        # init GPIO, INT Port 0-15
        self.new_i2c_device('pca9535', 0x22, 5)
        for i in range(464, 480):
            os.system("echo %d > /sys/class/gpio/export" % i)
            os.system("echo 1 > /sys/class/gpio/gpio%d/active_low" % i)

        # init GPIO, INT Port 16-31
        self.new_i2c_device('pca9535', 0x23, 5)
        for i in range(448, 464):
            os.system("echo %d > /sys/class/gpio/export" % i)
            os.system("echo 1 > /sys/class/gpio/gpio%d/active_low" % i)

        # init GPIO, SFP
        self.new_i2c_device('pca9535', 0x27, 5)
        for i in range(432, 448):
            os.system("echo %d > /sys/class/gpio/export" % i)
            if i == 180 or i == 181 or i == 184 or \
               i == 185 or i == 186 or i == 187:
                os.system("echo out > /sys/class/gpio/gpio%d/direction" % i)
            else:
                os.system("echo 1 > /sys/class/gpio/gpio%d/active_low" % i)

        # init GPIO, LP Mode Port 0-15
        self.new_i2c_device('pca9535', 0x20, 6)
        for i in range(416, 432):
            os.system("echo %d > /sys/class/gpio/export" % i)
            os.system("echo out > /sys/class/gpio/gpio%d/direction" % i)

        # init GPIO, LP Mode Port 16-31
        self.new_i2c_device('pca9535', 0x21, 6)
        for i in range(400, 416):
            os.system("echo %d > /sys/class/gpio/export" % i)
            os.system("echo out > /sys/class/gpio/gpio%d/direction" % i)

        # init GPIO, RST Port 0-15
        self.new_i2c_device('pca9535', 0x22, 6)
        for i in range(384, 400):
            os.system("echo %d > /sys/class/gpio/export" % i)
            os.system("echo out > /sys/class/gpio/gpio%d/direction" % i)
            os.system("echo 1 > /sys/class/gpio/gpio%d/active_low" % i)
            os.system("echo 0 > /sys/class/gpio/gpio%d/value" % i)

        # init GPIO, RST Port 16-31
        self.new_i2c_device('pca9535', 0x23, 6)
        for i in range(368, 384):
            os.system("echo %d > /sys/class/gpio/export" % i)
            os.system("echo out > /sys/class/gpio/gpio%d/direction" % i)
            os.system("echo 1 > /sys/class/gpio/gpio%d/active_low" % i)
            os.system("echo 0 > /sys/class/gpio/gpio%d/value" % i)

        # init QSFP EEPROM
        for port in range(1, 33):
            self.new_i2c_device('optoe1', 0x50, port + 8)

        subprocess.call(
            'echo port2 > /sys/bus/i2c/devices/9-0050/port_name', shell=True)
        subprocess.call(
            'echo port1 > /sys/bus/i2c/devices/10-0050/port_name', shell=True)
        subprocess.call(
            'echo port4 > /sys/bus/i2c/devices/11-0050/port_name', shell=True)
        subprocess.call(
            'echo port3 > /sys/bus/i2c/devices/12-0050/port_name', shell=True)
        subprocess.call(
            'echo port6 > /sys/bus/i2c/devices/13-0050/port_name', shell=True)
        subprocess.call(
            'echo port5 > /sys/bus/i2c/devices/14-0050/port_name', shell=True)
        subprocess.call(
            'echo port8 > /sys/bus/i2c/devices/15-0050/port_name', shell=True)
        subprocess.call(
            'echo port7 > /sys/bus/i2c/devices/16-0050/port_name', shell=True)
        subprocess.call(
            'echo port10 > /sys/bus/i2c/devices/17-0050/port_name', shell=True)
        subprocess.call(
            'echo port9 > /sys/bus/i2c/devices/18-0050/port_name', shell=True)
        subprocess.call(
            'echo port12 > /sys/bus/i2c/devices/19-0050/port_name', shell=True)
        subprocess.call(
            'echo port11 > /sys/bus/i2c/devices/20-0050/port_name', shell=True)
        subprocess.call(
            'echo port14 > /sys/bus/i2c/devices/21-0050/port_name', shell=True)
        subprocess.call(
            'echo port13 > /sys/bus/i2c/devices/22-0050/port_name', shell=True)
        subprocess.call(
            'echo port16 > /sys/bus/i2c/devices/23-0050/port_name', shell=True)
        subprocess.call(
            'echo port15 > /sys/bus/i2c/devices/24-0050/port_name', shell=True)
        subprocess.call(
            'echo port18 > /sys/bus/i2c/devices/25-0050/port_name', shell=True)
        subprocess.call(
            'echo port17 > /sys/bus/i2c/devices/26-0050/port_name', shell=True)
        subprocess.call(
            'echo port20 > /sys/bus/i2c/devices/27-0050/port_name', shell=True)
        subprocess.call(
            'echo port19 > /sys/bus/i2c/devices/28-0050/port_name', shell=True)
        subprocess.call(
            'echo port22 > /sys/bus/i2c/devices/29-0050/port_name', shell=True)
        subprocess.call(
            'echo port21 > /sys/bus/i2c/devices/30-0050/port_name', shell=True)
        subprocess.call(
            'echo port24 > /sys/bus/i2c/devices/31-0050/port_name', shell=True)
        subprocess.call(
            'echo port23 > /sys/bus/i2c/devices/32-0050/port_name', shell=True)
        subprocess.call(
            'echo port26 > /sys/bus/i2c/devices/33-0050/port_name', shell=True)
        subprocess.call(
            'echo port25 > /sys/bus/i2c/devices/34-0050/port_name', shell=True)
        subprocess.call(
            'echo port28 > /sys/bus/i2c/devices/35-0050/port_name', shell=True)
        subprocess.call(
            'echo port27 > /sys/bus/i2c/devices/36-0050/port_name', shell=True)
        subprocess.call(
            'echo port30 > /sys/bus/i2c/devices/37-0050/port_name', shell=True)
        subprocess.call(
            'echo port29 > /sys/bus/i2c/devices/38-0050/port_name', shell=True)
        subprocess.call(
            'echo port32 > /sys/bus/i2c/devices/39-0050/port_name', shell=True)
        subprocess.call(
            'echo port31 > /sys/bus/i2c/devices/40-0050/port_name', shell=True)

        # init SFP(0/1) EEPROM
        self.new_i2c_device('optoe1', 0x50, 45)
        subprocess.call(
            'echo e1-33 > /sys/bus/i2c/devices/45-0050/port_name', shell=True)
        self.new_i2c_device('optoe1', 0x50, 46)
        subprocess.call(
            'echo e2-34 > /sys/bus/i2c/devices/46-0050/port_name', shell=True)

        # init PSU(0/1) EEPROM devices
        self.new_i2c_device('eeprom', 0x50, 57)
        self.new_i2c_device('eeprom', 0x50, 58)

        # _mac_vdd_init
        reg_val_str = subprocess.check_output(
            """i2cget -y 44 0x33 0x42 2>/dev/null""", shell=True)
        reg_val = int(reg_val_str, 16)
        vid = reg_val & 0x7
        mac_vdd_val = vdd_val_array[vid]
        rov_reg = rov_reg_array[vid]

        msg("Setting mac vdd %1.2f with rov register value 0x%x\n" %
            (mac_vdd_val, rov_reg))
        os.system("i2cset -y -r 55 0x22 0x21 0x%x w" % rov_reg)

        # init SYS LED
        os.system("i2cset -y -r 50 0x75 2 0x01")
        os.system("i2cset -y -r 50 0x75 4 0x00")
        os.system("i2cset -y -r 50 0x75 5 0x00")
        os.system("i2cset -y -r 50 0x75 6 0x00")
        os.system("i2cset -y -r 50 0x75 7 0x00")

        return True
