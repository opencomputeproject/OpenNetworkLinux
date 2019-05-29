###############################################################################
#
# 
#
###############################################################################
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
x86_64_inventec_d10056_INCLUDES := -I $(THIS_DIR)inc
x86_64_inventec_d10056_INTERNAL_INCLUDES := -I $(THIS_DIR)src
x86_64_inventec_d10056_DEPENDMODULE_ENTRIES := init:x86_64_inventec_d10056 ucli:x86_64_inventec_d10056

