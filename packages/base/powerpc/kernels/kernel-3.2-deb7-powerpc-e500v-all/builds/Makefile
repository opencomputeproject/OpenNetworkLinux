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
	$(MAKE) -C $(ONL)/packages/base/any/kernels/3.2.65-1+deb7u2/configs/powerpc-e500v-all K_TARGET_DIR=$(THIS_DIR) K_COPY_DST=kernel-3.2-deb7-powerpc-e500v-all.bin.gz

clean:
	rm -rf linux-3.2.65-1+deb7u2 linux-3.2.65-1+deb7u2-mbuild linux-3.2.65-1+deb7u2-dtbs
