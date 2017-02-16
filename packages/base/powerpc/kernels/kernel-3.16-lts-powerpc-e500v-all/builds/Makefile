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

kernel:
	$(MAKE) -C $(ONL)/packages/base/any/kernels/3.16-lts/configs/powerpc-e500v-all K_TARGET_DIR=$(THIS_DIR) K_COPY_DST=kernel-3.16-lts-powerpc-e500v-all.bin.gz $(ONL_MAKE_PARALLEL)

clean:
	rm -rf linux-3.16.39 linux-3.16.39-mbuild linux-3.16.39-dtbs
