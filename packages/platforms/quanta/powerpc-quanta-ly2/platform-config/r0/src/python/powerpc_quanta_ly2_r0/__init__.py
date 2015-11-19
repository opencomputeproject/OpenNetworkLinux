#!/usr/bin/python
############################################################
#
############################################################
import subprocess
from onl.platform.base import *
from onl.platform.quanta import *

class OnlPlatform_powerpc_quanta_ly2_r0(OnlPlatformQuanta):

    def model(self):
        return "LY2"

    def platform(self):
        return "powerpc-quanta-ly2-r0"

    def baseconfig(self):
        fan_dir='/sys/devices/soc.0/ffe03000.i2c/i2c-0/i2c-4/4-002e/fan_dir'
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

        subprocess.call("%s/sbin/gpio_init" % self.basedir_onl())
        return True

    def sys_oid_platform(self):
        return ".3048.1"



