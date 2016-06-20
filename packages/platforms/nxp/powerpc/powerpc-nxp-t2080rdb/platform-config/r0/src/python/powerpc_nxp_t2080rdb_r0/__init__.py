#!/usr/bin/python
############################################################
#
#
#
############################################################
from onl.platform.base import *

class OnlPlatform_powerpc_nxp_t2080rdb_r0(OnlPlatformnxp):

    CPLDVERSION="cpldversion"

    def model(self):
        return "T2080RDB"

    def platform(self):
        return 'powerpc-nxp-t2080rdb-r0'

    def sys_oid_platform(self):
        return ".t2080"

