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
	$(MAKE) -C $(ONL)/packages/base/any/kernels/3.2-lts/configs/x86_64-all K_TARGET_DIR=$(THIS_DIR) $(ONL_MAKE_PARALLEL)
	rm -rf lib
	ARCH=x86_64 $(ONL)/tools/scripts/kmodbuild.sh linux-3.2.84-mbuild "$(wildcard $(ONL)/packages/base/any/kernels/modules/*)" onl/onl/common

clean:
	rm -rf linux-3.2* kernel-3.2*
