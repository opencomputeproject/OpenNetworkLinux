include $(ONL)/make/config.powerpc.mk

# Hardcoded to match ONL File naming conventions.
include $(ONL)/make/version-onl.mk
INSTALLER_NAME=$(FNAME_PRODUCT_VERSION)_ONL-OS_$(FNAME_BUILD_ID)_$(UARCH)_INSTALLER

FIT_IMAGE_ALL := $(shell $(ONLPM) --find-file onl-loader-fit:powerpc onl-loader-fit.itb)

# Fixme -- should come from the FIT manifest, not the initrd manifest
PLATFORMS := $(shell $(ONLPM) --platform-manifest onl-loader-initrd:powerpc)

MKSHAR = $(ONL)/tools/mkshar
MKSHAR_OPTS = --lazy --unzip-pad
MKSHAR_PERMS = autoperms.sh

__installer:
	$(ONL_V_at)rm -rf *INSTALLER* *.md5sum
	$(ONL_V_at)cp $(FIT_IMAGE_ALL) .
	$(foreach p,$(PLATFORMS), $(ONLPM) --extract-dir onl-platform-config-$(p):powerpc .;)
	# Fixme
	$(ONLPM) --extract-dir onl-swi:powerpc .
	mv ./usr/share/onl/packages/powerpc/onl-swi/*.swi .
	rm -rf ./usr
	$(ONL_V_at)cp /dev/null $(MKSHAR_PERMS)
	$(ONL_V_at) cp $(ONL)/make/version-onl.sh .
	$(ONL_V_at)echo "#!/bin/sh" >> $(MKSHAR_PERMS)
	$(ONL_V_at)echo "set -e" >> $(MKSHAR_PERMS)
	$(ONL_V_at)echo "set -x" >> $(MKSHAR_PERMS)
	$(MKSHAR) $(MKSHAR_OPTS) "$(INSTALLER_NAME)" $(ONL)/tools/scripts/sfx.sh.in ppc-installer.sh ppc-install-lib *.itb lib *.swi version-onl.sh boot-config
	$(ONL_V_at)rm -rf ./lib *.swi version-onl.sh autoperms.sh *.itb
	md5sum "$(INSTALLER_NAME)" | awk '{ print $$1 }' > "$(INSTALLER_NAME).md5sum"


shar installer: __installer

