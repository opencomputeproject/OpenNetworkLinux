###############################################################################
#
# 
#
###############################################################################
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
x86_64_accton_as9926_24d_INCLUDES := -I $(THIS_DIR)inc
x86_64_accton_as9926_24d_INTERNAL_INCLUDES := -I $(THIS_DIR)src
x86_64_accton_as9926_24d_DEPENDMODULE_ENTRIES := init:x86_64_accton_as9926_24d ucli:x86_64_accton_as9926_24d

