from onl.platform.base import *
from onl.platform.nxp import *

class OnlPlatform_arm64_nxp_ls2080ardb_r0(OnlPlatformNxp,
                                          OnlPlatformPortConfig_8x1_8x10):
    PLATFORM='arm64-nxp-ls2080ardb-r0'
    MODEL="LS2080ARDB"
    SYS_OBJECT_ID=".2080"
