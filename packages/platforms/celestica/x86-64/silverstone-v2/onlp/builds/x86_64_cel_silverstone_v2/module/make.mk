###############################################################################
#
# 
#
###############################################################################
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
x86_64_cel_silverstone_v2_INCLUDES := -I $(THIS_DIR)inc
x86_64_cel_silverstone_v2_INTERNAL_INCLUDES := -I $(THIS_DIR)src
x86_64_cel_silverstone_v2_DEPENDMODULE_ENTRIES := init:x86_64_cel_silverstone_v2 ucli:x86_64_cel_silverstone_v2

