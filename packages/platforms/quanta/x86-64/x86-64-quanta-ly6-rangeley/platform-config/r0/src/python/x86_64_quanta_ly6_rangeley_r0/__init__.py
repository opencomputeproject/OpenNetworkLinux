from onl.platform.base import *
from onl.platform.quanta import *

class OnlPlatform_x86_64_quanta_ly6_rangeley_r0(OnlPlatformQuanta):

    def model(self):
        return "LY6"

    def platform(self):
        return "x86-64-quanta-ly6-rangeley-r0"

    def sys_oid_platform(self):
        return ".6.1"

    def baseconfig(self):
        # fixme
        try:
            files = os.listdir("%s/etc/init.d" % self.basedir_onl())
            for file in files:
                src = "%s/etc/init.d/%s" % (self.basedir_onl(), file)
                dst = "/etc/init.d/%s" % file
                os.system("cp -f %s %s" % (src, dst))
                os.system("/usr/sbin/update-rc.d %s defaults" % file)
        except:
            pass

        # make ds1339 as default rtc
        os.system("ln -snf /dev/rtc1 /dev/rtc")
        os.system("hwclock --hctosys")

        # fixme
        # set system led to green
        sled = self.basedir_onl('sbin', 'systemled')
        if os.path.exists(sled):
            os.system(sled)

        return True
