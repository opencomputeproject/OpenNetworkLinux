###############################################################################
#
# 
#
###############################################################################
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
x86_64_accton_as4630_54pe_INCLUDES := -I $(THIS_DIR)inc
x86_64_accton_as4630_54pe_INTERNAL_INCLUDES := -I $(THIS_DIR)src
x86_64_accton_as4630_54pe_DEPENDMODULE_ENTRIES := init:x86_64_accton_as4630_54pe ucli:x86_64_accton_as4630_54pe

