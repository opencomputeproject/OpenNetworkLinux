from onl.platform.base import *
from onl.platform.accton import *

class OnlPlatform_x86_64_accton_as5812_54x_r0(OnlPlatformAccton):


    def model(self):
        return "AS5812-54X"

    def platform(self):
        return "x86-64-accton-as5812-54x-r0"

    def sys_init(self):
        pass

    def sys_oid_platform(self):
        return ".5812.54"

    def baseconfig(self):
        return os.system(os.path.join(self.platform_basedir(), "boot", "x86-64-accton-as5812-54x-r0-devices.sh")) == 0
        return True
