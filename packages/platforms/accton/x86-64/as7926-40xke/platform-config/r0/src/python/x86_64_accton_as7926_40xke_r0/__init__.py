from onl.platform.base import *
from onl.platform.accton import *

import commands

#IR3570A chip casue problem when read eeprom by i2c-block mode.
#It happen when read 16th-byte offset that value is 0x8. So disable chip
def disable_i2c_ir3570a(addr):
    check_i2c="i2cget -y 0 0x4 0x1"
    status, output = commands.getstatusoutput(check_i2c)
    if status!=0:
        return -1
    cmd = "i2cset -y 0 0x%x 0xE5 0x01" % addr
    status, output = commands.getstatusoutput(cmd)
    cmd = "i2cset -y 0 0x%x 0x12 0x02" % addr
    status, output = commands.getstatusoutput(cmd)
    return status

def ir3570_check():
    check_i2c="i2cget -y 0 0x42 0x1"
    status, output = commands.getstatusoutput(check_i2c)
    if status!=0:
        return -1
    cmd = "i2cdump -y 0 0x42 s 0x9a"
    try:
        status, output = commands.getstatusoutput(cmd)
        lines = output.split('\n')
        hn = re.findall(r'\w+', lines[-1])
        version = int(hn[1], 16)
        if version == 0x24:  #Find IR3570A
            ret = disable_i2c_ir3570a(4)
        else:
            ret = 0
    except Exception as e:
        #print "Error on ir3570_check() e:" + str(e)
        return -1
    return ret

class OnlPlatform_x86_64_accton_as7926_40xke_r0(OnlPlatformAccton,
                                              OnlPlatformPortConfig_80x100):
    PLATFORM='x86-64-accton-as7926-40xke-r0'
    MODEL="AS7926-40xke"
    SYS_OBJECT_ID=".7926.40"

    def baseconfig(self):
        self.insmod('optoe')
        self.insmod('ym2651y')
        for m in [ 'cpld', 'fan', 'psu', 'leds']:
            self.insmod("x86-64-accton-as7926-40xke-%s.ko" % m)

        ########### initialize I2C bus 0 ###########
        self.new_i2c_devices([
                # initialize multiplexer (PCA9548)
                ('pca9548', 0x77, 0),

                # initiate  multiplexer (PCA9548) of bottom board
                ('pca9548', 0x76, 1),
                ('pca9548', 0x72, 2),
                ('pca9548', 0x73, 9),

                 #HW SPEC:6.3. PDU BOARD I2C ADDRESS TABLE
                 # initiate PSU-1
                ('as7926_40xke_psu1', 0x52, 18),
                ('ym2401', 0x5a, 18),

                # initiate PSU-2
                ('as7926_40xke_psu2', 0x53, 18),
                ('ym2401', 0x5b, 18),

                # initiate PSU-3
                ('as7926_40xke_psu3', 0x51, 18),
                ('ym2401', 0x59, 18),

                # initiate fan
                ('as7926_40xke_fan', 0x66, 22),

                # initiate bottom board multiplexer (PCA9548)
                ('pca9548', 0x74, 25),
                ('pca9548', 0x74, 26),
                ('pca9548', 0x74, 27),
                ('pca9548', 0x74, 28),
                ('pca9548', 0x74, 29),
                ('24c02', 0x57, 0),

                # initiate leaf multiplexer (PCA9548) of top board
                ('pca9548', 0x70, 1),
                ('pca9548', 0x74, 73),
                ('pca9548', 0x74, 74),
                ('pca9548', 0x74, 75),

                #initiate CPLD
                ('as7926_40xke_cpld1', 0x60, 11),
                ('as7926_40xke_cpld2', 0x62, 12),
                ('as7926_40xke_cpld3', 0x63, 13),
                ('as7926_40xke_cpld4', 0x64, 76),
                ('as7926_40xke_cpld5', 0x70, 20),
                ('as7926_40xke_cpld6', 0x73, 20),
                ])

        # Bring QSFP out of reset
        for i in range(1, 21):
            subprocess.call("echo 0 > /sys/bus/i2c/devices/12-0062/module_reset_"+str(i), shell=True)

        for i in range(21, 41):
            subprocess.call("echo 0 > /sys/bus/i2c/devices/13-0063/module_reset_"+str(i), shell=True)

        # initialize QSFP port 1-40 of bottom board
        for port in range(1, 41):
            self.new_i2c_device('optoe1', 0x50, port+32)
            subprocess.call('echo port%d > /sys/bus/i2c/devices/%d-0050/port_name' % (port, port+32), shell=True)

        # initialize SFP port 41-42 of bottom board
        self.new_i2c_device('optoe2', 0x50, 30)
        subprocess.call('echo port41 > /sys/bus/i2c/devices/30-0050/port_name', shell=True)
        self.new_i2c_device('optoe2', 0x50, 31)
        subprocess.call('echo port42 > /sys/bus/i2c/devices/31-0050/port_name', shell=True)

        # initialize  port 43-55 of QSFP-DD Board
        for port in range(43, 56):
            self.new_i2c_device('optoe1', 0x50, port+38)

        subprocess.call('echo port47 > /sys/bus/i2c/devices/81-0050/port_name', shell=True)
        subprocess.call('echo port46 > /sys/bus/i2c/devices/82-0050/port_name', shell=True)
        subprocess.call('echo port45 > /sys/bus/i2c/devices/83-0050/port_name', shell=True)
        subprocess.call('echo port44 > /sys/bus/i2c/devices/84-0050/port_name', shell=True)
        subprocess.call('echo port49 > /sys/bus/i2c/devices/85-0050/port_name', shell=True)
        subprocess.call('echo port48 > /sys/bus/i2c/devices/86-0050/port_name', shell=True)
        subprocess.call('echo port51 > /sys/bus/i2c/devices/87-0050/port_name', shell=True)
        subprocess.call('echo port50 > /sys/bus/i2c/devices/88-0050/port_name', shell=True)
        subprocess.call('echo port53 > /sys/bus/i2c/devices/89-0050/port_name', shell=True)
        subprocess.call('echo port52 > /sys/bus/i2c/devices/90-0050/port_name', shell=True)
        subprocess.call('echo port55 > /sys/bus/i2c/devices/91-0050/port_name', shell=True)
        subprocess.call('echo port54 > /sys/bus/i2c/devices/92-0050/port_name', shell=True)
        subprocess.call('echo port43 > /sys/bus/i2c/devices/93-0050/port_name', shell=True)

        ir3570_check()

        return True
