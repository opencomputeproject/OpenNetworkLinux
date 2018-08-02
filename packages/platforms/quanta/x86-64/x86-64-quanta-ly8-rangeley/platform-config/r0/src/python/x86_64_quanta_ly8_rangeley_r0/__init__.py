from onl.platform.base import *
from onl.platform.quanta import *

class OnlPlatform_x86_64_quanta_ly8_rangeley_r0(OnlPlatformQuanta,
                                                OnlPlatformPortConfig_48x10_6x40):
    PLATFORM='x86-64-quanta-ly8-rangeley-r0'
    MODEL="LY8"
    """ Define Quanta SYS_OBJECT_ID rule.

    SYS_OBJECT_ID = .xxxx.ABCC
    "xxxx" define QCT device mark. For example, LB9->1048, LY2->3048
    "A" define QCT switch series name: LB define 1, LY define 2, IX define 3
    "B" define QCT switch series number 1: For example, LB9->9, LY2->2
    "CC" define QCT switch series number 2: For example, LY2->00, LY4R->18(R is 18th english letter)
    """
    SYS_OBJECT_ID=".3048.2800"

    def baseconfig(self):
        self.insmod("emerson700")
        self.insmod("quanta_hwmon_ly_series")
        self.insmod("optoe")
        self.insmod("quanta_platform_ly8")

        # make ds1339 as default rtc
        os.system("ln -snf /dev/rtc1 /dev/rtc")
        os.system("hwclock --hctosys")

        #SFP for 1~48 port
        #QSFP for 49~52 port
        for port_number in range(1,53):
            bus_number = port_number + 31
            os.system("echo %d >/sys/bus/i2c/devices/%d-0050/port_name" % (port_number, bus_number))

        #QDB QSFP 53~54port
        os.system("echo 53 >/sys/bus/i2c/devices/88-0050/port_name")
        os.system("echo 54 >/sys/bus/i2c/devices/89-0050/port_name")

        return True
