###############################################################################
#
#
#
###############################################################################
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
sff_INCLUDES := -I $(THIS_DIR)inc
sff_INTERNAL_INCLUDES := -I $(THIS_DIR)src
sff_DEPENDMODULE_ENTRIES := init:sff

