from onl.platform.base import *
from onl.vendor.accton import *

class OpenNetworkPlatformImplementation(OpenNetworkPlatformAccton):

    def model(self):
        return "Wedge"

    def platform(self):
        return "x86-64-accton-wedge-16x-r0"

    def _plat_info_dict(self):
        return {
            platinfo.LAG_COMPONENT_MAX : 24,
            platinfo.PORT_COUNT : 16,
            platinfo.ENHANCED_HASHING : True,
            platinfo.SYMMETRIC_HASHING : True,
            }

    def sys_init(self):
        pass

    def sys_oid_platform(self):
        # FIXME
        return ".16.1"

    def baseconfig(self):
        return os.system(os.path.join(self.platform_basedir(), "boot", "x86-64-accton-wedge-r0-devices.sh")) == 0

if __name__ == "__main__":
    print OpenNetworkPlatformImplementation()
