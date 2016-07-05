# -*- Makefile -*-
THIS_DIR := $(abspath $(dir $(lastword $(MAKEFILE_LIST))))

include $(ONL)/make/config.mk

kernel:
	$(MAKE) -C $(ONL)/packages/base/any/kernels/3.18.25/configs/arm64-all K_TARGET_DIR=$(THIS_DIR) $(ONL_MAKE_PARALLEL)

clean:
	rm -rf linux-3.18.25*
