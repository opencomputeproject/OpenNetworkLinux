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
VENDOR_PACKAGES := $(foreach p,$(PLATFORMS),$(shell python $(ONL)/tools/onlplatform.py $(p) $(ARCH) vendor))

ROOT := root
TARGET := onl-loader-initrd-$(ARCH).cpio.gz
.PHONY: $(TARGET)

$(TARGET):
	$(ONLPM) --copy-file onl-buildroot-initrd:$(ARCH) onl-buildroot-initrd-$(ARCH).cpio.gz .
	sudo rm -rf $(ROOT) && mkdir $(ROOT)
	$(ONLPM) --sudo --force --extract-dir onl-loader-initrd-files:all $(ROOT)
	$(ONLPM) --sudo $(foreach p,$(PLATFORM_PACKAGES),--extract-dir $(p) $(ROOT))
	$(MAKE) __vendor_config_data
	$(ONLPM) --sudo --force --extract-dir onl-vendor-config-onl:all $(ROOT)
	$(ONL)/tools/sjson.py --kj version $(ONL)/make/versions/version-onl.json --kl platforms $(PLATFORMS) --kv arch $(ARCH) --out manifest.json
	sudo mkdir -p $(ROOT)/etc/onl/loader && sudo cp manifest.json $(ROOT)/etc/onl/loader
	sudo $(ONL)/tools/makedevs -d $(ROOT)/etc/rootperms $(abspath $(ROOT))
	sudo $(ONL)/tools/cpiomod.py --cpio onl-buildroot-initrd-$(ARCH).cpio.gz --add-directory $(ROOT) --out $@
	sudo rm -rf $(ROOT) onl-buildroot-initrd-$(ARCH).cpio.gz

__vendor_config_data:
	set -e ;\
	vpkgs= ;\
	l="$(PLATFORMS)"; for p in $$l; do \
	  vpkg=$$(python $(ONL)/tools/onlplatform.py $$p $(ARCH) vendor) ;\
	  case " $$vpkgs " in *" $$vpkg "*) continue ;; esac ;\
	  vpkgs=$$vpkgs$${vpkgs:+" "}$$vpkg ;\
	  echo "Adding vendor package $$vpkg" ;\
	  $(ONLPM) --sudo --force --extract-dir $$vpkg $(ROOT) ;\
	done ;\
	:
