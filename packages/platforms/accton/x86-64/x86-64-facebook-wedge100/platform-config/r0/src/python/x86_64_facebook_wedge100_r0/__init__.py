from onl.platform.base import *
from onl.vendor.accton import *

class OnlPlatform_x86_64_facebook_wedge_100_r0(OpenNetworkPlatformAccton):

    def model(self):
        return "Wedge-100"

    def platform(self):
        return "x86-64-facebook-wedge100-r0"

    def baseconfig(self):
        return True

    def sys_oid_platform(self):
        return ".100.1"
