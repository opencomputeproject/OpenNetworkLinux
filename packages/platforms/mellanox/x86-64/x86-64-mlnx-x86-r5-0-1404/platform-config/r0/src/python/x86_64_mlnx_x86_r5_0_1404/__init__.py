from onl.platform.base import *
from onl.platform.mellanox import *

class OnlPlatform_x86_64_mlnx_x86_r5_0_1404(OnlPlatformMellanox,
                                               OnlPlatformPortConfig_32x100):
    MODEL="SN2700"
    PLATFORM="x86-64-mlnx-x86-r5-0-1404"
    SYS_OBJECT_ID=".2700.1"
