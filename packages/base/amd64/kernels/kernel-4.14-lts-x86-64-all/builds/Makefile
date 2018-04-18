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
	rm -rf lib
	dpkg -l libelf-dev > /dev/null 2>1 || sudo apt-get install libelf-dev # Fixme -- add to builder
	$(MAKE) -C $(ONL)/packages/base/any/kernels/4.14-lts/configs/x86_64-all K_TARGET_DIR=$(THIS_DIR) $(ONL_MAKE_PARALLEL)
	ARCH=x86_64 $(ONL)/tools/scripts/kmodbuild.sh linux-4.14.*-mbuild "$(wildcard $(ONL)/packages/base/any/kernels/modules/*)" onl/onl/common

clean:
	rm -rf linux-4.14* kernel-4.14* lib
