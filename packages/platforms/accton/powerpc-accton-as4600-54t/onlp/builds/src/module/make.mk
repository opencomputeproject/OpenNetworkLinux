###############################################################################
#
#
#
###############################################################################
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
powerpc_accton_as4600_54t_INCLUDES := -I $(THIS_DIR)inc
powerpc_accton_as4600_54t_INTERNAL_INCLUDES := -I $(THIS_DIR)src
powerpc_accton_as4600_54t_DEPENDMODULE_ENTRIES := init:powerpc_accton_as4600_54t

