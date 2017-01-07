from onl.platform.base import *
from onl.platform.nxp import *

class OnlPlatform_arm64_nxp_ls2088ardb_r1(OnlPlatformNxp,
                                          OnlPlatformPortConfig_8x1_8x10):
    PLATFORM='arm64-nxp-ls2088ardb-r1'
    MODEL="LS2088ARDB"
    SYS_OBJECT_ID=".2088"
