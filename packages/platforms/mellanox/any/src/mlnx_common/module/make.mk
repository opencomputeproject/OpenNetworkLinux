###############################################################################
#
#
#
###############################################################################
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
mlnx_common_INCLUDES := -I $(THIS_DIR)inc
mlnx_common_INTERNAL_INCLUDES := -I $(THIS_DIR)src
mlnx_common_DEPENDMODULE_ENTRIES := init:mlnx_common ucli:mlnx_common

#
# This is a real bug:
#
# mlnx_common_fani.c:310:61: error: division 'sizeof (int *) / sizeof (int)' does not compute the number of array elements [-Werror=sizeof-pointer-div]
#
mlnx_common_CFLAGS := -Wno-sizeof-pointer-div
