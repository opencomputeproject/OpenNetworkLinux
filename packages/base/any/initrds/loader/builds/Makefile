############################################################
#
# ONL Loader Initrd Build Template.
#
############################################################
ifndef ARCH
$(error $$ARCH must be set)
endif

ifndef PLATFORMS
# Include all available platforms in the current architecture.
PLATFORMS := $(shell onlpm --list-platforms --arch $(ARCH))
endif

PLATFORM_PACKAGES := $(foreach p,$(PLATFORMS),onl-platform-config-$(p):$(ARCH))

ROOT := root
TARGET := onl-loader-initrd-$(ARCH).cpio.gz
.PHONY: $(TARGET)

$(TARGET):
	$(ONLPM) --copy-file onl-buildroot-initrd:$(ARCH) onl-buildroot-initrd-$(ARCH).cpio.gz .
	sudo rm -rf $(ROOT) && mkdir $(ROOT)
	$(ONLPM) --sudo --force --extract-dir onl-loader-initrd-files:all $(ROOT)
	$(ONLPM) --sudo $(foreach p,$(PLATFORM_PACKAGES),--extract-dir $(p) $(ROOT))
	$(ONL)/tools/sjson.py --kj version $(ONL)/make/version-onl.json --kl platforms $(PLATFORMS) --kv arch $(ARCH) --out manifest.json
	sudo mkdir -p $(ROOT)/etc/onl/loader && sudo cp manifest.json $(ROOT)/etc/onl/loader
	sudo $(ONL)/tools/makedevs -d $(ROOT)/etc/rootperms $(abspath $(ROOT))
	sudo $(ONL)/tools/cpiomod.py --cpio onl-buildroot-initrd-$(ARCH).cpio.gz --add-directory $(ROOT) --out $@
	sudo rm -rf $(ROOT) onl-buildroot-initrd-$(ARCH).cpio.gz





