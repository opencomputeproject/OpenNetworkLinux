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

class OnlPlatform_x86_64_accton_as7936_22xke_r0(OnlPlatformAccton,
                                    OnlPlatformPortConfig_10x400_18x100_2x10):
    PLATFORM='x86-64-accton-as7936-22xke-r0'
    MODEL="AS7936-22xke"
    SYS_OBJECT_ID=".7936.22"

    def baseconfig(self):
        self.insmod('optoe')
        self.insmod('ym2651y')
        self.insmod_platform()  #insmod *.ko of platform dir under /lib/module/*/extra

        ########### initialize I2C bus 0 ###########
        self.new_i2c_devices([
                # initialize root multiplexer (PCA9548)
                ('pca9548', 0x77, 0),

                # initiate  multiplexer (PCA9548) of bottom board
                ('pca9548', 0x76, 1),
                ('pca9548', 0x72, 2),
                ('pca9548', 0x73, 9),
                
                # initiate transcivers' multiplexer (PCA9548)
                ('pca9548', 0x74, 25),
                ('pca9548', 0x74, 26),
                ('pca9548', 0x74, 27),
                ('pca9548', 0x74, 28),

                #system eeprom 
                ('24c02', 0x57, 0),

                #initiate CPLD
                ('as7936_22xke_cpld1', 0x60, 11),
                ('as7936_22xke_cpld2', 0x61, 12),
                ('as7936_22xke_cpld3', 0x62, 13),

                # initiate PSU-1&2
                ('as7936_22xke_psu1', 0x51, 18),
                ('ym2651', 0x59, 18),
                ('as7936_22xke_psu2', 0x53, 18),
                ('ym2651', 0x5b, 18),

                # initiate fan
                ('as7936_22xke_fan', 0x68, 22),
                ('tmp75', 0x4d, 22),
                ('tmp75', 0x4e, 22),

                ])
        
        ########### For transcievers' eeprom ###########
        fmt_str = 'echo port%d > /sys/bus/i2c/devices/%d-0050/port_name'
        # initialize 6 QSFP-4DD,the port 1-6
        bstart = 32
        for port in range(1, 7):
            bstart += 1
            self.new_i2c_device('optoe1', 0x50, bstart)            
            subprocess.call(fmt_str % (port, bstart), shell=True)


        # initialize 4 QSFP-4DD,the port 7-10 and 2 QSFP-16Q, the port 11~12.
        bstart = 40 
        for port in range(7, 13):
            bstart += 1
            self.new_i2c_device('optoe1', 0x50, bstart)            
            subprocess.call(fmt_str % (port, bstart), shell=True)
        
        # initialize 16 QSFP-16Q, the port 13~28.
        bstart = 48 
        for port in range(13, 29):
            bstart += 1
            self.new_i2c_device('optoe1', 0x50, bstart)            
            subprocess.call(fmt_str % (port, bstart), shell=True)

        # initialize OOB SFP+ port 29-30, reversed.
        self.new_i2c_device('optoe2', 0x50, 31)
        subprocess.call('echo port29 > /sys/bus/i2c/devices/30-0050/port_name', shell=True)
        self.new_i2c_device('optoe2', 0x50, 30)
        subprocess.call('echo port30 > /sys/bus/i2c/devices/31-0050/port_name', shell=True)

      
        ir3570_check() 
                
        return True
