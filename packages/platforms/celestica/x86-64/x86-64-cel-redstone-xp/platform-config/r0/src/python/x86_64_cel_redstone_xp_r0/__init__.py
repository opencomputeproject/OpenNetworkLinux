from onl.platform.base import *
from onl.platform.celestica import *

class OnlPlatform_x86_64_cel_redstone_xp_r0(OnlPlatformCelestica):

    def model(self):
        return "Redstone XP"

    def platform(self):
        return "x86-64-cel-redstone-xp-r0"

    def sys_oid_platform(self):
        return ".2060.1"

    def baseconfig(self):
        return True
