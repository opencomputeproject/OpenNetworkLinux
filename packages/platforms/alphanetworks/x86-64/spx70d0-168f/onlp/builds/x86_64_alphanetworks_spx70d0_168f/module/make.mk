###############################################################################
#
# 
#
###############################################################################
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
x86_64_alphanetworks_spx70d0_168f_INCLUDES := -I $(THIS_DIR)inc
x86_64_alphanetworks_spx70d0_168f_INTERNAL_INCLUDES := -I $(THIS_DIR)src
x86_64_alphanetworks_spx70d0_168f_DEPENDMODULE_ENTRIES := init:x86_64_alphanetworks_spx70d0_168f ucli:x86_64_alphanetworks_spx70d0_168f

