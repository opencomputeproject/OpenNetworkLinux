# -*- Makefile -*-
############################################################
# <bsn.cl fy=2013 v=none>
#
#        Copyright 2013, 2014 BigSwitch Networks, Inc.
#
#
#
# </bsn.cl>
############################################################
THIS_DIR := $(abspath $(dir $(lastword $(MAKEFILE_LIST))))

include $(ONL)/make/config.mk

export MODSYNCLIST_EXTRA := arch/arm/plat-iproc/include

kernel:
	$(MAKE) -C $(ONL)/packages/base/any/kernels/3.2-lts/configs/arm-iproc-all K_TARGET_DIR=$(THIS_DIR) $(ONL_MAKE_PARALLEL)

clean:
	rm -rf kernel-* linux-3.2.*
