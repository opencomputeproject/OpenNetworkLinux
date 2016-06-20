# -*- Makefile -*-
############################################################
# <bsn.cl fy=2016 v=none>
#
#        Copyright 2013, 2014 BigSwitch Networks, Inc.
#        Copyright 2016 Freescale Semiconductor, Inc.
#
#
# </bsn.cl>
############################################################
THIS_DIR := $(abspath $(dir $(lastword $(MAKEFILE_LIST))))

include $(ONL)/make/config.mk

kernel:
	$(MAKE) -C $(ONL)/packages/base/any/kernels/3.18.25/configs/powerpc64-all K_TARGET_DIR=$(THIS_DIR) $(ONL_MAKE_PARALLEL)

clean:
	rm -rf linux-3.18.25 linux-3.18.25-mbuild
