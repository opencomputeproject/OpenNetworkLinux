from onl.platform.base import *
from onl.platform.ufispace import *
from struct import *
from ctypes import c_int, sizeof
import os
import sys
import subprocess
import time
import fcntl

def msg(s, fatal=False):
    sys.stderr.write(s)
    sys.stderr.flush()
    if fatal:
        sys.exit(1)

class IPMI_Ioctl(object):
    _IONONE = 0
    _IOWRITE = 1
    _IOREAD = 2

    IPMI_MAINTENANCE_MODE_AUTO = 0
    IPMI_MAINTENANCE_MODE_OFF  = 1
    IPMI_MAINTENANCE_MODE_ON   = 2

    IPMICTL_GET_MAINTENANCE_MODE_CMD = _IOREAD << 30 | sizeof(c_int) << 16 | \
        ord('i') << 8 | 30  # from ipmi.h
    IPMICTL_SET_MAINTENANCE_MODE_CMD = _IOWRITE << 30 | sizeof(c_int) << 16 | \
        ord('i') << 8 | 31  # from ipmi.h

    def __init__(self):
        self.ipmidev = None
        devnodes=["/dev/ipmi0", "/dev/ipmi/0", "/dev/ipmidev/0"]
        for dev in devnodes:
            try:
                self.ipmidev = open(dev, 'rw')
                break
            except Exception as e:
                print("open file {} failed, error: {}".format(dev, e))

    def __del__(self):
        if self.ipmidev is not None:
            self.ipmidev.close()

    def get_ipmi_maintenance_mode(self):
        input_buffer=pack('i',0)
        out_buffer=fcntl.ioctl(self.ipmidev, self.IPMICTL_GET_MAINTENANCE_MODE_CMD, input_buffer)
        maintanence_mode=unpack('i',out_buffer)[0]

        return maintanence_mode

    def set_ipmi_maintenance_mode(self, mode):
        fcntl.ioctl(self.ipmidev, self.IPMICTL_SET_MAINTENANCE_MODE_CMD, c_int(mode))

