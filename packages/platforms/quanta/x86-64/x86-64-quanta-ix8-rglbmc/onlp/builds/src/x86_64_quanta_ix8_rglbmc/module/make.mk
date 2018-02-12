###############################################################################
#
#
#
###############################################################################
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
x86_64_quanta_ix8_rglbmc_INCLUDES := -I $(THIS_DIR)inc
x86_64_quanta_ix8_rglbmc_INTERNAL_INCLUDES := -I $(THIS_DIR)src
x86_64_quanta_ix8_rglbmc_DEPENDMODULE_ENTRIES := init:x86_64_quanta_ix8_rglbmc ucli:x86_64_quanta_ix8_rglbmc

