###############################################################################
#
# 
#
###############################################################################
THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
onlp_platform_sim_INCLUDES := -I $(THIS_DIR)inc
onlp_platform_sim_INTERNAL_INCLUDES := -I $(THIS_DIR)src
onlp_platform_sim_DEPENDMODULE_ENTRIES := init:onlp_platform_sim ucli:onlp_platform_sim

