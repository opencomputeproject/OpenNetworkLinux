###############################################################################
#
# 
#
###############################################################################
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
x86_64_accton_as7716_24xc_INCLUDES := -I $(THIS_DIR)inc
x86_64_accton_as7716_24xc_INTERNAL_INCLUDES := -I $(THIS_DIR)src
x86_64_accton_as7716_24xc_DEPENDMODULE_ENTRIES := init:x86_64_accton_as7716_24xc ucli:x86_64_accton_as7716_24xc

