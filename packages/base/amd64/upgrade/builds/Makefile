include $(ONL)/make/config.amd64.mk

# All amd64 kernels
KERNELS := $(shell $(ONLPM) --find-file onl-kernel-3.9.6-x86-64-all:amd64    kernel-3.9.6-x86-64-all) \
	   $(shell $(ONLPM) --find-file onl-kernel-3.2-deb7-x86-64-all:amd64 kernel-3.2-deb7-x86_64-all) \
	   $(shell $(ONLPM) --find-file onl-kernel-3.18-x86-64-all:amd64     kernel-3.18-x86_64-all) \

# Loader initrd
INITRD := $(shell $(ONLPM) --find-file onl-loader-initrd:amd64 onl-loader-initrd-amd64.cpio.gz)
MANIFEST := $(shell $(ONLPM) --find-file onl-loader-initrd:amd64 manifest.json)

all:
	mkdir -p files
	cp $(KERNELS) files
	cp $(INITRD) files/initrd-amd64
	cp $(MANIFEST) files


