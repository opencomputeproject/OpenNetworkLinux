from onl.platform.base import *
from onl.platform.accton import *

class OnlPlatform_x86_64_accton_dcg8510_32d_r0(OnlPlatformAccton,
                                                OnlPlatformPortConfig_32x400):
    MODEL="dcg-8510-32d"
    PLATFORM="x86-64-accton-dcg8510-32d-r0"
    SYS_OBJECT_ID=".8510.32.1"

    def baseconfig(self):
        # rootfs overlay
        stamp_dirname = os.path.dirname(__file__)

        #rootfs overlay
        if not os.path.exists(stamp_dirname + "/rootfs_overlay.stamp"):
            os.system("cp -r " + stamp_dirname + "/rootfs_overlay/* /")
            os.system("sync /")
            os.system("touch " + stamp_dirname + "/rootfs_overlay.stamp")


        self.insmod('s3ip/switch_s3ip_switch')
        self.insmod('s3ip/switch_s3ip_bmc')
        self.insmod('s3ip/switch_s3ip_cpld')
        self.insmod('s3ip/switch_s3ip_fan')
        self.insmod('s3ip/switch_s3ip_fpga')
        self.insmod('s3ip/switch_s3ip_psu')
        self.insmod('s3ip/switch_s3ip_sensor')
        self.insmod('s3ip/switch_s3ip_syseeprom')
        self.insmod('s3ip/switch_s3ip_sysled')
        self.insmod('s3ip/switch_s3ip_transceiver')
        self.insmod('s3ip/switch_s3ip_watchdog')
        self.insmod('third_party/switch_system_fpga')
        self.insmod('third_party/switch_optoe')
        self.insmod('third_party/switch_at24')
        self.insmod('third_party/switch_pmbus_core')
        self.insmod('third_party/switch_lm75')
        self.insmod('third_party/sysfs_ipmi')
        self.insmod('third_party/switch_i2c_cpld')
        self.insmod('third_party/switch_system_cpld')
        self.insmod('switch_mb_driver')
        self.insmod('switch_cpld_driver')
        self.insmod('switch_fpga_driver')
        self.insmod('switch_bmc_driver')
        self.insmod('switch_transceiver_driver')
        self.insmod('switch_led_driver')
        self.insmod('switch_wdt_driver')
        self.insmod('switch_fan_driver')
        self.insmod('switch_psu_driver')
        self.insmod('third_party/switch_coretemp')
        self.insmod('switch_sensor_driver')        
        MODULE_FOLDER="/lib/modules/4.19.81-OpenNetworkLinux/onl/accton/x86-64-accton-dcg8510-32d"
        
        self.new_i2c_devices([
             ('24c64', 0x57, 0),
             ('port_cpld1', 0x62, 634),
             ('port_cpld2', 0x64, 635),
             ('switch_optoe3', 0x50, 601),
             ('switch_optoe3', 0x50, 602),
             ('switch_optoe3', 0x50, 603),
             ('switch_optoe3', 0x50, 604),
             ('switch_optoe3', 0x50, 605),
             ('switch_optoe3', 0x50, 606),
             ('switch_optoe3', 0x50, 607),
             ('switch_optoe3', 0x50, 608),
             ('switch_optoe3', 0x50, 609),
             ('switch_optoe3', 0x50, 610),
             ('switch_optoe3', 0x50, 611),
             ('switch_optoe3', 0x50, 612),
             ('switch_optoe3', 0x50, 613),
             ('switch_optoe3', 0x50, 614),
             ('switch_optoe3', 0x50, 615),
             ('switch_optoe3', 0x50, 616),
             ('switch_optoe3', 0x50, 617),
             ('switch_optoe3', 0x50, 618),
             ('switch_optoe3', 0x50, 619),
             ('switch_optoe3', 0x50, 620),
             ('switch_optoe3', 0x50, 621),
             ('switch_optoe3', 0x50, 622),
             ('switch_optoe3', 0x50, 623),
             ('switch_optoe3', 0x50, 624),
             ('switch_optoe3', 0x50, 625),
             ('switch_optoe3', 0x50, 626),
             ('switch_optoe3', 0x50, 627),
             ('switch_optoe3', 0x50, 628),
             ('switch_optoe3', 0x50, 629),
             ('switch_optoe3', 0x50, 630),
             ('switch_optoe3', 0x50, 631),
             ('switch_optoe3', 0x50, 632),             
                               ])
        commands = [ 
	        "ifconfig usb0 up",
        ]
        #for cmd in commands:
        #    process = subprocess.call(cmd, shell=True)
        return True
