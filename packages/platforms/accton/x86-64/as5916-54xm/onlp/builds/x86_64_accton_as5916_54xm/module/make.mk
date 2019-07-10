###############################################################################
#
# 
#
###############################################################################
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
x86_64_accton_as5916_54xm_INCLUDES := -I $(THIS_DIR)inc
x86_64_accton_as5916_54xm_INTERNAL_INCLUDES := -I $(THIS_DIR)src
x86_64_accton_as5916_54xm_DEPENDMODULE_ENTRIES := init:x86_64_accton_as5916_54xm ucli:x86_64_accton_as5916_54xm

