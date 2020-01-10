from onl.platform.base import *
from onl.platform.accton import *

import commands

def eeprom_check():
    cmd = "i2cget -y 0 0x56"
    status, output = commands.getstatusoutput(cmd)
    return status

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
                ])

        self.new_i2c_devices([
                # initialize multiplexer (PCA9548)
                ('pca9548', 0x76, 1),
                
                # initiate chassis fan
                ('as5916_54xm_fan', 0x66, 9),

                # inititate LM75
                ('lm75', 0x48, 10),  #For R0A
                ('lm75', 0x4c, 10),  #For R0B or later
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

        ir3570_check()

        #Dut to new board eeprom i2c-addr is 0x57, old board eeprom i2c-addr is 0x56. So need to check and set correct i2c-addr sysfs
        ret=eeprom_check()
        if ret==0:
            self.new_i2c_device('24c02', 0x56, 0)
        else:
            ir3570_check()
            self.new_i2c_device('24c02', 0x57, 0)

        return True

