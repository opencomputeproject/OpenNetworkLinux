#!/usr/bin/python
############################################################
#
############################################################
import os
from onl.platform.base import *
from onl.platform.accton import *

class OnlPlatform_powerpc_accton_as5710_54x_r0b(OnlPlatformAccton,
                                                OnlPlatformPortConfig_48x10_6x40):
    PLATFORM='powerpc-accton-as5710-54x-r0b'
    MODEL='AS5710-54X (R0B)'
    SYS_OBJECT_ID='.5710.54'

