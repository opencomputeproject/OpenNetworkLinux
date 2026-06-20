###############################################################################
#
# 
#
###############################################################################
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
x86_64_accton_as9947_36xkb_INCLUDES := -I $(THIS_DIR)inc
x86_64_accton_as9947_36xkb_INTERNAL_INCLUDES := -I $(THIS_DIR)src
x86_64_accton_as9947_36xkb_DEPENDMODULE_ENTRIES := init:x86_64_accton_as9947_36xkb ucli:x86_64_accton_as9947_36xkb

