###############################################################################
#
#
#
###############################################################################
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
x86_64_inventec_d10064_INCLUDES := -I $(THIS_DIR)inc
x86_64_inventec_d10064_INTERNAL_INCLUDES := -I $(THIS_DIR)src
x86_64_inventec_d10064_DEPENDMODULE_ENTRIES := init:x86_64_inventec_d10064 ucli:x86_64_inventec_d10064
x86_64_inventec_d10064_CFLAGS := -Wno-restrict -Wno-format-truncation
