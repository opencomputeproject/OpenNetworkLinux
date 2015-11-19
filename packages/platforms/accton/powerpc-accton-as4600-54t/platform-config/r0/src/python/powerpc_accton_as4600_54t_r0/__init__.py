#!/usr/bin/python
############################################################
#
#
#
############################################################
from onl.platform.base import *
from onl.platform.accton import *

class OnlPlatform_powerpc_accton_as4600_54t_r0(OnlPlatformAccton):

    def model(self):
        return 'AS4600-54T'

    def platform(self):
        return 'powerpc-accton-as4600-54t-r0'

    def sys_oid_platform(self):
        return ".4600.54"

