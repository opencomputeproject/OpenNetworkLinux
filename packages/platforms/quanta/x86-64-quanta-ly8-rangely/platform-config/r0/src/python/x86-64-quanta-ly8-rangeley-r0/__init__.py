from onl.platform.base import *
from onl.platform.accton import *

class OnlPlatform_x86_64_quanta_ly8_rangeley_r0(OnlPlatformAccton):

    def model(self):
        return "LY8-Rangeley"

    def platform(self):
        return "x86-64-quanta-ly8-rangeley-r0"

    def sys_oid_platform(self):
        return ".8.1"

    def baseconfig(self):
        try:
            files = os.listdir("%s/etc/init.d" % self.platform_basedir())
            for file in files:
                src = "%s/etc/init.d/%s" % (self.platform_basedir(), file)
                dst = "/etc/init.d/%s" % file
                os.system("cp -f %s %s" % (src, dst))
                os.system("/usr/sbin/update-rc.d %s defaults" % file)
        except:
            pass

        # make ds1339 as default rtc
        os.system("ln -snf /dev/rtc1 /dev/rtc")
        os.system("hwclock --hctosys")

        # set system led to green
        os.system("%s/sbin/systemled green" % self.platform_basedir())

        return True
