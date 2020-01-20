###############################################################################
#
# 
#
###############################################################################
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
x86_64_cel_silverstone_INCLUDES := -I $(THIS_DIR)inc
x86_64_cel_silverstone_INTERNAL_INCLUDES := -I $(THIS_DIR)src
x86_64_cel_silverstone_DEPENDMODULE_ENTRIES := init:x86_64_cel_silverstone ucli:x86_64_cel_silverstone

