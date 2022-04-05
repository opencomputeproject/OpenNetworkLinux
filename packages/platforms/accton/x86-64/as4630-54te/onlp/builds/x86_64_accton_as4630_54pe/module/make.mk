###############################################################################
#
# 
#
###############################################################################
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
x86_64_accton_as4630_54te_INCLUDES := -I $(THIS_DIR)inc
x86_64_accton_as4630_54te_INTERNAL_INCLUDES := -I $(THIS_DIR)src
x86_64_accton_as4630_54te_DEPENDMODULE_ENTRIES := init:x86_64_accton_as4630_54te ucli:x86_64_accton_as4630_54te

