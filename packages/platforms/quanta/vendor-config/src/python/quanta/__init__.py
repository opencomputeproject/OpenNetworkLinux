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
# OnlPlatform support for Quanta platforms.
#
############################################################
from onl.platform.base import *

class OnlPlatformQuanta(OnlPlatformBase):

    def manufacturer(self):
        return "Quanta"

    def sys_oid_vendor(self):
        return ".7244"
