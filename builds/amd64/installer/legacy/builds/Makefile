include $(ONL)/make/config.amd64.mk

KERNELS := $(shell $(ONLPM) --find-file onl-kernel-3.9.6-x86-64-all:amd64 kernel-3.9.6-x86-64-all) \
	   $(shell $(ONLPM) --find-file onl-kernel-3.2-deb7-x86-64-all:amd64 kernel-3.2-deb7-x86_64-all) \
           $(shell $(ONLPM) --find-file onl-kernel-3.18-x86-64-all:amd64 kernel-3.18-x86_64-all)

INITRD := $(shell $(ONLPM) --find-file onl-loader-initrd:amd64 onl-loader-initrd-amd64.cpio.gz)
PLATFORMS := $(shell $(ONLPM) --platform-manifest onl-loader-initrd:amd64)

MKSHAR = $(ONL)/tools/mkshar
MKSHAR_OPTS = --lazy --unzip-pad --fixup-perms autoperms.sh
MKSHAR_PERMS = autoperms.sh


# Hardcoded to match ONL File naming conventions.
include $(ONL)/make/version-onl.mk
INSTALLER_NAME=$(FNAME_PRODUCT_VERSION)_ONL-OS_$(FNAME_BUILD_ID)_$(UARCH)_LEGACY_INSTALLER



__installer:
	$(ONL_V_at)rm -rf *INSTALLER* *.md5sum
	$(foreach k,$(KERNELS),cp $(k) .;)
	$(ONL_V_at)cp $(INITRD) initrd-amd64
	$(foreach p,$(PLATFORMS), $(ONLPM) --extract-dir onl-platform-config-$(p):amd64 .;)
	# Fixme
	$(ONLPM) --extract-dir onl-swi:amd64 .
	mv ./usr/share/onl/packages/amd64/onl-swi/*.swi .
	rm -rf ./usr
	$(ONL_V_at)cp /dev/null $(MKSHAR_PERMS)
	$(ONL_V_at) cp $(ONL)/make/version-onl.sh .
	$(ONL_V_at)echo "#!/bin/sh" >> $(MKSHAR_PERMS)
	$(ONL_V_at)echo "set -e" >> $(MKSHAR_PERMS)
	$(ONL_V_at)echo "set -x" >> $(MKSHAR_PERMS)
	$(MKSHAR) $(MKSHAR_OPTS) "$(INSTALLER_NAME)" $(ONL)/tools/scripts/sfx.sh.in amd64-installer.sh $(notdir $(KERNELS)) initrd-amd64 lib *.swi version-onl.sh boot-config
	$(ONL_V_at)rm -rf ./lib ./usr kernel-* initrd-amd64 $(ZTN_MANIFEST) *.swi version-onl.sh autoperms.sh
	md5sum "$(INSTALLER_NAME)" | awk '{ print $$1 }' > "$(INSTALLER_NAME).md5sum"


shar installer: installer

clean:
	rm -f *.swi *.installer $(notdir $(KERNELS)) initrd-amd64
