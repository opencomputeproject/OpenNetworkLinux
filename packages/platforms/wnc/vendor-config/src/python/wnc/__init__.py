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
# OnlPlatform support for WNC platforms.
#
############################################################
from onl.platform.base import *

class OnlPlatformWNC(OnlPlatformBase):
    def manufacturer(self):
        return "WNC"

    def sys_oid_vendor(self):
        return ".15756"
