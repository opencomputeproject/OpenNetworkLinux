###############################################################################
#
#
#
###############################################################################
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
x86_64_quanta_ly4r_INCLUDES := -I $(THIS_DIR)inc
x86_64_quanta_ly4r_INTERNAL_INCLUDES := -I $(THIS_DIR)src
x86_64_quanta_ly4r_DEPENDMODULE_ENTRIES := init:x86_64_quanta_ly4r ucli:x86_64_quanta_ly4r

