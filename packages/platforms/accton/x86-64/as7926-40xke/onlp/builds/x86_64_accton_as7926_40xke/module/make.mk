###############################################################################
#
#
#
###############################################################################
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
x86_64_accton_as7926_40xke_INCLUDES := -I $(THIS_DIR)inc
x86_64_accton_as7926_40xke_INTERNAL_INCLUDES := -I $(THIS_DIR)src
x86_64_accton_as7926_40xke_DEPENDMODULE_ENTRIES := init:x86_64_accton_as7926_40xke ucli:x86_64_accton_as7926_40xke
