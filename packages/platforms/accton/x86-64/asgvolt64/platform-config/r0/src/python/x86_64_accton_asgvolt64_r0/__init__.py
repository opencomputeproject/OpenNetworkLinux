from onl.platform.base import *
from onl.platform.accton import *

import commands

def fpga_pcie_init():
    cmd= "setpci -s 16:00.0 0x04.B=0x7"
    status, output = commands.getstatusoutput(cmd)
    return status

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


class OnlPlatform_x86_64_accton_asgvolt64_r0(OnlPlatformAccton,
                                             OnlPlatformPortConfig_20x100):
    PLATFORM='x86-64-accton-asgvolt64-r0'
    MODEL="ASGVOLT64"
    SYS_OBJECT_ID=".volt.64"

    port_map={           
        1: 0,
        2: 1,
        3: 15,
        4: 14,
        5: 2,
        6: 3,
        7: 13,
        8: 12,
        9: 4,
        10:5, 
        11:11,
        12:10,
        13:6,
        14:7,
        15:9,
        16:8
    }

    def baseconfig(self):
        #self.insmod('ym2651y')
        self.insmod('optoe')
        for m in [ 'cpld', 'fan', 'psu', 'leds', 'thermal', 'sys', 'fpga' ]:
            self.insmod("x86-64-accton-asgvolt64-%s.ko" % m)

        ########### initialize I2C bus 0 ###########
        self.new_i2c_devices([
            # initialize root(one-level) multiplexer (PCA9548)
            ('pca9548', 0x77, 0),
            # initiate (two-level) multiplexer (PCA9548)
            ('pca9548', 0x70, 1),
            # initiate (three-level) multiplexer (PCA9548)
            ('pca9548', 0x72, 16),
            
            # initiate (four-level) multiplexer (PCA9548) (for 25g-sfp)
            ('pca9548', 0x75, 19),
            # initiate (four-level) multiplexer (PCA9548) (for 8 pca mux to xfp)
            ('pca9548', 0x75, 22),
            
            # initiate (five-level) multiplexer (PCA9548) (for xfp)
            ('pca9548', 0x76, 33),
            ('pca9548', 0x76, 34),
            ('pca9548', 0x76, 35),
            ('pca9548', 0x76, 36),
            ('pca9548', 0x76, 37),
            ('pca9548', 0x76, 38),
            ('pca9548', 0x76, 39),
            ('pca9548', 0x76, 40),
            
            ('asgvlot64_cpld1', 0x60, 9),
            ('asgvlot64_cpld2', 0x61, 10),
            ('asgvlot64_cpld3', 0x62, 11)
        ])
       
        # initialize XFP port 1~64
        for port in range(41, 105):
            self.new_i2c_device('optoe2', 0x50, port)
        
        base=41    
        for port in range(1, 65):
            q=port/16
            r=port%16
            if r==0:
                r=16
		q=q-1
            bus=base+q*16+self.port_map[r]
            subprocess.call('echo port%d > /sys/bus/i2c/devices/%d-0050/port_name' % (port, bus), shell=True)
            
        # initialize QSFP port 1~2 (port_name=port65~66)
        for port in range(20, 22):
            self.new_i2c_device('optoe1', 0x50, port)
            subprocess.call('echo port%d > /sys/bus/i2c/devices/%d-0050/port_name' % (port+45, port), shell=True)        
        
        # initialize SFP port 1~8 (port_name=port67~74)
        for port in range(25, 33):
            self.new_i2c_device('optoe2', 0x50, port)
            subprocess.call('echo port%d > /sys/bus/i2c/devices/%d-0050/port_name' % (port+41, port), shell=True)
        # initiate IDPROM
        #self.new_i2c_device('24c02', 0x57, 0)
        
        ir3570_check()

        fpga_pcie_init()

        return True

