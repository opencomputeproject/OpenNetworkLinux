from onl.platform.base import *
from onl.platform.celestica import *


class OnlPlatform_x86_64_cel_silverstone_v2_r0(
    OnlPlatformCelestica, OnlPlatformPortConfig_32x400_2x10
):
    PLATFORM = "x86-64-cel-silverstone-v2-r0"
    MODEL = "silverstone-v2"
    SYS_OBJECT_ID = ".2060.1"

    def baseconfig(self):
        # rootfs overlay
        stamp_dirname = os.path.dirname(__file__)

        if not os.path.exists(stamp_dirname + "/rootfs_overlay.stamp"):
            os.system("cp -r " + stamp_dirname + "/rootfs_overlay/* /")
            os.system("sync /")
            os.system("touch " + stamp_dirname + "/rootfs_overlay.stamp")

        onlp_interval_time = 30  # second
        file_path = "/var/opt/interval_time.txt"

        print("Initialize Silverstone-v2 driver")

        qsfp_qty = 32
        sfp_qty = 2
        pca_qty = 4
        fan_qty = 7
        bus_offset = 11
        qsfp_bus_map = [
            27,
            28,
            25,
            26,
            31,
            30,
            48,
            29,
            45,
            44,
            47,
            43,
            42,
            46,
            32,
            41,
            34,
            40,
            37,
            33,
            38,
            35,
            22,
            39,
            36,
            23,
            17,
            24,
            19,
            21,
            20,
            18,
        ]

        self.insmod("fpga_system.ko")
        self.insmod("fpga_device.ko")
        self.insmod("fpga_i2c_ocores.ko")
        self.insmod("i2c_switchcpld.ko")
        self.insmod("fpga_xcvr.ko")
        self.insmod("lpc_basecpld.ko")
        self.insmod("watch_dog.ko")

        # Add device
        os.system("modprobe optoe")

        # tlv eeprom device
        self.new_i2c_device("24c64", 0x56, 0)

        # PCA9548
        self.new_i2c_device("pca9548", 0x70, 10)
        self.new_i2c_device("pca9548", 0x71, 10)
        self.new_i2c_device("pca9548", 0x72, 10)
        self.new_i2c_device("pca9548", 0x73, 10)

        # Switch CPLD
        self.new_i2c_device("switchboard", 0x30, 9)
        self.new_i2c_device("switchboard", 0x31, 9)

        # transceiver device
        for sfp_index in range(sfp_qty):
            sfp_bus = sfp_index + bus_offset
            self.new_i2c_device("optoe2", 0x50, sfp_bus)
            sfp_cmd = "echo 'SFP%d' > /sys/bus/i2c/devices/%d-0050/port_name" % (
                sfp_index + 1,
                sfp_bus,
            )
            os.system(sfp_cmd)

        for qsfp_index in range(qsfp_qty):
            self.new_i2c_device("optoe1", 0x50, qsfp_bus_map[qsfp_index])
            qsfp_cmd = "echo 'QSFP%d' > /sys/bus/i2c/devices/%d-0050/port_name" % (
                qsfp_index + 1,
                qsfp_bus_map[qsfp_index],
            )
            os.system(qsfp_cmd)

        # disconnect the idle,set it to -2.
        for pca_index in range(pca_qty):
            pca_cmd = "echo -2 > /sys/bus/i2c/devices/10-007%d/idle_state" % pca_index
            os.system(pca_cmd)

        os.system("echo '3' > /proc/sys/kernel/printk")

        if os.path.exists(file_path):
            pass
        else:
            with open(file_path, "w") as f:
                f.write("{0}\r\n".format(onlp_interval_time))
            f.close()

        # Prevent ONL take a long waiting time at Waiting for /dev to be fully populated... step when boot-up
        rulefilepath = "/etc/udev/rules.d/70-persistent-net.rules"
        if os.path.exists(rulefilepath):
            os.system("rm {0}".format(rulefilepath))
        else:
            pass

        # check BMC present status
        os.system("echo 0xA108 > /sys/bus/platform/devices/sys_cpld/getreg")
        result = os.popen("cat /sys/bus/platform/devices/sys_cpld/getreg").read()

        if "0x00" in result:
            print("BMC is present")

            # initialize onlp cache files
            print("Initialize ONLP Cache files")
            os.system(
                "ipmitool fru > /tmp/onlp-fru-cache.tmp; sync; rm -f /tmp/onlp-fru-cache.txt; mv /tmp/onlp-fru-cache.tmp /tmp/onlp-fru-cache.txt"
            )
            os.system(
                "ipmitool sensor list > /tmp/onlp-sensor-list-cache.tmp; sync; rm -f /tmp/onlp-sensor-list-cache.txt; mv /tmp/onlp-sensor-list-cache.tmp /tmp/onlp-sensor-list-cache.txt"
            )

        elif "0x01" in result:
            print("BMC is absent")

            self.insmod("platform_fan.ko")
            self.insmod("platform_psu.ko")
            self.insmod("mp2975.ko")
            self.insmod("tps536c7.ko")
            self.insmod("max31730.ko")

            os.system("modprobe lm75")
            os.system("modprobe ucd9000")

            self.new_i2c_device("platform_fan", 0x0d, 8)

            # EEprom
            self.new_i2c_device("24c64", 0x50, 2)
            self.new_i2c_device("24c64", 0x57, 2)
            self.new_i2c_device("24c64", 0x50, 5)

            self.new_i2c_device("ucd90120", 0x34, 3)
            self.new_i2c_device("ucd90120", 0x35, 3)
            self.new_i2c_device("tps536c7", 0x6c, 3)

            self.new_i2c_device("mp2975", 0x76, 4)
            self.new_i2c_device("mp2975", 0x7b, 4)
            self.new_i2c_device("mp2975", 0x70, 4)

            self.new_i2c_device("lm75", 0x48, 7)
            self.new_i2c_device("lm75", 0x49, 7)

            self.new_i2c_device("max31730", 0x4c, 15)
            self.new_i2c_device("max31730", 0x1e, 15)

            self.new_i2c_device("pca9548", 0x77, 8)  # FAN 49-56
            self.new_i2c_device("pca9548", 0x70, 6)  # PSU 57-64

            # disconnect the idle,set it to -2.
            fan_cmd = "echo -2 > /sys/bus/i2c/devices/8-0077/idle_state"
            os.system(fan_cmd)

            for fan_index in range(fan_qty):
                fan_bus = fan_index + 49
                self.new_i2c_device("24c02", 0x50, fan_bus)
            self.new_i2c_device("24c64", 0x50, 56)
            self.new_i2c_device("24c02", 0x50, 57)
            self.new_i2c_device("platform_psu", 0x58, 57)
            self.new_i2c_device("24c02", 0x51, 58)
            self.new_i2c_device("platform_psu", 0x59, 58)

            self.new_i2c_device("lm75", 0x48, 56)
            self.new_i2c_device("lm75", 0x49, 56)

            # I2C_IMC for DIMM Temperature
            os.system("modprobe jc42")
            os.system("modprobe i2c-imc")
            
        print("Running Watchdog")
        os.system("python /usr/lib/python2.7/dist-packages/onl/platform/x86_64_cel_silverstone_v2_r0/wdt/wdt_control.py &")


        return True
