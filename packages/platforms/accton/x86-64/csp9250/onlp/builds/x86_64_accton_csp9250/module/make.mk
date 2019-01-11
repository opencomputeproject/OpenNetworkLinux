###############################################################################
#
# 
#
###############################################################################
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
x86_64_accton_csp9250_INCLUDES := -I $(THIS_DIR)inc
x86_64_accton_csp9250_INTERNAL_INCLUDES := -I $(THIS_DIR)src
x86_64_accton_csp9250_DEPENDMODULE_ENTRIES := init:x86_64_accton_csp9250 ucli:x86_64_accton_csp9250

