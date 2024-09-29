import subprocess
import time
from onl.platform.base import *
from onl.platform.celestica import *
import tempfile
import subprocess
import re

sfp_quantity = 2
osfp_quantity = 32
fan_quantity = 7
sfp_i2c_start_bus = 2
osfp_i2c_start_bus = 15
fan_i2c_start_bus = 63


class OnlPlatform_x86_64_cls_ds4101_r0(
    OnlPlatformCelestica, OnlPlatformPortConfig_32x800_2x10
):
    PLATFORM = "x86-64-cls-ds4101-r0"
    MODEL = "DS4101"
    SYS_OBJECT_ID = ".2060.1"

    @staticmethod
    def send_cmd(cmd):
        """
        send command
        :param cmd: command
        :return: command status and response
        """
        try:
            data = subprocess.check_output(cmd, shell=True,
                                           universal_newlines=True, stderr=subprocess.STDOUT)
            sta = True
        except subprocess.CalledProcessError as ex:
            data = ex.output
            sta = False
        if data[-1:] == '\n':
            data = data[:-1]
        return sta, data

    def get_boot_partition(self):
        """
        get the onl boot partition name.
        If ssd interface protocol is NVME, partition name is /dev/nvme0n1p3
        If ssd interface protocol is sata, partition name is /dev/sda3
        :return: /dev/nvme0n1p3 or /dev/sda3
        """
        partition_status, partition = self.send_cmd("blkid | grep 'ONL-BOOT' | awk -F ':' '{print $1}'")
        if partition_status:
            partition = str(partition).strip()
        else:
            partition = "/dev/nvme0n1p3" if os.path.exists("/sys/block/nvme0n1/removable") else "/dev/sda3"
        return partition

    def register_hwdevice_multi_times(self, driver, bus, addr, times, node):
        """
        Register the hwmon device multiple times to fix loading issue
        Param:
            string: driver name like "tdps2000"
            int:    bus, decimal format like 11
            hex:    addr, hex format like 0x59
            string: node like 'fan1_input'
        Returns:
            bool:   true for success , false for fail
        """
        count = 0
        while count < times:
            self.new_i2c_device(driver, addr, bus)
            ret = os.system("ls /sys/bus/i2c/devices/i2c-%d/%d-%4.4x/hwmon/hwmon*/ | grep %s > /dev/null" % (bus, bus, addr, node))
            if ret == 0:
                return True
            os.system("echo 0x%4.4x > /sys/bus/i2c/devices/i2c-%d/delete_device" % (addr, bus))
            count = count + 1
        return False

    def baseconfig(self):
        # disable i2c-imc autoload because there's a conflict if BMC exists
        os.system("echo %s > /etc/hostname" % self.MODEL)
        os.system("echo 'blacklist i2c-imc' > /etc/modprobe.d/blacklist.conf")

        # Install platform drivers
        print("Initialize and Install the driver here")
        self.insmod("fpga-device.ko")
        self.insmod("fpga-i2c-xiic.ko")
        self.insmod("fpga-sys.ko")
        self.insmod("fpga-xcvr.ko")
        self.insmod("i2c-xcvr.ko")
        self.insmod("lpc-basecpld.ko")
        self.insmod("watch_dog.ko")
        os.system("rmmod at24")
        self.insmod("at24.ko")

        # Add devices
        os.system("modprobe i2c-ismt")
        os.system("modprobe tpm_i2c_nuvoton")
        os.system("modprobe mpl3115")
        os.system("modprobe optoe")
        ## Support to use internal USB to update firmwares
        os.system("modprobe usbnet")
        os.system("modprobe cdc_ether")
        os.system("modprobe mce-inject")
        self.new_i2c_device("24c64", 0x56, 0)
        self.new_i2c_device("pca9548", 0x71, 1)
        self.new_i2c_device("pca9548", 0x73, 1)
        self.new_i2c_device("pca9548", 0x70, 1)
        self.new_i2c_device("pca9548", 0x72, 1)
        os.system("echo -2 > /sys/bus/i2c/drivers/pca954x/1-0070/idle_state")
        os.system("echo -2 > /sys/bus/i2c/drivers/pca954x/1-0071/idle_state")
        os.system("echo -2 > /sys/bus/i2c/drivers/pca954x/1-0072/idle_state")
        os.system("echo -2 > /sys/bus/i2c/drivers/pca954x/1-0073/idle_state")

        print("Running Watchdog")
        os.system("python /usr/lib/python2.7/dist-packages/onl/platform/x86_64_cls_ds4101_r0/wdt/wdt_control.py &")
        # check BMC present status
        os.system("echo 0xA108 > /sys/bus/platform/devices/sys_cpld/getreg")
        result = os.popen("cat /sys/bus/platform/devices/sys_cpld/getreg").read()
        if "0x01" in result:
            print("BMC is absent")

            self.insmod("platform-fan.ko")
            self.insmod("platform-psu.ko")

            self.insmod("mp2975.ko")
            self.insmod("mp2880.ko")
            self.insmod("mp5023.ko")

            self.new_i2c_device("pca9548", 0x70, 7)
            self.new_i2c_device("pca9548", 0x70, 10)
            self.new_i2c_device("pca9548", 0x77, 11)
            os.system("modprobe i2c-imc")
            os.system("modprobe jc42")

            # deselect pca9548 channel.
            os.system("echo -2 > /sys/bus/i2c/drivers/pca954x/7-0070/idle_state")
            os.system("echo -2 > /sys/bus/i2c/drivers/pca954x/10-0070/idle_state")
            os.system("echo -2 > /sys/bus/i2c/drivers/pca954x/11-0077/idle_state")

            self.new_i2c_device("platform_fan", 0x0d, 11)

            # reinstall tdps2000 driver since it may fail to create fanx_input node
            # if there's error 0x80 in 0x7e register when driver probes
            ret = self.register_hwdevice_multi_times('tdps2000', 47, 0x58, 6, 'fan1_input')
            if ret is False:
                print("*** # Fail to register tdps2000 on 47-0058, please check...")
            ret = self.register_hwdevice_multi_times('tdps2000', 48, 0x59, 6, 'fan1_input')
            if ret is False:
                print("*** # Fail to register tdps2000 on 48-0059, please check...")
            self.new_i2c_device("24c02", 0x50, 47)
            self.new_i2c_device("24c02", 0x51, 48)

            self.new_i2c_device("ucd90120", 0x35, 8)
            self.new_i2c_device("ucd90160", 0x34, 8)

            self.new_i2c_device("mp2975", 0x70, 9)
            self.new_i2c_device("mp2975", 0x76, 9)
            self.new_i2c_device("mp2880", 0x20, 9)
            self.new_i2c_device("mp5023", 0x40, 8)

            # EEprom
            self.new_i2c_device("24c64", 0x50, 4)
            self.new_i2c_device("24c64", 0x50, 5)
            self.new_i2c_device("24c64", 0x57, 5)
            self.new_i2c_device("24c64", 0x50, 70)

            for fan_index in range(fan_quantity):
                fan_bus = fan_index + fan_i2c_start_bus
                self.new_i2c_device("24c64", 0x50, fan_bus)

            self.new_i2c_device("lm75b", 0x4e, 10)
            self.new_i2c_device("lm75b", 0x48, 55)
            self.new_i2c_device("lm75b", 0x49, 56)
            self.new_i2c_device("lm75b", 0x4a, 57)
            self.new_i2c_device("lm75b", 0x4b, 58)
            self.new_i2c_device("lm75b", 0x4c, 59)
            self.new_i2c_device("lm75b", 0x4d, 60)
            self.new_i2c_device("mpl3115", 0x60, 61)
            self.new_i2c_device("lm75", 0x48, 70)
            self.new_i2c_device("lm75", 0x49, 70)

        # Instantiate XCVR devices; one device manage 4 ports, totally 32 ports from 0x20 to 0x27
        for xcvr in range(0x20, 0x24):
            self.new_i2c_device("xcvr", xcvr, 13)

        for xcvr in range(0x24, 0x28):
            self.new_i2c_device("xcvr", xcvr, 14)

        # Initialize SFPs & QSFPs name
        for actual_i2c_port in range(
            sfp_i2c_start_bus, sfp_i2c_start_bus + sfp_quantity
        ):
            self.new_i2c_device("optoe2", 0x50, actual_i2c_port)
            port_number = actual_i2c_port - (sfp_i2c_start_bus - 1)
            os.system(
                "echo 'SFP{1}' > /sys/bus/i2c/devices/i2c-{0}/{0}-0050/port_name".format(
                    actual_i2c_port, port_number
                )
            )

        for actual_i2c_port in range(
            osfp_i2c_start_bus, osfp_i2c_start_bus + osfp_quantity
        ):
            self.new_i2c_device("optoe1", 0x50, actual_i2c_port)
            oport_number = actual_i2c_port - (osfp_i2c_start_bus - 1)
            os.system(
                "echo 'OSFP{1}' > /sys/bus/i2c/devices/i2c-{0}/{0}-0050/port_name".format(
                    actual_i2c_port, oport_number
                )
            )

        os.system("echo 4 > /proc/sys/kernel/printk")

        return True
