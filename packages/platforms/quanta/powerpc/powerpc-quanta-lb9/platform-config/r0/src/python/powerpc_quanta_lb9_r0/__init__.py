#!/usr/bin/python

from onl.platform.base import *
from onl.platform.quanta import *

class OnlPlatform_powerpc_quanta_lb9_r0(OnlPlatformQuanta,
                                        OnlPlatformPortConfig_48x1_4x10):
    PLATFORM='powerpc-quanta-lb9-r0'
    MODEL="LB9"
    SYS_OBJECT_ID=".1048.1"

    def baseconfig(self):
        platform_fancontrol="%s/etc/fancontrol" % self.basedir_onl()
        FAN_CONF = '/etc/fancontrol'
        if os.path.exists(FAN_CONF):
            os.unlink(FAN_CONF)
        if os.path.exists(platform_fancontrol):
            os.symlink(platform_fancontrol, FAN_CONF)
        else:
            sys.exit(1)

        return True



