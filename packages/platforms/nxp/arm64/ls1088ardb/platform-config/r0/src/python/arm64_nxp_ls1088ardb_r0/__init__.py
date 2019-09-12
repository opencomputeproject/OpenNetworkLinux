#!/usr/bin/python

from onl.platform.base import *
from onl.platform.nxp import *

class OnlPlatform_arm64_nxp_ls1088ardb_r0(OnlPlatformNxp,
                                          OnlPlatformPortConfig_8x1_8x10):
    PLATFORM='arm64-nxp-ls1088ardb-r0'
    MODEL="LS1088ARDB"
    SYS_OBJECT_ID=".1088"
