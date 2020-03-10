from onl.platform.base import *
from onl.platform.accton import *
import commands

#IR3570A chip casue problem when read eeprom by i2c-block mode.
#It happen when read 16th-byte offset that value is 0x8. So disable chip 
def disable_i2c_ir3570a(addr):
    cmd = "i2cset -y 0 0x%x 0xE5 0x01" % addr
    status, output = commands.getstatusoutput(cmd)
    cmd = "i2cset -y 0 0x%x 0x12 0x02" % addr
    status, output = commands.getstatusoutput(cmd)
    return status

def ir3570_check():
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

class OnlPlatform_x86_64_accton_asxvolt16_r0(OnlPlatformAccton,
                                             OnlPlatformPortConfig_20x100):
    PLATFORM='x86-64-accton-asxvolt16-r0'
    MODEL="ASXvOLT16"
    SYS_OBJECT_ID=".816.1"

    def baseconfig(self):
        self.insmod('ym2651y')
        for m in [ 'cpld', 'fan', 'psu', 'leds', 'sfp' ]:
            self.insmod("x86-64-accton-asxvolt16-%s.ko" % m)

        ########### initialize I2C bus 0 ###########
        self.new_i2c_devices([
                # initialize root multiplexer (PCA9548)
                ('pca9548', 0x77, 0),

                # initiate leaf multiplexer (PCA9548)
                ('pca9548', 0x76, 1),
                ('pca9548', 0x74, 2),
                ('pca9548', 0x72, 2),
                ('pca9548', 0x75, 25),
                ('pca9548', 0x75, 26),
                ('pca9548', 0x75, 27),

                # initiate chassis fan
                ('asxvolt16_fan', 0x66, 9),

                # inititate LM75
                ('lm75', 0x49, 10),
                ('lm75', 0x4a, 10),
                ('lm75', 0x4b, 10),
                ('lm75', 0x4e, 10),

                #initiate CPLD and FPGA
                ('asxvolt16_fpga', 0x60, 11),
                ('asxvolt16_cpld', 0x62, 12),

                # initiate PSU-1
                ('asxvolt16_psu1', 0x53, 18),
                ('ym2651', 0x5b, 18),

                # initiate PSU-2
                ('asxvolt16_psu2', 0x50, 17),
                ('ym2651', 0x58, 17),

                # initiate XFP/QSFP
                ('asxvolt16_sfp7', 0x50, 33),
                ('asxvolt16_sfp8', 0x50, 34),
                ('asxvolt16_sfp5', 0x50, 35),
                ('asxvolt16_sfp6', 0x50, 36),
                ('asxvolt16_sfp3', 0x50, 37),
                ('asxvolt16_sfp4', 0x50, 38),
                ('asxvolt16_sfp9', 0x50, 39),
                ('asxvolt16_sfp10', 0x50, 40),

                ('asxvolt16_sfp11', 0x50, 41),
                ('asxvolt16_sfp12', 0x50, 42),
                ('asxvolt16_sfp13', 0x50, 43),
                ('asxvolt16_sfp14', 0x50, 44),
                ('asxvolt16_sfp15', 0x50, 45),
                ('asxvolt16_sfp16', 0x50, 46),
                ('asxvolt16_sfp1', 0x50, 47),
                ('asxvolt16_sfp2', 0x50, 48),

                ('asxvolt16_sfp17', 0x50, 49),
                ('asxvolt16_sfp18', 0x50, 50),
                ('asxvolt16_sfp19', 0x50, 51),
                ('asxvolt16_sfp20', 0x50, 52),

                # initiate IDPROM
                ('24c02', 0x56, 0),
                ])

        ir3570_check()

        return True
