###############################################################################
#
# 
#
###############################################################################
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
x86_64_lenovo_ne10032_INCLUDES := -I $(THIS_DIR)inc
x86_64_lenovo_ne10032_INTERNAL_INCLUDES := -I $(THIS_DIR)src
x86_64_lenovo_ne10032_DEPENDMODULE_ENTRIES := init:x86_64_lenovo_ne10032 ucli:x86_64_lenovo_ne10032

