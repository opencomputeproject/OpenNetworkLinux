###############################################################################
#
# 
#
###############################################################################
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
x86_64_accton_as7946_30xb_INCLUDES := -I $(THIS_DIR)inc
x86_64_accton_as7946_30xb_INTERNAL_INCLUDES := -I $(THIS_DIR)src
x86_64_accton_as7946_30xb_DEPENDMODULE_ENTRIES := init:x86_64_accton_as7946_30xb ucli:x86_64_accton_as7946_30xb

