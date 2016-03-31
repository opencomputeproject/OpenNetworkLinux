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
# OnlPlatform support for the KVM simulation platform.
#
############################################################
from onl.platform.base import *

class OnlPlatformQEMU(OnlPlatformBase):
    def manufacturer(self):
        return "QEMU"

    def sys_oid_vendor(self):
        return ".42623"
