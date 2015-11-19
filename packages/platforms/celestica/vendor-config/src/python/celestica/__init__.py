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
# OnlPlatform support for Celestica platforms.
#
############################################################
from onl.platform.base import *

class OnlPlatformCelestica(OnlPlatformBase):
    def manufacturer(self):
        return "Celestica"

    def sys_oid_vendor(self):
        return ".12290"

