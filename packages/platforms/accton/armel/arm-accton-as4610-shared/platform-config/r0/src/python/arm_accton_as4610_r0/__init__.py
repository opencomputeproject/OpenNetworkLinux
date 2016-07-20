from onl.platform.base import *
from onl.platform.accton import *

class OnlPlatform_arm_accton_as4610_30_r0(OnlPlatformAccton,
                                          OnlPlatformPortConfig_24x1_4x10):
    PLATFORM='arm-accton-as4610-30-r0'
    MODEL="AS4610-30"
    SYS_OBJECT_ID=".4610"

class OnlPlatform_arm_accton_as4610_54_r0(OnlPlatformAccton,
                                          OnlPlatformPortConfig_48x1_4x10):
    PLATFORM='arm-accton-as4610-54-r0'
    MODEL="AS4610-54"
    SYS_OBJECT_ID=".4610"

