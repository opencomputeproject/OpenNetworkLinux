###############################################################################
#
# 
#
###############################################################################

LIBRARY := x86_64_accton_as7946_30xb
$(LIBRARY)_SUBDIR := $(dir $(lastword $(MAKEFILE_LIST)))
include $(BUILDER)/lib.mk
