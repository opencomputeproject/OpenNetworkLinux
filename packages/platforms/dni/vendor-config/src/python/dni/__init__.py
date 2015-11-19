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
# OnlPlatform support for DNI platforms.
#
############################################################
from onl.platform.base import *

class OnlPlatformDNI(OnlPlatformBase):
    def manufacturer(self):
        return "DNI"

    def sys_oid_vendor(self):
        return ".5324"
