###############################################################################
#
# 
#
###############################################################################
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
x86_64_accton_as5812_54t_INCLUDES := -I $(THIS_DIR)inc
x86_64_accton_as5812_54t_INTERNAL_INCLUDES := -I $(THIS_DIR)src
x86_64_accton_as5812_54t_DEPENDMODULE_ENTRIES := init:x86_64_accton_as5812_54t ucli:x86_64_accton_as5812_54t

