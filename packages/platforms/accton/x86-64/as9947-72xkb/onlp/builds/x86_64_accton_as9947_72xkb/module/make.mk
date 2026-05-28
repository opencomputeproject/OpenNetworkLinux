###############################################################################
#
# 
#
###############################################################################
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
x86_64_accton_as9947_72xkb_INCLUDES := -I $(THIS_DIR)inc
x86_64_accton_as9947_72xkb_INTERNAL_INCLUDES := -I $(THIS_DIR)src
x86_64_accton_as9947_72xkb_DEPENDMODULE_ENTRIES := init:x86_64_accton_as9947_72xkb ucli:x86_64_accton_as9947_72xkb

