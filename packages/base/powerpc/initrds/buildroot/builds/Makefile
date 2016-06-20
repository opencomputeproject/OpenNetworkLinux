# -*- Makefile -*-
############################################################
# <bsn.cl fy=2013 v=onl>
#
#        Copyright 2013, 2014 Big Switch Networks, Inc.
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
#
############################################################
include $(ONL)/make/config.powerpc.mk

all: buildroot-initrd-$(ARCH).cpio.gz buildroot-initrd-ppc64.cpio.gz

include $(ONL)/packages/base/any/initrds/buildroot/Makefile.any

BUILDROOT_SOURCE_GIT := $(ONL_BUILDROOT)/buildroot-git
BUILDROOT_GIT_URL := https://github.com/buildroot/buildroot.git
BUILDROOT_TOOLCHAIN_PPC64 := $(ONL_BUILDROOT)/buildroot-ppc64/host/usr/bin/powerpc64-buildroot-linux-gnu-

buildroot-initrd-ppc64.cpio.gz: $(ONL_BUILDROOT)/.ppc64.setup.done
	make -C $(BUILDROOT_SOURCE_GIT) O=../buildroot-ppc64
	cp $(ONL_BUILDROOT)/buildroot-ppc64/images/rootfs.cpio.gz onl-buildroot-initrd-ppc64.cpio.gz
	@sudo ln -sf $(BUILDROOT_TOOLCHAIN_PPC64)gcc  /usr/bin/powerpc64-buildroot-linux-gnu-gcc
	@sudo ln -sf $(BUILDROOT_TOOLCHAIN_PPC64)ld   /usr/bin/powerpc64-buildroot-linux-gnu-ld
	@sudo ln -sf $(BUILDROOT_TOOLCHAIN_PPC64)ar   /usr/bin/powerpc64-buildroot-linux-gnu-ar
	@sudo ln -sf $(BUILDROOT_TOOLCHAIN_PPC64)nm   /usr/bin/powerpc64-buildroot-linux-gnu-nm
	@sudo ln -sf $(BUILDROOT_TOOLCHAIN_PPC64)size   /usr/bin/powerpc64-buildroot-linux-gnu-size
	@sudo ln -sf $(BUILDROOT_TOOLCHAIN_PPC64)strip  /usr/bin/powerpc64-buildroot-linux-gnu-strip
	@sudo ln -sf $(BUILDROOT_TOOLCHAIN_PPC64)objcopy   /usr/bin/powerpc64-buildroot-linux-gnu-objcopy
	@sudo ln -sf $(BUILDROOT_TOOLCHAIN_PPC64)objdump   /usr/bin/powerpc64-buildroot-linux-gnu-objdump

$(ONL_BUILDROOT)/.ppc64.setup.done:
	if [ ! -d $(BUILDROOT_SOURCE_GIT) ]; then \
		cd $(ONL_BUILDROOT); git clone $(BUILDROOT_GIT_URL); \
		mv buildroot buildroot-git; cd buildroot-git; git reset --hard f6c2e55a87e5a00; cd ..; \
	fi
	@sed -i '/CONFIG_CHECKPASS/'d  $(ONL_BUILDROOT)/busybox.config
	@sed -i '/legacy configuration/'d $(BUILDROOT_SOURCE_GIT)/Makefile.legacy
	@sed -i 's/utf8\$$$()/utf/g' $(BUILDROOT_SOURCE_GIT)/support/dependencies/dependencies.sh
	@mkdir -p $(ONL_BUILDROOT)/buildroot-ppc64
	@cp $(ONL_BUILDROOT)/buildroot.config-ppc64 $(ONL_BUILDROOT)/buildroot-ppc64/.config
	touch $(ONL_BUILDROOT)/.ppc64.setup.done
