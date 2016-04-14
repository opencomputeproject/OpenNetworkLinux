from onl.platform.base import *
from onl.platform.accton import *

class OnlPlatform_arm_accton_as4610_54_r0(OnlPlatformAccton):

    def model(self):
        return "AS4610-54"

    def platform(self):
        return "arm-accton-as4610-54-r0"

    def sys_oid_platform(self):
        return ".4610"

    def baseconfig(self):
        return True
