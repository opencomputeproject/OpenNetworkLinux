###############################################################################
#
# 
#
###############################################################################
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
x86_64_wnc_rseb_w1_32_INCLUDES := -I $(THIS_DIR)inc
x86_64_wnc_rseb_w1_32_INTERNAL_INCLUDES := -I $(THIS_DIR)src
x86_64_wnc_rseb_w1_32_DEPENDMODULE_ENTRIES := init:x86_64_wnc_rseb_w1_32 ucli:x86_64_wnc_rseb_w1_32

