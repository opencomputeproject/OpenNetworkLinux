###############################################################################
#
# 
#
###############################################################################
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
x86_64_accton_as4630_54npe_INCLUDES := -I $(THIS_DIR)inc
x86_64_accton_as4630_54npe_INTERNAL_INCLUDES := -I $(THIS_DIR)src
x86_64_accton_as4630_54npe_DEPENDMODULE_ENTRIES := init:x86_64_accton_as4630_54npe ucli:x86_64_accton_as4630_54npe

