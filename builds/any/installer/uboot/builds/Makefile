ifndef ARCH
$(error $$ARCH not set)
endif

ifndef BOOTMODE
$(error $$BOOTMODE not set)
endif

# Hardcoded to match ONL File naming conventions.
include $(ONL)/make/versions/version-onl.mk
INSTALLER_NAME=$(FNAME_PRODUCT_VERSION)_ONL-OS_$(FNAME_BUILD_ID)_$(UARCH)_$(BOOTMODE)_INSTALLER

MKINSTALLER_OPTS	= \
  --arch $(ARCH) \
  --boot-config boot-config \
  --add-dir config \
  --fit onl-loader-fit:$(ARCH) onl-loader-fit.itb \
  --swi onl-swi:$(ARCH) \
  --preinstall-script $(ONL)/builds/any/installer/sample-preinstall.sh \
  --postinstall-script $(ONL)/builds/any/installer/sample-postinstall.sh \
  --plugin $(ONL)/builds/any/installer/sample-preinstall.py \
  --plugin $(ONL)/builds/any/installer/sample-postinstall.py \
  # THIS LINE INTENTIONALLY LEFT BLANK

__installer:
	$(ONL)/tools/mkinstaller.py $(MKINSTALLER_OPTS) --out $(INSTALLER_NAME)
	md5sum "$(INSTALLER_NAME)" | awk '{ print $$1 }' > "$(INSTALLER_NAME).md5sum"


