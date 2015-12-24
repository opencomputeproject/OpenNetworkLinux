#!/usr/bin/python
############################################################
# <bsn.cl fy=2013 v=none>
#
#        Copyright 2013, 2014 BigSwitch Networks, Inc.
#
#
#
# </bsn.cl>
############################################################
#
# Platform Driver for the Quanta LB9
#
############################################################
from onl.platform.base import *
from onl.platform.quanta import *

class OnlPlatform_powerpc_quanta_lb9_r0(OnlPlatformQuanta):

    def model(self):
        return "LB9"

    def platform(self):
        return "powerpc-quanta-lb9-r0"

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

    def sys_oid_platform(self):
        return ".1048.1"


