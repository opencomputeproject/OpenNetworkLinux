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


class OnlPlatform_x86_64_accton_as7926_80xk_r0(OnlPlatformAccton,
                                              OnlPlatformPortConfig_80x100):
    PLATFORM='x86-64-accton-as7926-80xk-r0'
    MODEL="AS7926-80xk"
    SYS_OBJECT_ID=".7926.80"

    def baseconfig(self):
        self.insmod('optoe')
        self.insmod('ym2651y')
        self.insmod('accton_i2c_cpld')
        for m in [ 'fan', 'cpld', 'psu', 'leds']:
            self.insmod("x86-64-accton-as7926-80xk-%s.ko" % m)

        ########### initialize I2C bus 0 ###########
        self.new_i2c_devices([
                # initialize multiplexer (PCA9548)
                ('pca9548', 0x77, 0),

                # initiate leaf multiplexer (PCA9548) of bottom board
                ('pca9548', 0x76, 1),
                ('pca9548', 0x72, 2),
                ('pca9548', 0x73, 9),

                # initiate PSU-1
                ('as7926_80xk_psu1', 0x53, 18),
                ('ym2651', 0x5b, 18),
                
                # initiate PSU-2
                ('as7926_80xk_psu2', 0x51, 18),
                ('ym2651', 0x59, 18),
                
                 # initiate PSU-3
                ('as7926_80xk_psu3', 0x52, 18),
                ('ym2651', 0x5a, 18),
                
                # initiate PSU-4
                ('as7926_80xk_psu4', 0x50, 18),
                ('ym2651', 0x58, 18),
                
                # initiate chassis fan
                ('as7926_80xk_fan', 0x66, 22),
                
                #initiate CPLD
                ('accton_i2c_cpld', 0x60, 11),
                ('as7926_80xk_cpld1', 0x62, 12),
                ('as7926_80xk_cpld2', 0x63, 13),

                # initiate leaf multiplexer (PCA9548)
                ('pca9548', 0x74, 25),
                ('pca9548', 0x74, 26),
                ('pca9548', 0x74, 27),
                ('pca9548', 0x74, 28),
                ('pca9548', 0x74, 29),
                
                # initialize QSFP port 1-40 of bottom board
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
                ('optoe1', 0x50, 57),
                ('optoe1', 0x50, 58),
                ('optoe1', 0x50, 59),
                ('optoe1', 0x50, 60),
                ('optoe1', 0x50, 61),
                ('optoe1', 0x50, 62),
                ('optoe1', 0x50, 63),
                ('optoe1', 0x50, 64),
                ('optoe1', 0x50, 65),
                ('optoe1', 0x50, 66),
                ('optoe1', 0x50, 67),
                ('optoe1', 0x50, 68),
                ('optoe1', 0x50, 69),
                ('optoe1', 0x50, 70),
                ('optoe1', 0x50, 71),
                ('optoe1', 0x50, 72),

                ('24c02', 0x57, 0),
                
                # initiate leaf multiplexer (PCA9548) of top board
                ('pca9548', 0x75, 1),
                ('pca9548', 0x71, 2),
                ('pca9548', 0x73, 73),
                
                ('as7926_80xk_cpld3', 0x62, 76),
                ('as7926_80xk_cpld4', 0x63, 77),
                
                # initiate leaf multiplexer (PCA9548)
                ('pca9548', 0x74, 89),
                ('pca9548', 0x74, 90),
                ('pca9548', 0x74, 91),
                ('pca9548', 0x74, 92),
                ('pca9548', 0x74, 93),
                
                # initialize QSFP port 41-80 of top board
                ('optoe1', 0x50, 97),
                ('optoe1', 0x50, 98),
                ('optoe1', 0x50, 99),
                ('optoe1', 0x50, 100),
                ('optoe1', 0x50, 101),
                ('optoe1', 0x50, 102),
                ('optoe1', 0x50, 103),
                ('optoe1', 0x50, 104),
                ('optoe1', 0x50, 105),
                ('optoe1', 0x50, 106),
                ('optoe1', 0x50, 107),
                ('optoe1', 0x50, 108),
                ('optoe1', 0x50, 109),
                ('optoe1', 0x50, 110),
                ('optoe1', 0x50, 111),
                ('optoe1', 0x50, 112),
                ('optoe1', 0x50, 113),
                ('optoe1', 0x50, 114),
                ('optoe1', 0x50, 115),
                ('optoe1', 0x50, 116),
                ('optoe1', 0x50, 117),
                ('optoe1', 0x50, 118),
                ('optoe1', 0x50, 119),
                ('optoe1', 0x50, 120),
                ('optoe1', 0x50, 121),
                ('optoe1', 0x50, 122),
                ('optoe1', 0x50, 123),
                ('optoe1', 0x50, 124),
                ('optoe1', 0x50, 125),
                ('optoe1', 0x50, 126),
                ('optoe1', 0x50, 127),
                ('optoe1', 0x50, 128),
                ('optoe1', 0x50, 129),
                ('optoe1', 0x50, 130),
                ('optoe1', 0x50, 131),
                ('optoe1', 0x50, 132),
                ('optoe1', 0x50, 133),
                ('optoe1', 0x50, 134),
                ('optoe1', 0x50, 135),
                ('optoe1', 0x50, 136),
                
                
                ])
                
                
        subprocess.call('echo port1 > /sys/bus/i2c/devices/33-0050/port_name', shell=True)
        subprocess.call('echo port2 > /sys/bus/i2c/devices/34-0050/port_name', shell=True)
        subprocess.call('echo port3 > /sys/bus/i2c/devices/35-0050/port_name', shell=True)
        subprocess.call('echo port4 > /sys/bus/i2c/devices/36-0050/port_name', shell=True)
        subprocess.call('echo port5 > /sys/bus/i2c/devices/37-0050/port_name', shell=True)
        subprocess.call('echo port6 > /sys/bus/i2c/devices/38-0050/port_name', shell=True)
        subprocess.call('echo port7 > /sys/bus/i2c/devices/39-0050/port_name', shell=True)
        subprocess.call('echo port8 > /sys/bus/i2c/devices/40-0050/port_name', shell=True)
        subprocess.call('echo port9  > /sys/bus/i2c/devices/41-0050/port_name', shell=True)
        subprocess.call('echo port10 > /sys/bus/i2c/devices/42-0050/port_name', shell=True)
        subprocess.call('echo port11 > /sys/bus/i2c/devices/43-0050/port_name', shell=True)
        subprocess.call('echo port12 > /sys/bus/i2c/devices/44-0050/port_name', shell=True)
        subprocess.call('echo port13  > /sys/bus/i2c/devices/45-0050/port_name', shell=True)
        subprocess.call('echo port14  > /sys/bus/i2c/devices/46-0050/port_name', shell=True)
        subprocess.call('echo port15  > /sys/bus/i2c/devices/47-0050/port_name', shell=True)
        subprocess.call('echo port16  > /sys/bus/i2c/devices/48-0050/port_name', shell=True)
        subprocess.call('echo port17  > /sys/bus/i2c/devices/49-0050/port_name', shell=True)
        subprocess.call('echo port18  > /sys/bus/i2c/devices/50-0050/port_name', shell=True)
        subprocess.call('echo port19  > /sys/bus/i2c/devices/51-0050/port_name', shell=True)
        subprocess.call('echo port20  > /sys/bus/i2c/devices/52-0050/port_name', shell=True)
        subprocess.call('echo port21 > /sys/bus/i2c/devices/53-0050/port_name', shell=True)
        subprocess.call('echo port22 > /sys/bus/i2c/devices/54-0050/port_name', shell=True)
        subprocess.call('echo port23 > /sys/bus/i2c/devices/55-0050/port_name', shell=True)
        subprocess.call('echo port24 > /sys/bus/i2c/devices/56-0050/port_name', shell=True)
        subprocess.call('echo port25 > /sys/bus/i2c/devices/57-0050/port_name', shell=True)
        subprocess.call('echo port26 > /sys/bus/i2c/devices/58-0050/port_name', shell=True)
        subprocess.call('echo port27 > /sys/bus/i2c/devices/59-0050/port_name', shell=True)
        subprocess.call('echo port28 > /sys/bus/i2c/devices/60-0050/port_name', shell=True)
        subprocess.call('echo port29 > /sys/bus/i2c/devices/61-0050/port_name', shell=True)
        subprocess.call('echo port30 > /sys/bus/i2c/devices/62-0050/port_name', shell=True)
        subprocess.call('echo port31 > /sys/bus/i2c/devices/63-0050/port_name', shell=True)
        subprocess.call('echo port32 > /sys/bus/i2c/devices/64-0050/port_name', shell=True)
        subprocess.call('echo port33 > /sys/bus/i2c/devices/65-0050/port_name', shell=True)
        subprocess.call('echo port34 > /sys/bus/i2c/devices/66-0050/port_name', shell=True)
        subprocess.call('echo port35 > /sys/bus/i2c/devices/67-0050/port_name', shell=True)
        subprocess.call('echo port36 > /sys/bus/i2c/devices/68-0050/port_name', shell=True)
        subprocess.call('echo port37 > /sys/bus/i2c/devices/69-0050/port_name', shell=True)
        subprocess.call('echo port38 > /sys/bus/i2c/devices/70-0050/port_name', shell=True)
        subprocess.call('echo port39 > /sys/bus/i2c/devices/71-0050/port_name', shell=True)
        subprocess.call('echo port40 > /sys/bus/i2c/devices/72-0050/port_name', shell=True)
        
        subprocess.call('echo port41 > /sys/bus/i2c/devices/97-0050/port_name', shell=True)
        subprocess.call('echo port42 > /sys/bus/i2c/devices/98-0050/port_name', shell=True)
        subprocess.call('echo port43 > /sys/bus/i2c/devices/99-0050/port_name', shell=True)
        subprocess.call('echo port44 > /sys/bus/i2c/devices/100-0050/port_name', shell=True)
        subprocess.call('echo port45 > /sys/bus/i2c/devices/101-0050/port_name', shell=True)
        subprocess.call('echo port46 > /sys/bus/i2c/devices/102-0050/port_name', shell=True)
        subprocess.call('echo port47 > /sys/bus/i2c/devices/103-0050/port_name', shell=True)
        subprocess.call('echo port48 > /sys/bus/i2c/devices/104-0050/port_name', shell=True)
        subprocess.call('echo port49  > /sys/bus/i2c/devices/105-0050/port_name', shell=True)
        subprocess.call('echo port50 > /sys/bus/i2c/devices/106-0050/port_name', shell=True)
        subprocess.call('echo port51 > /sys/bus/i2c/devices/107-0050/port_name', shell=True)
        subprocess.call('echo port52 > /sys/bus/i2c/devices/108-0050/port_name', shell=True)
        subprocess.call('echo port53  > /sys/bus/i2c/devices/109-0050/port_name', shell=True)
        subprocess.call('echo port54  > /sys/bus/i2c/devices/110-0050/port_name', shell=True)
        subprocess.call('echo port55  > /sys/bus/i2c/devices/111-0050/port_name', shell=True)
        subprocess.call('echo port56  > /sys/bus/i2c/devices/112-0050/port_name', shell=True)
        subprocess.call('echo port57  > /sys/bus/i2c/devices/113-0050/port_name', shell=True)
        subprocess.call('echo port58  > /sys/bus/i2c/devices/114-0050/port_name', shell=True)
        subprocess.call('echo port59  > /sys/bus/i2c/devices/115-0050/port_name', shell=True)
        subprocess.call('echo port60  > /sys/bus/i2c/devices/116-0050/port_name', shell=True)
        subprocess.call('echo port61 > /sys/bus/i2c/devices/117-0050/port_name', shell=True)
        subprocess.call('echo port62 > /sys/bus/i2c/devices/118-0050/port_name', shell=True)
        subprocess.call('echo port63 > /sys/bus/i2c/devices/119-0050/port_name', shell=True)
        subprocess.call('echo port64 > /sys/bus/i2c/devices/120-0050/port_name', shell=True)
        subprocess.call('echo port65 > /sys/bus/i2c/devices/121-0050/port_name', shell=True)
        subprocess.call('echo port66 > /sys/bus/i2c/devices/122-0050/port_name', shell=True)
        subprocess.call('echo port67 > /sys/bus/i2c/devices/123-0050/port_name', shell=True)
        subprocess.call('echo port68 > /sys/bus/i2c/devices/124-0050/port_name', shell=True)
        subprocess.call('echo port69 > /sys/bus/i2c/devices/125-0050/port_name', shell=True)
        subprocess.call('echo port70 > /sys/bus/i2c/devices/126-0050/port_name', shell=True)
        subprocess.call('echo port71 > /sys/bus/i2c/devices/127-0050/port_name', shell=True)
        subprocess.call('echo port72 > /sys/bus/i2c/devices/128-0050/port_name', shell=True)
        subprocess.call('echo port73 > /sys/bus/i2c/devices/129-0050/port_name', shell=True)
        subprocess.call('echo port74 > /sys/bus/i2c/devices/130-0050/port_name', shell=True)
        subprocess.call('echo port75 > /sys/bus/i2c/devices/131-0050/port_name', shell=True)
        subprocess.call('echo port76 > /sys/bus/i2c/devices/132-0050/port_name', shell=True)
        subprocess.call('echo port77 > /sys/bus/i2c/devices/133-0050/port_name', shell=True)
        subprocess.call('echo port78 > /sys/bus/i2c/devices/134-0050/port_name', shell=True)
        subprocess.call('echo port79 > /sys/bus/i2c/devices/135-0050/port_name', shell=True)
        subprocess.call('echo port80 > /sys/bus/i2c/devices/136-0050/port_name', shell=True)
                
        ir3570_check()

        return True
