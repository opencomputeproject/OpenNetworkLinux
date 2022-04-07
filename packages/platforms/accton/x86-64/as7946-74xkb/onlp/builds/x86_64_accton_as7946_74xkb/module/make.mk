###############################################################################
#
# 
#
###############################################################################
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
x86_64_accton_as7946_74xkb_INCLUDES := -I $(THIS_DIR)inc
x86_64_accton_as7946_74xkb_INTERNAL_INCLUDES := -I $(THIS_DIR)src
x86_64_accton_as7946_74xkb_DEPENDMODULE_ENTRIES := init:x86_64_accton_as7946_74xkb ucli:x86_64_accton_as7946_74xkb

