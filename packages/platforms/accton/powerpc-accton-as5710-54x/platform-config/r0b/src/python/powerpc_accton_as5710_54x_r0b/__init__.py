#!/usr/bin/python
############################################################
#
############################################################
import os
from onl.platform.base import *
from onl.platform.accton import *

class OnlPlatform_powerpc_accton_as5710_54x_r0b(OnlPlatformAccton):

    CPLDVERSION="cpldversion"

    def model(self):
        return "AS5710-54X (R0B)"

    def platform(self):
        return 'powerpc-accton-as5710-54x-r0b'

    def sys_oid_platform(self):
        return ".5710.54"

