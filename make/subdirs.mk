############################################################
# <bsn.cl fy=2014 v=none>
# </bsn.cl>
############################################################
include $(ONL)/make/config.mk

ifndef DIRECTORIES
DIRECTORIES := $(notdir $(wildcard $(CURDIR)/*))
endif

FILTER := make Makefile Makefile~ $(FILTER)
DIRECTORIES := $(filter-out $(FILTER),$(DIRECTORIES))

all $(MAKECMDGOALS):
	+$(ONL_V_at) $(foreach d,$(DIRECTORIES),$(ONL_MAKE) -C $(d) $(MAKECMDGOALS) || exit 1;)




