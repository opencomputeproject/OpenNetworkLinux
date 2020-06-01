###############################################################################
#
#
#
###############################################################################
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
x86_64_accton_as7936_22xke_INCLUDES := -I $(THIS_DIR)inc
x86_64_accton_as7936_22xke_INTERNAL_INCLUDES := -I $(THIS_DIR)src
x86_64_accton_as7936_22xke_DEPENDMODULE_ENTRIES := init:x86_64_accton_as7936_22xke ucli:x86_64_accton_as7936_22xke
