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
# OnlPlatform support for Accton platforms.
#
############################################################
from onl.platform.base import *

class OnlPlatformAccton(OnlPlatformBase):
    def manufacturer(self):
        return "Accton"

    def sys_oid_vendor(self):
        return ".259"
