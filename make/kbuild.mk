############################################################
# <bsn.cl fy=2015 v=onl>
#
#           Copyright 2015 Big Switch Networks, Inc.
#
# Licensed under the Eclipse Public License, Version 1.0 (the
# "License"); you may not use this file except in compliance
# with the License. You may obtain a copy of the License at
#
#        http://www.eclipse.org/legal/epl-v10.html
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
# either express or implied. See the License for the specific
# language governing permissions and limitations under the
# License.
#
# </bsn.cl>
############################################################
#
# Prepare and build a kernel.
#
############################################################

ifndef ARCH
$(error $$ARCH must be set)
endif

#
# The kernel major version
#
ifndef K_MAJOR_VERSION
$(error $$K_MAJOR_VERSION must be set)
endif

#
# The kernel patchlevel.
#
ifndef K_PATCH_LEVEL
$(error $$K_PATCH_LEVEL must be set)
endif

#
# The kernel sublevel
#
ifndef K_SUB_LEVEL
$(error $$K_SUB_LEVEL must be set)
endif

#
# The directory containing the patches to be applied
# to the kernel sources.
#
ifndef K_PATCH_DIR
$(error $$K_PATCH_DIR must be set)
endif

ifndef K_PATCH_SERIES_FILE
    ifndef K_PATCH_SERIES
        K_PATCH_SERIES = series
    endif
    K_PATCH_SERIES_FILE = $(K_PATCH_DIR)/$(K_PATCH_SERIES)
endif

#
# This is the directory that will receive the build targets.
# The kernel build tree is placed in this directory,
# as well as any custom copy targets.
#
ifndef K_TARGET_DIR
$(error $$K_TARGET_DIR not set)
endif

#
# This is the absolute path to the kernel configuration
# that should be used for this build.
#
ifndef K_CONFIG
$(error $$K_CONFIG not set)
endif

#
# This is the build target. bzImage, uImage, etc.
#
ifndef K_BUILD_TARGET
$(error $$K_BUILD_TARGET not set)
endif

############################################################
############################################################
ONL_KERNELS := $(ONL)/packages/base/any/kernels

K_VERSION := $(K_MAJOR_VERSION).$(K_PATCH_LEVEL).$(K_SUB_LEVEL)$(K_SUFFIX)
ifndef K_NAME
K_NAME := linux-$(K_VERSION)
endif
ifndef K_ARCHIVE_EXT
K_ARCHIVE_EXT := tar.xz
endif
ifndef K_ARCHIVE_NAME
K_ARCHIVE_NAME := $(K_NAME).$(K_ARCHIVE_EXT)
endif
K_ARCHIVE_PATH := $(ONL_KERNELS)/archives/$(K_ARCHIVE_NAME)
ifndef K_ARCHIVE_URL
K_ARCHIVE_URL := https://www.kernel.org/pub/linux/kernel/v$(K_MAJOR_VERSION).x/$(K_ARCHIVE_NAME)
endif
K_SOURCE_DIR := $(K_TARGET_DIR)/$(K_NAME)
K_MBUILD_DIR := $(K_SOURCE_DIR)-mbuild
K_INSTALL_MOD_PATH := $(K_TARGET_DIR)
K_DTBS_DIR := $(K_SOURCE_DIR)-dtbs

#
# The kernel source archive. Download if not present.
#
$(K_ARCHIVE_PATH):
	cd $(ONL_KERNELS)/archives && wget $(K_ARCHIVE_URL)


.PHONY : ksource kpatched

#
# The extracted kernel sources
#
$(K_SOURCE_DIR)/Makefile: $(K_ARCHIVE_PATH)
	mkdir -p $(K_TARGET_DIR) && cd $(K_TARGET_DIR) && tar kxf $(K_ARCHIVE_PATH)
	touch -c $(K_SOURCE_DIR)/Makefile
	$(K_MAKE) mrproper

ksource: $(K_SOURCE_DIR)/Makefile

#
# The patched kernel sources
#
$(K_SOURCE_DIR)/.PATCHED: $(K_SOURCE_DIR)/Makefile
	$(ONL)/tools/scripts/apply-patches.sh $(K_SOURCE_DIR) $(K_PATCH_DIR) $(K_PATCH_SERIES_FILE)
	touch $(K_SOURCE_DIR)/.PATCHED

kpatched: $(K_SOURCE_DIR)/.PATCHED

#
# Setup the kernel and output directory for the build.
#
setup: $(K_SOURCE_DIR)/.PATCHED
	cp $(K_CONFIG) $(K_SOURCE_DIR)/.config

#
# Kernel build command.
#
K_MAKE    := $(MAKE) -C $(K_SOURCE_DIR)

#
# Build the kernel.
#
build: setup
	+$(K_MAKE) $(K_BUILD_TARGET)
	+$(K_MAKE) modules
	+$(K_MAKE) modules_install INSTALL_MOD_PATH=$(K_INSTALL_MOD_PATH)
	find $(K_INSTALL_MOD_PATH) -type l -name source -delete
	find $(K_INSTALL_MOD_PATH) -type l -name build -delete


