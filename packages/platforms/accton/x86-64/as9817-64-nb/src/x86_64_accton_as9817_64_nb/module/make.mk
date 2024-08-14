###############################################################################
#
#
#
###############################################################################
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
x86_64_accton_as9817_64_nb_INCLUDES := -I $(THIS_DIR)inc
x86_64_accton_as9817_64_nb_INTERNAL_INCLUDES := -I $(THIS_DIR)src
x86_64_accton_as9817_64_nb_DEPENDMODULE_ENTRIES := init:x86_64_accton_as9817_64_nb ucli:x86_64_accton_as9817_64_nb
