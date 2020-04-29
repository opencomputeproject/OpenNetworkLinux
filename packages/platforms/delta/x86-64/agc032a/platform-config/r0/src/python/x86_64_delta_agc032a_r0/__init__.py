from onl.platform.base import *
from onl.platform.delta import *
import os
import subprocess

class OnlPlatform_x86_64_delta_agc032a_r0(OnlPlatformDelta,
                                              OnlPlatformPortConfig_32x400):
    PLATFORM='x86-64-delta-agc032a-r0'
    MODEL="AGC032A"
    SYS_OBJECT_ID=".032.1"


    def baseconfig(self):
        # RuntimeError: kernel module i2c_ismt could not be found.
        #  The following paths were searched:
        #    /lib/modules/4.9.75-OpenNetworkLinux/onl/delta/x86-64-delta-agc032a-r0/i2c_ismt.ko
        #    /lib/modules/4.9.75-OpenNetworkLinux/onl/delta/x86-64-delta-agc032a-r0/i2c_ismt
        #    /lib/modules/4.9.75-OpenNetworkLinux/onl/delta/x86-64-delta-agc032a/i2c_ismt.ko
        #    /lib/modules/4.9.75-OpenNetworkLinux/onl/delta/x86-64-delta-agc032a/i2c_ismt
        #    /lib/modules/4.9.75-OpenNetworkLinux/onl/delta/common/i2c_ismt.ko
        #    /lib/modules/4.9.75-OpenNetworkLinux/onl/delta/common/i2c_ismt
        #    /lib/modules/4.9.75-OpenNetworkLinux/onl/onl/common/i2c_ismt.ko
        #    /lib/modules/4.9.75-OpenNetworkLinux/onl/onl/common/i2c_ismt
        #    /lib/modules/4.9.75-OpenNetworkLinux/onl/i2c_ismt.ko
        #    /lib/modules/4.9.75-OpenNetworkLinux/onl/i2c_ismt
        #    /lib/modules/4.9.75-OpenNetworkLinux/i2c_ismt.ko
        #    /lib/modules/4.9.75-OpenNetworkLinux/i2c_ismt

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

        return ret

