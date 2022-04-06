###############################################################################
#
# 
#
###############################################################################
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
x86_64_accton_es7632bt3_INCLUDES := -I $(THIS_DIR)inc
x86_64_accton_es7632bt3_INTERNAL_INCLUDES := -I $(THIS_DIR)src
x86_64_accton_es7632bt3_DEPENDMODULE_ENTRIES := init:x86_64_accton_es7632bt3 ucli:x86_64_accton_es7632bt3

