###############################################################################
#
# 
#
###############################################################################
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
x86_64_alphanetworks_stx60d0_126f_INCLUDES := -I $(THIS_DIR)inc
x86_64_alphanetworks_stx60d0_126f_INTERNAL_INCLUDES := -I $(THIS_DIR)src
x86_64_alphanetworks_stx60d0_126f_DEPENDMODULE_ENTRIES := init:x86_64_alphanetworks_stx60d0_126f ucli:x86_64_alphanetworks_stx60d0_126f

