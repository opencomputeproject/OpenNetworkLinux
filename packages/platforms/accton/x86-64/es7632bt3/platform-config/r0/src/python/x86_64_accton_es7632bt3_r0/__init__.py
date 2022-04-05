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
        print "Error on ir3570_check() e:" + str(e)
        return -1
    return ret

def _8v89307_init():
    script = os.path.join(os.path.dirname(os.path.realpath(__file__)), "8v89307_init.sh")
    if os.path.exists(script):
        status, output = commands.getstatusoutput(script)
        print output
        if status != 0:
            print "Error in 8v89307_init: " + str(e)
            return False
    return True

class OnlPlatform_x86_64_accton_es7632bt3_r0(OnlPlatformAccton,
                                              OnlPlatformPortConfig_32x100):

    PLATFORM='x86-64-accton-es7632bt3-r0'
    MODEL="IXR7220-D3L"
    SYS_OBJECT_ID=".7220.D3"

    def baseconfig(self):
        self.insmod('optoe')
        self.insmod('ym2651y')
        os.system('insmod /lib/modules/4.19.81-OpenNetworkLinux/kernel/drivers/i2c/busses/i2c-ismt.ko')

        for m in [ 'cpld', 'fan', 'psu', 'leds' ]:
            self.insmod("x86-64-accton-es7632bt3-%s.ko" % m)

        ########### initialize I2C bus 0 ###########
        # initialize multiplexer (PCA9548)
        self.new_i2c_device('pca9548', 0x77, 1)
        # initiate multiplexer (PCA9548)
        self.new_i2c_devices(
            [
                # initiate multiplexer (PCA9548)
                ('pca9548', 0x76, 2),
                ('pca9548', 0x72, 2),
                ('pca9548', 0x73, 2),
                ('pca9548', 0x74, 2),
                ('pca9548', 0x75, 2),
                ('pca9548', 0x71, 3)
                ]
            )

        self.new_i2c_devices([
            # initialize CPLD
            ('es7632bt3_cpld1', 0x60, 12),
            ('es7632bt3_cpld2', 0x62, 13),
            ('es7632bt3_cpld3', 0x64, 14),
            ])
        self.new_i2c_devices([
            # initiate fan
            ('es7632bt3_fan', 0x66, 55),
            ('lm75', 0x4c, 55),

            # inititate LM75
            ('lm75', 0x48, 56),
            ('lm75', 0x49, 56),
            ('lm75', 0x4a, 56),
            ('lm75', 0x4b, 56),
         ])

        self.new_i2c_devices([
            # initiate PSU-1
            ('es7632bt3_psu1', 0x53, 51),
            ('ym2651', 0x5B, 51),

            # initiate PSU-2
            ('es7632bt3_psu2', 0x50, 50),
            ('ym2651', 0x58, 50),
         ])

        # initialize QSFP port 1~8
        for port in range(1, 5):
            self.new_i2c_device('optoe1', 0x50, port+21)
            subprocess.call('echo port%d > /sys/bus/i2c/devices/%d-0050/port_name' % (port, port+21), shell=True)

        self.new_i2c_device('optoe1', 0x50, 27)
        subprocess.call('echo port%d > /sys/bus/i2c/devices/%d-0050/port_name' % (5, 27), shell=True)
        self.new_i2c_device('optoe1', 0x50, 26)
        subprocess.call('echo port%d > /sys/bus/i2c/devices/%d-0050/port_name' % (6, 26), shell=True)
        self.new_i2c_device('optoe1', 0x50, 29)
        subprocess.call('echo port%d > /sys/bus/i2c/devices/%d-0050/port_name' % (7, 29), shell=True)
        self.new_i2c_device('optoe1', 0x50, 28)
        subprocess.call('echo port%d > /sys/bus/i2c/devices/%d-0050/port_name' % (8, 28), shell=True)

        # initialize QSFP port 9~16
        for port in range(9, 13):
            self.new_i2c_device('optoe1', 0x50, port+9)
            subprocess.call('echo port%d > /sys/bus/i2c/devices/%d-0050/port_name' % (port, port+9), shell=True)
        for port in range(13, 17):
            self.new_i2c_device('optoe1', 0x50, port+17)
            subprocess.call('echo port%d > /sys/bus/i2c/devices/%d-0050/port_name' % (port, port+17), shell=True)

        # initialize QSFP port 17~24
        for port in range(17, 21):
            self.new_i2c_device('optoe1', 0x50, port+17)
            subprocess.call('echo port%d > /sys/bus/i2c/devices/%d-0050/port_name' % (port, port+17), shell=True)
        for port in range(21, 25):
            self.new_i2c_device('optoe1', 0x50, port+25)
            subprocess.call('echo port%d > /sys/bus/i2c/devices/%d-0050/port_name' % (port, port+25), shell=True)

        # initialize QSFP port 25~32
        for port in range(25, 33):
            self.new_i2c_device('optoe1', 0x50, port+13)
            subprocess.call('echo port%d > /sys/bus/i2c/devices/%d-0050/port_name' % (port, port+13), shell=True)

        # initialize SFP port 33~34
        for port in range(33, 35):
            self.new_i2c_device('optoe2', 0x50, port-17)
            subprocess.call('echo port%d > /sys/bus/i2c/devices/%d-0050/port_name' % (port, port-17), shell=True)

        self.new_i2c_device('24c02', 0x57, 1)

        ir3570_check()
        _8v89307_init()

        return True
