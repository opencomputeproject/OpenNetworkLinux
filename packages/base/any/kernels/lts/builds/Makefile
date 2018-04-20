include $(ONL)/make/config.mk

ifndef KERNEL_BUILD_DIR
$(error $$KERNEL_BUILD_DIR not set)
endif

ifndef KERNEL_ARCH
$(error $$KERNEL_ARCH not set)
endif

ifndef KERNEL_LTS_VERSION
$(error $$KERNEL_LTS_VERSION not set)
endif

ifndef KERNEL_CONFIG
$(error $$KERNEL_CONFIG not set)
endif

export K_TARGET_DIR := $(KERNEL_BUILD_DIR)/$(ONL_DEBIAN_SUITE)

kernel: setup
	$(MAKE) -C $(ONL)/packages/base/any/kernels/$(KERNEL_LTS_VERSION)-lts/configs/$(KERNEL_CONFIG) $(ONL_MAKE_PARALLEL)
	cd $(K_TARGET_DIR) && ARCH=$(KERNEL_ARCH) $(ONL)/tools/scripts/kmodbuild.sh linux-*-mbuild "$(wildcard $(ONL)/packages/base/any/kernels/modules/*)" onl/onl/common

clean:
	rm -rf $(K_TARGET_DIR)

setup::
