###############################################################################
#
# 
#
###############################################################################
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
x86_64_quanta_ly8_rangeley_INCLUDES := -I $(THIS_DIR)inc
x86_64_quanta_ly8_rangeley_INTERNAL_INCLUDES := -I $(THIS_DIR)src
x86_64_quanta_ly8_rangeley_DEPENDMODULE_ENTRIES := init:x86_64_quanta_ly8_rangeley ucli:x86_64_quanta_ly8_rangeley

