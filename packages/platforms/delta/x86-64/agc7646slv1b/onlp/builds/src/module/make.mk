###############################################################################
#
#
#
###############################################################################
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
x86_64_delta_agc7646slv1b_INCLUDES := -I $(THIS_DIR)inc
x86_64_delta_agc7646slv1b_INTERNAL_INCLUDES := -I $(THIS_DIR)src
x86_64_delta_agc7646slv1b_DEPENDMODULE_ENTRIES := init:x86_64_delta_agc7646slv1b ucli:x86_64_delta_agc7646slv1b

