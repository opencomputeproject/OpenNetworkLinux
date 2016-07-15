###############################################################################
#
# 
#
###############################################################################
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
x86_64_alphanetworks_snx60a0_486f_INCLUDES := -I $(THIS_DIR)inc
x86_64_alphanetworks_snx60a0_486f_INTERNAL_INCLUDES := -I $(THIS_DIR)src
x86_64_alphanetworks_snx60a0_486f_DEPENDMODULE_ENTRIES := init:x86_64_alphanetworks_snx60a0_486f ucli:x86_64_alphanetworks_snx60a0_486f

