#!/usr/bin/python
############################################################
# <bsn.cl fy=2013 v=none>
#
#        Copyright 2013, 2014 BigSwitch Networks, Inc.
#
# </bsn.cl>
############################################################
#
# OnlPlatform support for nxp platforms.
#
############################################################
from onl.platform.base import *

class OnlPlatformnxp(OnlPlatformBase):
    def manufacturer(self):
        return "nxp"

    def sys_oid_vendor(self):
        return ".533"
