from onl.platform.base import *
from onl.platform.wnc import *

class OnlPlatform_x86_64_wnc_rseb_w1_32_r0(OnlPlatformWNC,
                                              OnlPlatformPortConfig_32x100):
    PLATFORM='x86-64-wnc-rseb-w1-32-r0'
    MODEL="RSEB-W1-32"
    SYS_OBJECT_ID=".6819.18.6649"

    def baseconfig(self):

        os.system("modprobe ucd9000")
        os.system("modprobe jc42")
        os.system("modprobe ads7828")
        os.system("modprobe rseb-w1-32-system_cpld")
        os.system("modprobe tps53681")
        os.system("modprobe ym2651y")
        os.system("modprobe rseb-w1-32-cpu_cpld")
        os.system("modprobe pca9541-master-force")

        platform_root='/lib/platform-config/current/onl'

        self.new_i2c_devices([
                ('ads7830', 0x48, 0),
                ('ads7830', 0x49, 0),
                ('lm75', 0x4F, 0),
                ('spd', 0x50, 0),
                ('spd', 0x52, 0),
                ('tps53622', 0x60, 0),
                ('tps53622', 0x61, 0),
                ('tps53622', 0x62, 0),
                ('pca9541-mux-force', 0x78, 0),
                ('24c02', 0x56, 1),
                ('rseb_w1_32_cpu_cpld', 0x40, 1),
                ('pca9541-mux-force', 0x79, 1),
                ('ucd9090', 0x34, 2),
                ('pca9548', 0x70, 2),
                ('24c02', 0x54, 3),
                ('pca9548', 0x71, 3),
                ('pca9545', 0x70, 3),
                ('tps53681', 0x65, 4),
                ('lm75', 0x4A, 6),
                ('lm75', 0x4B, 6),
                ('lm75', 0x4C, 6),
                ('rseb_w1_32_sys_cpld', 0x66, 7),
                ('tca6424', 0x22, 12),
                ('pca9548', 0x72, 12),
                ('tca6424', 0x23, 13),
                ('pca9548', 0x73, 13),
                ('tca6424', 0x22, 14),
                ('pca9548', 0x72, 14),
                ('tca6424', 0x23, 15),
                ('pca9548', 0x73, 15),
                ('tca6424', 0x22, 16),
                ('pca9548', 0x72, 16),
                ('tca6424', 0x23, 17),
                ('pca9548', 0x73, 17),
                ('24c02', 0x50, 18),
                ('ym2651y', 0x58, 18),
                ('24c02', 0x50, 19),
                ('ym2651y', 0x58, 19),
                ('24c02', 0x52, 20),
                ('24c02', 0x55, 20),
                ('pca9548', 0x74, 3),
                ('24c02', 0x57, 72),
                ('24c02', 0x57, 73),
                ('24c02', 0x57, 74),
                ('24c02', 0x57, 75),
                ('24c02', 0x57, 76),
                ('24c02', 0x57, 77),
                ('lm75', 0x4E, 78),
                ])

        port_num = [24, 25, 26, 27, 28, 29, \
                32, 33, 34, 35, 36, 37, \
                40, 41, 42, 43, 44, 45, \
                48, 49, 50, 51, 52, 53, \
                56, 57, 58, 59, 60, 61, \
                64, 65]

        for port in port_num:
            self.new_i2c_devices([
                ('optoe1', 0x50, port),
                ])

        count = 1
        for port in port_num:
            subprocess.call( \
                    'while true; do if \
                    [ -f /sys/bus/i2c/devices/%d-0050/port_name ]; \
                    then echo port%d > \
                    /sys/bus/i2c/devices/%d-0050/port_name; exit; fi; \
                    sleep 1; done &' % (port, count, port), shell=True)
            count += 1

        count = 0
        gpio_folder = "/sys/class/gpio"
        for x in range(368, 512):
            subprocess.call('echo %d > /sys/class/gpio/export' % x, \
                    shell=True)
            if count%4 >= 2: # count == 2 or 3
                subprocess.call('echo out > %s/gpio%d/direction' % \
                        (gpio_folder, x), shell=True)
                if count%4 == 3: # count == 3
                    subprocess.call('echo 1 > %s/gpio%d/value' % \
                            (gpio_folder, x), shell=True)
            count += 1

        os.system("ln -sf %s/etc/rc.local /etc/rc.local" % platform_root)
        os.system("ln -sf %s/boot/version /boot/version" % platform_root)

        python_lib = "/usr/local/lib/python2.7/dist-packages/"
        os.system("ln -sf %s/opt/oom /opt" % platform_root)
        os.system("ln -sf %s/%s/easy-install.pth %s" % \
                (platform_root, python_lib, python_lib))
        os.system("ln -sf %s/%s/oom-0.5-py2.7.egg %s" % \
                (platform_root, python_lib, python_lib))

        os.system("ln -sf %s/opt/tools /opt" % platform_root)
        os.system("ln -sf /opt/tools/eeupdate64e /usr/sbin")

        return True
