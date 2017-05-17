#!/usr/bin/python

import subprocess
from onl.platform.base import *
from onl.platform.quanta import *

class OnlPlatform_powerpc_quanta_ly2_r0(OnlPlatformQuanta,
                                        OnlPlatformPortConfig_48x10_4x40):
    PLATFORM='powerpc-quanta-ly2-r0'
    MODEL="LY2"
    SYS_OBJECT_ID=".3048.1"

    def baseconfig(self):
        self.insmod("quanta-ly2-i2c-mux.ko")
        self.insmod("quanta-ly-hwmon.ko")
        subprocess.check_call("%s/sbin/gpio_init" % self.basedir_onl())

        fan_dir='/sys/bus/i2c/devices/4-002e/fan_dir'
        if os.path.exists(fan_dir):
            with open(fan_dir) as f:
                data = f.read()
            if data == 'front-to-back':
                platform_fancontrol="%s/etc/fancontrol" % self.basedir_onl()
            elif data == 'back-to-front':
                platform_fancontrol="%s/etc/fancontrol.b2f" % self.basedir_onl()
            else:
                sys.exit(1)
        else:
            sys.exit(1)
        FAN_CONF = '/etc/fancontrol'
        if os.path.exists(FAN_CONF):
            os.unlink(FAN_CONF)
        if os.path.exists(platform_fancontrol):
            os.symlink(platform_fancontrol, FAN_CONF)
        else:
            sys.exit(1)

        return True





