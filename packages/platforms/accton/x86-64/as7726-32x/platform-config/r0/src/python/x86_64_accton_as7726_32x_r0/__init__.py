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


class OnlPlatform_x86_64_accton_as7726_32x_r0(OnlPlatformAccton,
                                              OnlPlatformPortConfig_48x25_6x100):

    PLATFORM='x86-64-accton-as7726-32x-r0'
    MODEL="AS7726-32X"
    SYS_OBJECT_ID=".7726.32"

    def baseconfig(self):
        self.insmod('optoe')
        self.insmod('ym2651y')
        for m in [ 'cpld', 'fan', 'psu', 'leds' ]:
            self.insmod("x86-64-accton-as7726-32x-%s.ko" % m)

        ########### initialize I2C bus 0 ###########
        # initialize multiplexer (PCA9548)        
        self.new_i2c_device('pca9548', 0x77, 0)
        # initiate multiplexer (PCA9548)
        self.new_i2c_devices(
            [
                # initiate multiplexer (PCA9548)
                ('pca9548', 0x76, 1),
                ('pca9548', 0x72, 1),
                ('pca9548', 0x73, 1),
                ('pca9548', 0x74, 1),
                ('pca9548', 0x75, 1),
                ('pca9548', 0x71, 2)
                ]
            )
        
        self.new_i2c_devices([
            # initialize CPLD
            ('as7726_32x_cpld1', 0x60, 11),
            ('as7726_32x_cpld2', 0x62, 12),
            ('as7726_32x_cpld3', 0x64, 13),
            ])
        self.new_i2c_devices([
            # initiate fan
            ('as7726_32x_fan', 0x66, 54),
            ('lm75', 0x4c, 54),

            # inititate LM75
            ('lm75', 0x48, 55),
            ('lm75', 0x49, 55),
            ('lm75', 0x4a, 55),
            ('lm75', 0x4b, 55),
         ])

        self.new_i2c_devices([
            # initiate PSU-1
            ('as7726_32x_psu1', 0x53, 50),
            ('ym2651', 0x5B, 50),

            # initiate PSU-2
            ('as7726_32x_psu2', 0x50, 49),
            ('ym2651', 0x58, 49),
         ])

        # initialize QSFP port 1~8
        for port in range(1, 5):
            self.new_i2c_device('optoe1', 0x50, port+20)
            subprocess.call('echo port%d > /sys/bus/i2c/devices/%d-0050/port_name' % (port, port+20), shell=True)
            
        self.new_i2c_device('optoe1', 0x50, 26) 
        subprocess.call('echo port%d > /sys/bus/i2c/devices/%d-0050/port_name' % (5, 26), shell=True)
        self.new_i2c_device('optoe1', 0x50, 25) 
        subprocess.call('echo port%d > /sys/bus/i2c/devices/%d-0050/port_name' % (6, 25), shell=True)
        self.new_i2c_device('optoe1', 0x50, 28) 
        subprocess.call('echo port%d > /sys/bus/i2c/devices/%d-0050/port_name' % (7, 28), shell=True)
        self.new_i2c_device('optoe1', 0x50, 27) 
        subprocess.call('echo port%d > /sys/bus/i2c/devices/%d-0050/port_name' % (8, 27), shell=True)
        
        # initialize QSFP port 9~16    
        for port in range(9, 13):
            self.new_i2c_device('optoe1', 0x50, port+8)
            subprocess.call('echo port%d > /sys/bus/i2c/devices/%d-0050/port_name' % (port, port+8), shell=True)
        for port in range(13, 17):
            self.new_i2c_device('optoe1', 0x50, port+16)
            subprocess.call('echo port%d > /sys/bus/i2c/devices/%d-0050/port_name' % (port, port+16), shell=True)

        # initialize QSFP port 17~24
        for port in range(17, 21):
            self.new_i2c_device('optoe1', 0x50, port+16)
            subprocess.call('echo port%d > /sys/bus/i2c/devices/%d-0050/port_name' % (port, port+16), shell=True)
        for port in range(21, 25):
            self.new_i2c_device('optoe1', 0x50, port+24)
            subprocess.call('echo port%d > /sys/bus/i2c/devices/%d-0050/port_name' % (port, port+24), shell=True)

        # initialize QSFP port 25~32
        for port in range(25, 33):
            self.new_i2c_device('optoe1', 0x50, port+12)
            subprocess.call('echo port%d > /sys/bus/i2c/devices/%d-0050/port_name' % (port, port+12), shell=True)
        
        # initialize SFP port 33~34
        for port in range(33, 35):
            self.new_i2c_device('optoe1', 0x50, port-18)
            subprocess.call('echo port%d > /sys/bus/i2c/devices/%d-0050/port_name' % (port, port-18), shell=True)

        self.new_i2c_device('24c02', 0x56, 0)

        ir3570_check() 

        return True
