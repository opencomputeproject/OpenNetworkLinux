from onl.platform.base import *
from onl.platform.delta import *
import os.path
import subprocess

class OnlPlatform_x86_64_delta_agv424_r0(OnlPlatformDelta,
                                              OnlPlatformPortConfig_24x25_4x100):
    PLATFORM='x86-64-delta-agv424-r0'
    MODEL="AGV424"
    SYS_OBJECT_ID=".032.0"


    def baseconfig(self):
        # kernel module i2c_ismt realpath
        # /lib/modules/4.9.75-OpenNetworkLinux/kernel/drivers/i2c/busses
        ret = False

        findcmd = "find /lib/modules/%s/kernel -iname 'i2c*ismt.ko'" % os.uname()[2]
        d = subprocess.check_output(findcmd, shell=True).strip()

        if (os.path.exists(d)):
            subprocess.check_call("insmod %s" % d, shell=True);
            ret = True
        else:
            msg("Kernel module could not be found.\n")
            ret = False

        if ret:
            # self.insmod('i2c_ismt')
            cmd = "echo 'blacklist delta_i2c_ismt' > /etc/modprobe.d/blacklist.conf"
            subprocess.check_call(cmd, shell=True);
        return True

