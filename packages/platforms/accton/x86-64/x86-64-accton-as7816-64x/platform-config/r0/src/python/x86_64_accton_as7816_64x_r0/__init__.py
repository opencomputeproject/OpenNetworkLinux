from onl.platform.base import *
from onl.platform.accton import *

class OnlPlatform_x86_64_accton_as7816_64x_r0(OnlPlatformAccton,
                                              OnlPlatformPortConfig_64x100):
    PLATFORM='x86-64-accton-as7816-64x-r0'
    MODEL="AS7816-64x"
    SYS_OBJECT_ID=".7816.64"
    PSU1_MODEL="/sys/bus/i2c/devices/i2c-10/10-005b/psu_mfr_model"
    PSU2_MODEL="/sys/bus/i2c/devices/i2c-9/9-0058/psu_mfr_model"
    PSU1_POWER="/sys/bus/i2c/devices/i2c-19/19-0060/psu1_power_good"
    PSU2_POWER="/sys/bus/i2c/devices/i2c-19/19-0060/psu2_power_good"
    

    def baseconfig(self):
        self.insmod('optoe')
        self.insmod('ym2651y')
        self.insmod('dps850')
        self.insmod('accton_i2c_cpld')
        for m in [ 'fan', 'cpld1', 'leds' ]:
            self.insmod("x86-64-accton-as7816-64x-%s.ko" % m)

        ########### initialize I2C bus 0 ###########
        self.new_i2c_devices([
                # initialize multiplexer (PCA9548)
                ('pca9548', 0x77, 0),

                # initiate leaf multiplexer (PCA9548)
                ('pca9548', 0x71, 1),
                ('pca9548', 0x76, 1),
                ('pca9548', 0x73, 1),

                # initiate PSU-1
                ('24c02', 0x53, 10),
                ('dps850', 0x5b, 10),
                #('ym2851', 0x5b, 10),

                # initiate PSU-2
                ('24c02', 0x50, 9),
                ('dps850', 0x58, 9),
                #('ym2851', 0x58, 9),

                # initiate chassis fan
                ('as7816_64x_fan', 0x68, 17),

                # inititate LM75
                ('lm75', 0x48, 18),
                ('lm75', 0x49, 18),
                ('lm75', 0x4a, 18),
                ('lm75', 0x4b, 18),
                ('lm75', 0x4d, 17),
                ('lm75', 0x4e, 17),

                #initiate CPLD
                ('as7816_64x_cpld1', 0x60, 19),
                ('accton_i2c_cpld', 0x62, 20),
                ('accton_i2c_cpld', 0x64, 21),
                ('accton_i2c_cpld', 0x66, 22),

                # initiate leaf multiplexer (PCA9548)
                ('pca9548', 0x70, 2),
                ('pca9548', 0x71, 2),
                ('pca9548', 0x72, 2),
                ('pca9548', 0x73, 2),
                ('pca9548', 0x74, 2),
                ('pca9548', 0x75, 2),
                ('pca9548', 0x76, 2),

                # initialize QSFP port 1-64
                ('optoe1', 0x50, 25),
                ('optoe1', 0x50, 26),
                ('optoe1', 0x50, 27),
                ('optoe1', 0x50, 28),
                ('optoe1', 0x50, 29),
                ('optoe1', 0x50, 30),
                ('optoe1', 0x50, 31),
                ('optoe1', 0x50, 32),
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
                ('optoe1', 0x50, 73),
                ('optoe1', 0x50, 74),
                ('optoe1', 0x50, 75),
                ('optoe1', 0x50, 76),
                ('optoe1', 0x50, 77),
                ('optoe1', 0x50, 78),
                ('optoe1', 0x50, 79),
                ('optoe1', 0x50, 80),
                ('optoe1', 0x50, 81),
                ('optoe1', 0x50, 82),
                ('optoe1', 0x50, 83),
                ('optoe1', 0x50, 84),
                ('optoe1', 0x50, 85),
                ('optoe1', 0x50, 86),
                ('optoe1', 0x50, 87),
                ('optoe1', 0x50, 88),

                #('24c02', 0x56, 0),
                ])

        subprocess.call('echo port61 > /sys/bus/i2c/devices/25-0050/port_name', shell=True)
        subprocess.call('echo port62 > /sys/bus/i2c/devices/26-0050/port_name', shell=True)
        subprocess.call('echo port63 > /sys/bus/i2c/devices/27-0050/port_name', shell=True)
        subprocess.call('echo port64 > /sys/bus/i2c/devices/28-0050/port_name', shell=True)
        subprocess.call('echo port55 > /sys/bus/i2c/devices/29-0050/port_name', shell=True)
        subprocess.call('echo port56 > /sys/bus/i2c/devices/30-0050/port_name', shell=True)
        subprocess.call('echo port53 > /sys/bus/i2c/devices/31-0050/port_name', shell=True)
        subprocess.call('echo port54 > /sys/bus/i2c/devices/32-0050/port_name', shell=True)
        subprocess.call('echo port9  > /sys/bus/i2c/devices/33-0050/port_name', shell=True)
        subprocess.call('echo port10 > /sys/bus/i2c/devices/34-0050/port_name', shell=True)
        subprocess.call('echo port11 > /sys/bus/i2c/devices/35-0050/port_name', shell=True)
        subprocess.call('echo port12 > /sys/bus/i2c/devices/36-0050/port_name', shell=True)
        subprocess.call('echo port1  > /sys/bus/i2c/devices/37-0050/port_name', shell=True)
        subprocess.call('echo port2  > /sys/bus/i2c/devices/38-0050/port_name', shell=True)
        subprocess.call('echo port3  > /sys/bus/i2c/devices/39-0050/port_name', shell=True)
        subprocess.call('echo port4  > /sys/bus/i2c/devices/40-0050/port_name', shell=True)
        subprocess.call('echo port6  > /sys/bus/i2c/devices/41-0050/port_name', shell=True)
        subprocess.call('echo port5  > /sys/bus/i2c/devices/42-0050/port_name', shell=True)
        subprocess.call('echo port8  > /sys/bus/i2c/devices/43-0050/port_name', shell=True)
        subprocess.call('echo port7  > /sys/bus/i2c/devices/44-0050/port_name', shell=True)
        subprocess.call('echo port13 > /sys/bus/i2c/devices/45-0050/port_name', shell=True)
        subprocess.call('echo port14 > /sys/bus/i2c/devices/46-0050/port_name', shell=True)
        subprocess.call('echo port15 > /sys/bus/i2c/devices/47-0050/port_name', shell=True)
        subprocess.call('echo port16 > /sys/bus/i2c/devices/48-0050/port_name', shell=True)
        subprocess.call('echo port17 > /sys/bus/i2c/devices/49-0050/port_name', shell=True)
        subprocess.call('echo port18 > /sys/bus/i2c/devices/50-0050/port_name', shell=True)
        subprocess.call('echo port19 > /sys/bus/i2c/devices/51-0050/port_name', shell=True)
        subprocess.call('echo port20 > /sys/bus/i2c/devices/52-0050/port_name', shell=True)
        subprocess.call('echo port25 > /sys/bus/i2c/devices/53-0050/port_name', shell=True)
        subprocess.call('echo port26 > /sys/bus/i2c/devices/54-0050/port_name', shell=True)
        subprocess.call('echo port27 > /sys/bus/i2c/devices/55-0050/port_name', shell=True)
        subprocess.call('echo port28 > /sys/bus/i2c/devices/56-0050/port_name', shell=True)

        subprocess.call('echo port29 > /sys/bus/i2c/devices/57-0050/port_name', shell=True)
        subprocess.call('echo port30 > /sys/bus/i2c/devices/58-0050/port_name', shell=True)
        subprocess.call('echo port31 > /sys/bus/i2c/devices/59-0050/port_name', shell=True)
        subprocess.call('echo port32 > /sys/bus/i2c/devices/60-0050/port_name', shell=True)
        subprocess.call('echo port21 > /sys/bus/i2c/devices/61-0050/port_name', shell=True)
        subprocess.call('echo port22 > /sys/bus/i2c/devices/62-0050/port_name', shell=True)
        subprocess.call('echo port23 > /sys/bus/i2c/devices/63-0050/port_name', shell=True)
        subprocess.call('echo port24 > /sys/bus/i2c/devices/64-0050/port_name', shell=True)
        subprocess.call('echo port41 > /sys/bus/i2c/devices/65-0050/port_name', shell=True)
        subprocess.call('echo port42 > /sys/bus/i2c/devices/66-0050/port_name', shell=True)
        subprocess.call('echo port43 > /sys/bus/i2c/devices/67-0050/port_name', shell=True)
        subprocess.call('echo port44 > /sys/bus/i2c/devices/68-0050/port_name', shell=True)
        subprocess.call('echo port33 > /sys/bus/i2c/devices/69-0050/port_name', shell=True)
        subprocess.call('echo port34 > /sys/bus/i2c/devices/70-0050/port_name', shell=True)
        subprocess.call('echo port35 > /sys/bus/i2c/devices/71-0050/port_name', shell=True)
        subprocess.call('echo port36 > /sys/bus/i2c/devices/72-0050/port_name', shell=True)
        subprocess.call('echo port45 > /sys/bus/i2c/devices/73-0050/port_name', shell=True)
        subprocess.call('echo port46 > /sys/bus/i2c/devices/74-0050/port_name', shell=True)
        subprocess.call('echo port47 > /sys/bus/i2c/devices/75-0050/port_name', shell=True)
        subprocess.call('echo port48 > /sys/bus/i2c/devices/76-0050/port_name', shell=True)
        subprocess.call('echo port37 > /sys/bus/i2c/devices/77-0050/port_name', shell=True)
        subprocess.call('echo port38 > /sys/bus/i2c/devices/78-0050/port_name', shell=True)
        subprocess.call('echo port39 > /sys/bus/i2c/devices/79-0050/port_name', shell=True)
        subprocess.call('echo port40 > /sys/bus/i2c/devices/80-0050/port_name', shell=True)
        subprocess.call('echo port57 > /sys/bus/i2c/devices/81-0050/port_name', shell=True)
        subprocess.call('echo port58 > /sys/bus/i2c/devices/82-0050/port_name', shell=True)
        subprocess.call('echo port59 > /sys/bus/i2c/devices/83-0050/port_name', shell=True)
        subprocess.call('echo port60 > /sys/bus/i2c/devices/84-0050/port_name', shell=True)
        subprocess.call('echo port49 > /sys/bus/i2c/devices/85-0050/port_name', shell=True)
        subprocess.call('echo port50 > /sys/bus/i2c/devices/86-0050/port_name', shell=True)
        subprocess.call('echo port51 > /sys/bus/i2c/devices/87-0050/port_name', shell=True)
        subprocess.call('echo port52 > /sys/bus/i2c/devices/88-0050/port_name', shell=True)

        PSU_DELTA="DPS-850"
        PSU_3Y= "YM-2851F"           
        if os.path.exists(self.PSU2_POWER):
            with open(self.PSU2_POWER, 'r') as fd:
                val=int(fd.read())            
                if val==1:
                    if os.path.exists(self.PSU2_MODEL):
                        with open(self.PSU2_MODEL, 'r') as fd:
                            f=open(self.PSU2_MODEL)
                            val_str=f.read()
                            if int(val_str.find(PSU_3Y))== 0:
                                subprocess.call('echo 0x58 > /sys/bus/i2c/devices/i2c-9/delete_device', shell=True)
                                subprocess.call('echo 0x5b > /sys/bus/i2c/devices/i2c-10/delete_device', shell=True)
                                self.new_i2c_devices([
                                 ('ym2851', 0x58, 9),
                                 ('ym2851', 0x5b, 10),
                                ])
                                return True
        if os.path.exists(self.PSU1_POWER):
            with open(self.PSU1_POWER, 'r') as fd:
                val=int(fd.read())            
                if val==1:
                    if os.path.exists(self.PSU1_MODEL):
                        with open(self.PSU1_MODEL, 'r') as fd:
                            f=open(self.PSU1_MODEL)
                            val_str=f.read()
                            if int(val_str.find(PSU_3Y))== 0:
                                subprocess.call('echo 0x58 > /sys/bus/i2c/devices/i2c-9/delete_device', shell=True)
                                subprocess.call('echo 0x5b > /sys/bus/i2c/devices/i2c-10/delete_device', shell=True)
                                self.new_i2c_devices([
                                 ('ym2851', 0x58, 9),
                                 ('ym2851', 0x5b, 10),
                                ])
                                return True                        
                
        return True
