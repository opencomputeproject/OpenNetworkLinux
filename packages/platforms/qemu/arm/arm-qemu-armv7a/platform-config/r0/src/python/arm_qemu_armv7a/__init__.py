from onl.platform.base import *
from onl.platform.qemu import *

class OnlPlatform_arm_qemu_armv7a_r0(OnlPlatformQEMU):

    def model(self):
        return "QEMU ARMv7a"

    def platform(self):
        return "arm-qemu-armv7a"

    def sys_oid_platform(self):
        return ".2"

    def baseconfig(self):
        return True
