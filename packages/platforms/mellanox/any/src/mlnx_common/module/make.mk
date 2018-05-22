###############################################################################
#
#
#
###############################################################################
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
mlnx_common_INCLUDES := -I $(THIS_DIR)inc
mlnx_common_INTERNAL_INCLUDES := -I $(THIS_DIR)src
mlnx_common_DEPENDMODULE_ENTRIES := init:mlnx_common ucli:mlnx_common
