from onl.platform.base import *
from onl.platform.mitac import *

class OnlPlatform_x86_64_mitac_ly1200_b32h0_c3_r0(OnlPlatformMiTAC,
                                              OnlPlatformPortConfig_32x100):
    PLATFORM='x86-64-mitac-ly1200-b32h0-c3-r0'
    MODEL="LY1200-B32H0-C3"
    SYS_OBJECT_ID=".1200.320"

    def baseconfig(self):

        for m in [ 'gpe', 'system_cpld', 'master_cpld', 'slave_cpld', 'cb_i2c', 'sb_i2c', 'pb_i2c', 'fb_i2c', 'fb_module_i2c', 'fse000' ]:
            self.insmod("x86-64-mitac-ly1200-b32h0-c3-%s" % m)

        platform_root='/lib/platform-config/current/onl'
        os.system("ln -sf %s/etc/watchdog.conf /etc/watchdog.conf" % platform_root)
        os.system("ln -sf %s/etc/default/watchdog /etc/default/watchdog" % platform_root)

        return True
