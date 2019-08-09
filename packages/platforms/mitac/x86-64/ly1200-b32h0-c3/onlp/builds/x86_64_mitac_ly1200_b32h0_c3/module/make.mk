###############################################################################
#
# 
#
###############################################################################
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
x86_64_mitac_ly1200_b32h0_c3_INCLUDES := -I $(THIS_DIR)inc
x86_64_mitac_ly1200_b32h0_c3_INTERNAL_INCLUDES := -I $(THIS_DIR)src
x86_64_mitac_ly1200_b32h0_c3_DEPENDMODULE_ENTRIES := init:x86_64_mitac_ly1200_b32h0_c3 ucli:x86_64_mitac_ly1200_b32h0_c3

