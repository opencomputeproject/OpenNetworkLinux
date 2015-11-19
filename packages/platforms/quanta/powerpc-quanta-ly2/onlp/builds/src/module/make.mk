###############################################################################
#
# 
#
###############################################################################
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
powerpc_quanta_ly2_INCLUDES := -I $(THIS_DIR)inc
powerpc_quanta_ly2_INTERNAL_INCLUDES := -I $(THIS_DIR)src
powerpc_quanta_ly2_DEPENDMODULE_ENTRIES := init:powerpc_quanta_ly2 ucli:powerpc_quanta_ly2

