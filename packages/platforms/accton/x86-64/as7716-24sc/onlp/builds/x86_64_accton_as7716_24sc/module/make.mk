###############################################################################
#
# 
#
###############################################################################
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
x86_64_accton_as7716_24sc_INCLUDES := -I $(THIS_DIR)inc
x86_64_accton_as7716_24sc_INTERNAL_INCLUDES := -I $(THIS_DIR)src
x86_64_accton_as7716_24sc_DEPENDMODULE_ENTRIES := init:x86_64_accton_as7716_24sc ucli:x86_64_accton_as7716_24sc

