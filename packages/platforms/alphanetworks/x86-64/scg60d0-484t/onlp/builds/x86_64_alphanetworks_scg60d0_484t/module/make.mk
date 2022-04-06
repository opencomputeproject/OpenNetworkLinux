###############################################################################
#
# 
#
###############################################################################
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
x86_64_alphanetworks_scg60d0_484t_INCLUDES := -I $(THIS_DIR)inc
x86_64_alphanetworks_scg60d0_484t_INTERNAL_INCLUDES := -I $(THIS_DIR)src
x86_64_alphanetworks_scg60d0_484t_DEPENDMODULE_ENTRIES := init:x86_64_alphanetworks_scg60d0_484t ucli:x86_64_alphanetworks_scg60d0_484t

