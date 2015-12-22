from onl.platform.base import *
from onl.platform.accton import *

class OnlPlatform_x86_64_accton_as7512_32x_r0(OnlPlatformAccton):

    def model(self):
        return "AS7512-32X"

    def platform(self):
        return "x86-64-accton-as7512-32x-r0"

    def sys_oid_platform(self):
        return ".7512.32"

    def baseconfig(self):
        return True