class OnlPlatform_x86_64_ufispace_s9500_22xst_r0(OnlPlatformUfiSpace):
    PLATFORM='x86-64-ufispace-s9500-22xst-r0'
    MODEL="S9500-22XST"
    SYS_OBJECT_ID=".9500.22"
    PORT_COUNT=22
    PORT_CONFIG="2x100 + 8x25 + 8x10 + 4x1"

    I2C_ADDR_TPS53667 = 0x61
    TPS53667_ROV_ADDR = 0x21
    ROV_List = ['N/A' , '1.00' , '0.95' , 'N/A' , '1.04']
    TPS53667_voltage = [
        {"rov": "1.00", "vol": 0x0097},
        {"rov": "0.95", "vol": 0x008D},
        {"rov": "1.04", "vol": 0x009F}
    ]
     
    def check_bmc_enable(self):
        return 1
        
    def baseconfig(self):

        bmc_enable = self.check_bmc_enable()
        msg("bmc enable : %r\n" % (True if bmc_enable else False))
        
        # record the result for onlp
        os.system("echo %d > /etc/onl/bmc_en" % bmc_enable)

        ########### initialize I2C bus 0 ###########
        msg("****** Start S9500-22XST platform initializing ... ******\n")
        # init PCA9546, PCA9548
        msg("Insert I2C Mux\n")
        self.new_i2c_devices(
            [
                ('pca9546', 0x75, 0),
                ('pca9546', 0x76, 0),
                ('pca9548', 0x72, 8),
                ('pca9548', 0x73, 8),
                ('pca9548', 0x74, 8),
                ('pca9546', 0x70, 8),
            ]
        )

        # Config MUX as MUX_IDLE_DISCONNECT mode
        os.system("ls /sys/devices/pci0000:00/0000:00:1f.3/i2c-0/0-0075/idle_state > /dev/null 2>&1 &&"
                  "echo -2 > /sys/devices/pci0000:00/0000:00:1f.3/i2c-0/0-0075/idle_state")
        os.system("ls /sys/devices/pci0000:00/0000:00:1f.3/i2c-0/0-0076/idle_state > /dev/null 2>&1 &&"
                  "echo -2 > /sys/devices/pci0000:00/0000:00:1f.3/i2c-0/0-0076/idle_state")
        os.system("ls /sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-8/8-0070/idle_state > /dev/null 2>&1 &&"
                  "echo -2 > /sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-8/8-0070/idle_state")
        os.system("ls /sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-8/8-0072/idle_state > /dev/null 2>&1 &&"
                  "echo -2 > /sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-8/8-0072/idle_state")
        os.system("ls /sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-8/8-0073/idle_state > /dev/null 2>&1 &&"
                  "echo -2 > /sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-8/8-0073/idle_state")
        os.system("ls /sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-8/8-0074/idle_state > /dev/null 2>&1 &&"
                  "echo -2 > /sys/devices/pci0000:00/0000:00:1f.3/i2c-0/i2c-8/8-0074/idle_state")
        
        self.insmod("x86-64-ufispace-eeprom-mb")
        self.insmod("optoe")

        # init SYS EEPROM devices
        msg("Insert EEPROM\n")
        self.new_i2c_devices(
            [
                #  on cpu board
                ('mb_eeprom', 0x57, 0),
            ]
        )

        # init SFP+ EEPROM
        msg("Insert SFP+ EEPROM\n")
        for port in range(9, 17):
            self.new_i2c_device('optoe2', 0x50, port)
            subprocess.call("echo {} > /sys/bus/i2c/devices/{}-0050/port_name".format(port-5, port), shell=True)

        # init SFP28 EEPROM
        msg("Insert SFP28 EEPROM\n")
        for port in range(21, 29):
            self.new_i2c_device('optoe2', 0x50, port)
            subprocess.call("echo {} > /sys/bus/i2c/devices/{}-0050/port_name".format(port-9, port), shell=True)

        # init QSFP28 EEPROM
        msg("Insert QSFP EEPROM\n")
        for port in range(35, 37):
            self.new_i2c_device('optoe1', 0x50, port)
            #self.new_i2c_device('sff8436', 0x50, port)
            subprocess.call("echo {} > /sys/bus/i2c/devices/{}-0050/port_name".format(56-port, port), shell=True)
           
        # init Temperature
        msg("Insert CPU TMP75\n")
        self.new_i2c_devices(
            [               
                # CPU Board Temp
                ('tmp75', 0x4F, 0),
            ]
        )

        # init GPIO sysfs
        msg("Insert IOExp\n")
        #9539_CPU_STATUS
        self.new_i2c_device('pca9539', 0x77, 0)
        #9535_SFP+_TX_DIS_1
        self.new_i2c_device('pca9535', 0x22, 5)
        #9535_SFP+_TX_DIS_2
        self.new_i2c_device('pca9535', 0x24, 5)
        #9535_QSFP28
        self.new_i2c_device('pca9535', 0x21, 5)
        #9535_SFP+_TX_FLT_1
        self.new_i2c_device('pca9535', 0x26, 6)
        #9535_SFP+_TX_FLT_2
        self.new_i2c_device('pca9535', 0x27, 6)
        #9535_SFP+_RATE_SEL_1
        self.new_i2c_device('pca9535', 0x25, 6)
        #9535_SFP+_RATE_SEL_2
        self.new_i2c_device('pca9535', 0x23, 6)
        #9535_SFP+_MOD_ABS_1
        self.new_i2c_device('pca9535', 0x20, 7)
        #9535_SFP+_MOD_ABS_2
        self.new_i2c_device('pca9535', 0x22, 7)
        #9535_SFP+_RX_LOS_1
        self.new_i2c_device('pca9535', 0x21, 7)
        #9535_SFP+_RX_LOS_2
        self.new_i2c_device('pca9535', 0x24, 7)
        #9535_BOARD_ID
        self.new_i2c_device('pca9539', 0x20, 3)

        # export GPIO
        msg("Export GPIO\n")
        for i in reversed(range(304, 512)):
            os.system("echo {} > /sys/class/gpio/export".format(i))

        # init GPIO direction
        msg("Config GPIO\n")
        # 9539_CPU_STATUS
        for i in range(496, 512):
            os.system("echo in > /sys/class/gpio/gpio{}/direction".format(i))

        # init GPIO direction
        # 9535_SFP+_TX_DIS_1
        for i in range(480, 496):
            os.system("echo out > /sys/class/gpio/gpio{}/direction".format(i))
        # init GPIO value
        # 9535_SFP+_TX_DIS_1
        for i in range(480, 496):
            os.system("echo 0 > /sys/class/gpio/gpio{}/value".format(i))

        # init GPIO direction
        # 9535_SFP+_TX_DIS_2
        for i in range(464, 480):
            os.system("echo out > /sys/class/gpio/gpio{}/direction".format(i))
        # init GPIO value
        # 9535_SFP+_TX_DIS_2
        for i in range(464, 480):
            os.system("echo 0 > /sys/class/gpio/gpio{}/value".format(i))

        # init GPIO direction
        # 9535_QSFP28
        os.system("echo in > /sys/class/gpio/gpio463/direction")
        os.system("echo in > /sys/class/gpio/gpio462/direction")
        os.system("echo out > /sys/class/gpio/gpio461/direction")
        os.system("echo out > /sys/class/gpio/gpio460/direction")
        os.system("echo in > /sys/class/gpio/gpio459/direction")
        os.system("echo in > /sys/class/gpio/gpio458/direction")
        os.system("echo out > /sys/class/gpio/gpio457/direction")
        os.system("echo out > /sys/class/gpio/gpio456/direction")
        os.system("echo in > /sys/class/gpio/gpio455/direction")
        os.system("echo in > /sys/class/gpio/gpio454/direction")
        os.system("echo in > /sys/class/gpio/gpio453/direction")
        os.system("echo in > /sys/class/gpio/gpio452/direction")
        os.system("echo in > /sys/class/gpio/gpio451/direction")
        os.system("echo in > /sys/class/gpio/gpio450/direction")
        os.system("echo in > /sys/class/gpio/gpio449/direction")
        os.system("echo in > /sys/class/gpio/gpio448/direction")
        # init GPIO value
        # 9535_QSFP28
        os.system("echo 1 > /sys/class/gpio/gpio461/value")
        os.system("echo 1 > /sys/class/gpio/gpio460/value")
        os.system("echo 0 > /sys/class/gpio/gpio457/value")
        os.system("echo 0 > /sys/class/gpio/gpio456/value")

        # init GPIO direction
        # 9535_SFP+_TX_FLT_1, 9535_SFP+_TX_FLT_2
        for i in range(416, 448):
            os.system("echo in > /sys/class/gpio/gpio{}/direction".format(i))

        # init GPIO direction
        # 9535_SFP+_RATE_SEL_1
        for i in range(400, 416):
            os.system("echo out > /sys/class/gpio/gpio{}/direction".format(i))
        # init GPIO value
        # 9535_SFP+_RATE_SEL_1
        for i in range(400, 416):
            os.system("echo 1 > /sys/class/gpio/gpio{}/value".format(i))

        # init GPIO direction
        # 9535_SFP+_RATE_SEL_2
        os.system("echo out > /sys/class/gpio/gpio399/direction")
        os.system("echo out > /sys/class/gpio/gpio398/direction")
        os.system("echo out > /sys/class/gpio/gpio397/direction")
        os.system("echo out > /sys/class/gpio/gpio396/direction")
        os.system("echo in > /sys/class/gpio/gpio395/direction")
        os.system("echo in > /sys/class/gpio/gpio394/direction")
        os.system("echo in > /sys/class/gpio/gpio393/direction")
        os.system("echo in > /sys/class/gpio/gpio392/direction")
        os.system("echo out > /sys/class/gpio/gpio391/direction")
        os.system("echo out > /sys/class/gpio/gpio390/direction")
        os.system("echo out > /sys/class/gpio/gpio389/direction")
        os.system("echo out > /sys/class/gpio/gpio388/direction")
        os.system("echo in > /sys/class/gpio/gpio387/direction")
        os.system("echo in > /sys/class/gpio/gpio386/direction")
        os.system("echo in > /sys/class/gpio/gpio385/direction")
        os.system("echo in > /sys/class/gpio/gpio384/direction")
        # init GPIO value
        # 9535_SFP+_RATE_SEL_2
        os.system("echo 1 > /sys/class/gpio/gpio399/value")
        os.system("echo 1 > /sys/class/gpio/gpio398/value")
        os.system("echo 1 > /sys/class/gpio/gpio397/value")
        os.system("echo 1 > /sys/class/gpio/gpio396/value")
        os.system("echo 1 > /sys/class/gpio/gpio391/value")
        os.system("echo 1 > /sys/class/gpio/gpio390/value")
        os.system("echo 1 > /sys/class/gpio/gpio389/value")
        os.system("echo 1 > /sys/class/gpio/gpio388/value")

        # init GPIO direction
        # 9535_SFP+_MOD_ABS_1, 9535_SFP+_MOD_ABS_2, 9535_SFP+_RX_LOS_1,
        # 9535_SFP+_RX_LOS_2, 9535_BOARD_ID
        for i in range(304, 384):
            os.system("echo in > /sys/class/gpio/gpio{}/direction".format(i))

        # get mac rov config
        msg("Config MAC ROV\n")
        data_4 = ""
        data_2 = ""
        data_1 = ""
        if (os.path.exists("/sys/class/gpio/gpio306")):
            with open("/sys/class/gpio/gpio306/value") as f:
              data_4 = f.read()
        if (os.path.exists("/sys/class/gpio/gpio305")):
            with open("/sys/class/gpio/gpio305/value") as f:
              data_2 = f.read()
        if (os.path.exists("/sys/class/gpio/gpio304")):
            with open("/sys/class/gpio/gpio304/value") as f:
              data_1 = f.read()
        if data_4 != "" and data_2 != "" and data_1 != "":        
            data = int(data_4)*4 + int(data_2)*2 + int(data_1)*1

        # set mac rov
        for element in self.TPS53667_voltage:
            if element["rov"] == self.ROV_List[data]:
                msg("ROV: {}\n".format(element["vol"]))
                os.system("i2cset -y 3 {} {} {} w".format(self.I2C_ADDR_TPS53667, self.TPS53667_ROV_ADDR, element["vol"]))

        # clear port interrupts
        for gpio in range(416, 450):
            with open("/sys/class/gpio/gpio{}/value".format(gpio)) as f:
                f.read()
        for gpio in range(320, 384):
            with open("/sys/class/gpio/gpio{}/value".format(gpio)) as f:
                f.read()

        self.enable_ipmi_maintenance_mode()

        msg("Platform hardware initialization done, set SYS_LED as stable green.\n")
        output = subprocess.check_output('iorw -r -b 0x700 -o 0x18 -l 1 -F', shell=True)
        val = output.splitlines()[0].rsplit(' ', 1)[-1]
        new_val = hex(int(val, 16) | 0x80)
        os.system("iorw -w -b 0x700 -o 0x18 -l 1 -v {} -F".format(new_val))
        output = subprocess.check_output('iorw -r -b 0x700 -o 0x1a -l 1 -F', shell=True)
        val = output.splitlines()[0].rsplit(' ', 1)[-1]
        new_val = hex(int(val, 16) & 0xf7)
        os.system("iorw -w -b 0x700 -o 0x1a -l 1 -v {} -F".format(new_val))

        msg("****** End S9500-22XST platform initializing ... ******\n")

        return True

    def enable_ipmi_maintenance_mode(self):
        ipmi_ioctl = IPMI_Ioctl()
            
        mode=ipmi_ioctl.get_ipmi_maintenance_mode()
        msg("Current IPMI_MAINTENANCE_MODE=%d\n" % (mode) )
            
        ipmi_ioctl.set_ipmi_maintenance_mode(IPMI_Ioctl.IPMI_MAINTENANCE_MODE_ON)
            
        mode=ipmi_ioctl.get_ipmi_maintenance_mode()
        msg("After IPMI_IOCTL IPMI_MAINTENANCE_MODE=%d\n" % (mode) )
