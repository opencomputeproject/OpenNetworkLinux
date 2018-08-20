###############################################################################
#
# 
#
###############################################################################
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
x86_64_accton_as5916_54xksb_INCLUDES := -I $(THIS_DIR)inc
x86_64_accton_as5916_54xksb_INTERNAL_INCLUDES := -I $(THIS_DIR)src
x86_64_accton_as5916_54xksb_DEPENDMODULE_ENTRIES := init:x86_64_accton_as5916_54xksb ucli:x86_64_accton_as5916_54xksb

