from onl.platform.base import *
from onl.platform.accton import *

class OnlPlatform_x86_64_accton_wedge100bf_32x_r0(OnlPlatformAccton,
                                                OnlPlatformPortConfig_32x100):
    MODEL="Wedge-100bf-32X"
    PLATFORM="x86-64-accton-wedge100bf-32x-r0"
    SYS_OBJECT_ID=".100.32.2"

    def baseconfig(self):
        self.insmod('optoe')
        
        ########### initialize I2C bus 1 ###########
        self.new_i2c_devices([
                # initialize multiplexer (PCA9548)
                ('pca9548', 0x70, 1),
                ('pca9548', 0x71, 1),
                ('pca9548', 0x72, 1),
                ('pca9548', 0x73, 1),
                ('pca9548', 0x74, 1),

                ('24c64', 0x50, 40),
                ])
                
        # Initialize QSFP devices
        self.new_i2c_device('optoe1', 0x50, 2)
        self.new_i2c_device('optoe1', 0x50, 3)
        self.new_i2c_device('optoe1', 0x50, 4)
        self.new_i2c_device('optoe1', 0x50, 5)
        self.new_i2c_device('optoe1', 0x50, 6)
        self.new_i2c_device('optoe1', 0x50, 7)
        self.new_i2c_device('optoe1', 0x50, 8)
        self.new_i2c_device('optoe1', 0x50, 9)
        self.new_i2c_device('optoe1', 0x50, 10)
        self.new_i2c_device('optoe1', 0x50, 11)
        self.new_i2c_device('optoe1', 0x50, 12)
        self.new_i2c_device('optoe1', 0x50, 13)
        self.new_i2c_device('optoe1', 0x50, 14)
        self.new_i2c_device('optoe1', 0x50, 15)
        self.new_i2c_device('optoe1', 0x50, 16)
        self.new_i2c_device('optoe1', 0x50, 17)
        self.new_i2c_device('optoe1', 0x50, 18)
        self.new_i2c_device('optoe1', 0x50, 19)
        self.new_i2c_device('optoe1', 0x50, 20)
        self.new_i2c_device('optoe1', 0x50, 21)
        self.new_i2c_device('optoe1', 0x50, 22)
        self.new_i2c_device('optoe1', 0x50, 23)
        self.new_i2c_device('optoe1', 0x50, 24)
        self.new_i2c_device('optoe1', 0x50, 25)
        self.new_i2c_device('optoe1', 0x50, 26)
        self.new_i2c_device('optoe1', 0x50, 27)
        self.new_i2c_device('optoe1', 0x50, 28)
        self.new_i2c_device('optoe1', 0x50, 29)
        self.new_i2c_device('optoe1', 0x50, 30)
        self.new_i2c_device('optoe1', 0x50, 31)
        self.new_i2c_device('optoe1', 0x50, 32)
        self.new_i2c_device('optoe1', 0x50, 33)
        subprocess.call('echo port1 > /sys/bus/i2c/devices/3-0050/port_name', shell=True)
        subprocess.call('echo port2 > /sys/bus/i2c/devices/2-0050/port_name', shell=True)
        subprocess.call('echo port3 > /sys/bus/i2c/devices/5-0050/port_name', shell=True)
        subprocess.call('echo port4 > /sys/bus/i2c/devices/4-0050/port_name', shell=True)
        subprocess.call('echo port5 > /sys/bus/i2c/devices/7-0050/port_name', shell=True)
        subprocess.call('echo port6 > /sys/bus/i2c/devices/6-0050/port_name', shell=True)
        subprocess.call('echo port7 > /sys/bus/i2c/devices/9-0050/port_name', shell=True)
        subprocess.call('echo port8 > /sys/bus/i2c/devices/8-0050/port_name', shell=True)
        subprocess.call('echo port9 > /sys/bus/i2c/devices/11-0050/port_name', shell=True)
        subprocess.call('echo port10 > /sys/bus/i2c/devices/10-0050/port_name', shell=True)
        subprocess.call('echo port11 > /sys/bus/i2c/devices/13-0050/port_name', shell=True)
        subprocess.call('echo port12 > /sys/bus/i2c/devices/12-0050/port_name', shell=True)
        subprocess.call('echo port13 > /sys/bus/i2c/devices/15-0050/port_name', shell=True)
        subprocess.call('echo port14 > /sys/bus/i2c/devices/14-0050/port_name', shell=True)
        subprocess.call('echo port15 > /sys/bus/i2c/devices/17-0050/port_name', shell=True)
        subprocess.call('echo port16 > /sys/bus/i2c/devices/16-0050/port_name', shell=True)
        subprocess.call('echo port17 > /sys/bus/i2c/devices/19-0050/port_name', shell=True)
        subprocess.call('echo port18 > /sys/bus/i2c/devices/18-0050/port_name', shell=True)
        subprocess.call('echo port19 > /sys/bus/i2c/devices/21-0050/port_name', shell=True)
        subprocess.call('echo port20 > /sys/bus/i2c/devices/20-0050/port_name', shell=True)
        subprocess.call('echo port21 > /sys/bus/i2c/devices/23-0050/port_name', shell=True)
        subprocess.call('echo port22 > /sys/bus/i2c/devices/22-0050/port_name', shell=True)
        subprocess.call('echo port23 > /sys/bus/i2c/devices/25-0050/port_name', shell=True)
        subprocess.call('echo port24 > /sys/bus/i2c/devices/24-0050/port_name', shell=True)
        subprocess.call('echo port25 > /sys/bus/i2c/devices/27-0050/port_name', shell=True)
        subprocess.call('echo port26 > /sys/bus/i2c/devices/26-0050/port_name', shell=True)
        subprocess.call('echo port27 > /sys/bus/i2c/devices/29-0050/port_name', shell=True)
        subprocess.call('echo port28 > /sys/bus/i2c/devices/28-0050/port_name', shell=True)
        subprocess.call('echo port29 > /sys/bus/i2c/devices/31-0050/port_name', shell=True)
        subprocess.call('echo port30 > /sys/bus/i2c/devices/30-0050/port_name', shell=True)
        subprocess.call('echo port31 > /sys/bus/i2c/devices/33-0050/port_name', shell=True)
        subprocess.call('echo port32 > /sys/bus/i2c/devices/32-0050/port_name', shell=True)
        
        return True
