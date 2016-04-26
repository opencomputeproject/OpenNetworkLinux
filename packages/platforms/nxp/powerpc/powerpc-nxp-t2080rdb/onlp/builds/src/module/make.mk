###############################################################################
#
# 
#
###############################################################################
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
powerpc_nxp_t2080rdb_INCLUDES := -I $(THIS_DIR)inc
powerpc_nxp_t2080rdb_INTERNAL_INCLUDES := -I $(THIS_DIR)src
powerpc_nxp_t2080rdb_DEPENDMODULE_ENTRIES := init:powerpc_nxp_t2080rdb ucli:powerpc_nxp_t2080rdb

