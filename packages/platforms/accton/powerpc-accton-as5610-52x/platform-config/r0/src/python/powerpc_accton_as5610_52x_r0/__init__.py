#!/usr/bin/python
############################################################
#
#
#
############################################################
from onl.platform.base import *
from onl.platform.accton import *

class OnlPlatform_powerpc_accton_as5610_52x_r0(OnlPlatformAccton):

    onie_base_address = "0xeff70000"

    def model(self):
        return "AS5610-52X"

    def platform(self):
        return 'powerpc-accton-as5610-52x-r0'

    def sys_oid_platform(self):
        return ".5610.52"


