ifndef ARCH
$(error $$ARCH not set)
endif

# Hardcoded to match ONL File naming conventions.
include $(ONL)/make/versions/version-onl.mk
INSTALLER_NAME=$(FNAME_PRODUCT_VERSION)_ONL-OS_$(FNAME_BUILD_ID)_$(UARCH)_INSTALLER

__installer:
	$(ONL)/tools/mkinstaller.py --arch $(ARCH) --boot-config boot-config --initrd onl-loader-initrd:$(ARCH) onl-loader-initrd-$(ARCH).cpio.gz --out $(INSTALLER_NAME)
	md5sum "$(INSTALLER_NAME)" | awk '{ print $$1 }' > "$(INSTALLER_NAME).md5sum"


