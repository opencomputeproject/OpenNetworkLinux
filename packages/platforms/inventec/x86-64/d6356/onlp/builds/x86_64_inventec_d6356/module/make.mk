###############################################################################
#
#
#
###############################################################################
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
x86_64_inventec_d6356_INCLUDES := -I $(THIS_DIR)inc
x86_64_inventec_d6356_INTERNAL_INCLUDES := -I $(THIS_DIR)src
x86_64_inventec_d6356_DEPENDMODULE_ENTRIES := init:x86_64_inventec_d6356 ucli:x86_64_inventec_d6356

# This error should be addressed in the code.
x86_64_inventec_d6356_BROKEN_CFLAGS += -Wno-stringop-truncation
