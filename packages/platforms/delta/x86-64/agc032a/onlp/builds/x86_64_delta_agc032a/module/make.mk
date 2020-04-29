###############################################################################
#
#
#
###############################################################################
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
x86_64_delta_agc032a_INCLUDES := -I $(THIS_DIR)inc
x86_64_delta_agc032a_INTERNAL_INCLUDES := -I $(THIS_DIR)src
x86_64_delta_agc032a_DEPENDMODULE_ENTRIES := init:x86_64_delta_agc032a ucli:x86_64_delta_agc032a