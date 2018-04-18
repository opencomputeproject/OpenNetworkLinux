include $(ONL)/make/config.amd64.mk

# All amd64 kernels
KERNELS := $(shell $(ONLPM) --find-file onl-kernel-3.16-lts-x86-64-all:amd64 kernel-3.16-lts-x86_64-all) \
	   $(shell $(ONLPM) --find-file onl-kernel-4.9-lts-x86-64-all:amd64 kernel-4.9-lts-x86_64-all) \
	   $(shell $(ONLPM) --find-file onl-kernel-4.14-lts-x86-64-all:amd64 kernel-4.14-lts-x86_64-all)


# Loader initrd
INITRD := $(shell $(ONLPM) --find-file onl-loader-initrd:amd64 onl-loader-initrd-amd64.cpio.gz)
MANIFEST := $(shell $(ONLPM) --find-file onl-loader-initrd:amd64 manifest.json)

all:
	mkdir -p files
	cp $(KERNELS) files
	cp $(INITRD) files
	cp $(MANIFEST) files