ifdef K_COPY_SRC
ifdef K_COPY_DST
ifdef K_COPY_GZIP
	gzip -c $(K_SOURCE_DIR)/$(K_COPY_SRC) > $(K_TARGET_DIR)/$(K_COPY_DST)
else
	cp $(K_SOURCE_DIR)/$(K_COPY_SRC) $(K_TARGET_DIR)/$(K_COPY_DST)
endif
endif
endif


MODSYNCKEEP := arch/powerpc/lib/crtsavres.o

mbuild: build
	rm -rf $(K_MBUILD_DIR)
	mkdir -p $(K_MBUILD_DIR)

	# first copy everything
	cd $(K_SOURCE_DIR) && \
	find . -type f -a \( -name 'Makefile*' -o -name 'Kconfig*' \) -a \
		-exec cp --parents -t $(K_MBUILD_DIR)/ {} +

	cd $(K_SOURCE_DIR) && \
	find drivers/ -type f -a -name '*.h' -a \
		-exec cp --parents -t $(K_MBUILD_DIR)/ {} +

	cp $(K_SOURCE_DIR)/.config $(K_MBUILD_DIR)/
	cp $(K_SOURCE_DIR)/Module.symvers $(K_MBUILD_DIR)/
	cp $(K_SOURCE_DIR)/System.map $(K_MBUILD_DIR)/

	# then drop all but the needed Makefiles/Kconfig files
	rm -rf $(K_MBUILD_DIR)/Documentation \
	       $(K_MBUILD_DIR)/scripts \
	       $(K_MBUILD_DIR)/include

	# copy scripts/
	cd $(K_SOURCE_DIR) && \
	find scripts/ -type f -a \! -name '*.o' -a \
		-exec cp --parents -t $(K_MBUILD_DIR)/ {} +

	# copy include/
	cp -a $(K_SOURCE_DIR)/include $(K_MBUILD_DIR)/

	# copy objtool for kernel-devel (needed for building external modules)
	if grep -q CONFIG_STACK_VALIDATION=y $(K_MBUILD_DIR)/.config && \
	   [ -e $(K_SOURCE_DIR)/tools/objtool/objtool ]; then \
		cd $(K_SOURCE_DIR) && \
		cp -a --parents tools/objtool/objtool $(K_MBUILD_DIR)/; \
	fi

	# Make sure the Makefile and version.h have a matching timestamp so that
	# external modules can be built
	touch -r $(K_MBUILD_DIR)/.config  $(K_MBUILD_DIR)/include/generated/autoconf.h
	touch -r $(K_MBUILD_DIR)/Makefile $(K_MBUILD_DIR)/include/generated/uapi/linux/version.h

	# Copy .config to include/config/auto.conf so "make prepare" is unnecessary.
	cp $(K_MBUILD_DIR)/.config $(K_MBUILD_DIR)/include/config/auto.conf

ifeq ($(ARCH),powerpc)
	# This file must be preserved for PPC module builds.
	cd $(K_SOURCE_DIR) && cp -a --parents arch/powerpc/lib/crtsavres.[So] $(K_MBUILD_DIR)/
endif

ifeq ($(ARCH),x86_64)
	$(eval ARCH := x86)
endif
	if [ -d $(K_SOURCE_DIR)/arch/$(ARCH) ]; then \
		cd $(K_SOURCE_DIR) && \
		find arch/$(ARCH) -type f -a -name '*lds' -a \
			-exec cp --parents -t $(K_MBUILD_DIR)/ {} +; \
	fi
	if [ -d $(K_SOURCE_DIR)/arch/$(ARCH)/scripts ]; then \
		cd $(K_SOURCE_DIR) && \
		find arch/$(ARCH)/scripts \( -type f -o -type l \) -a \! -name '*.o' -a \
			-exec cp --parents -d -t $(K_MBUILD_DIR)/ {} +; \
	fi
	if [ -d $(K_SOURCE_DIR)/arch/$(ARCH)/include ]; then \
		cd $(K_SOURCE_DIR) && \
		cp -a --parents arch/$(ARCH)/include $(K_MBUILD_DIR)/; \
	fi

	# Get rid of special files
	cd $(K_MBUILD_DIR) && \
	find \( -name '*~' -o -name '*.bak' -o -name '*.orig' -o -name '*.rej' \) -a \
		-exec rm -f {} +

dtbs: mbuild
ifdef DTS_LIST
	rm -rf $(K_DTBS_DIR)
	mkdir -p $(K_DTBS_DIR)
ifeq ($(ARCH),arm64)
	cp $(K_SOURCE_DIR)/arch/$(ARCH)/boot/dts/freescale/*.dtb $(K_DTBS_DIR)
else
	$(foreach name,$(DTS_LIST),$(K_SOURCE_DIR)/scripts/dtc/dtc -I dts -O dtb -o $(K_DTBS_DIR)/$(name).dtb $(K_SOURCE_DIR)/arch/$(ARCH)/boot/dts/$(name).dts; )
endif
endif

#
# This target can be used to manage the configuration file.
#
configure: setup
	$(K_MAKE) menuconfig
	cp $(K_SOURCE_DIR)/.config $(K_CONFIG)

.DEFAULT_GOAL := dtbs
