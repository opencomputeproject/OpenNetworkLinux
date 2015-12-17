from onl.platform.base import *
from onl.vendor.accton import *

class OnlPlatform_x86_64_accton_wedge_16x_r0(OpenNetworkPlatformAccton):

    def model(self):
        return "Wedge-16X"

    def platform(self):
        return "x86-64-accton-wedge-16x-r0"

    def baseconfig(self):
        return True

    def sys_oid_platform(self):
        return ".16.1"
