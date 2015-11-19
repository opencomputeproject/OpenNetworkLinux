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
# OnlPlatform support for DELL platforms.
#
############################################################
from onl.platform.base import *

class OnlPlatformDell(OnlPlatformBase):

    def manufacturer(self):
        return "Dell"

    def sys_oid_vendor(self):
        return ".674"
