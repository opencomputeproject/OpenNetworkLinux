###############################################################################
#
#
#
###############################################################################
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
x86_64_inventec_d5264q28b_INCLUDES := -I $(THIS_DIR)inc
x86_64_inventec_d5264q28b_INTERNAL_INCLUDES := -I $(THIS_DIR)src
x86_64_inventec_d5264q28b_DEPENDMODULE_ENTRIES := init:x86_64_inventec_d5264q28b ucli:x86_64_inventec_d5264q28b
x86_64_inventec_d5264q28b_CFLAGS := -Wno-restrict -Wno-format-truncation
