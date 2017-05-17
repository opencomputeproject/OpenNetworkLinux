DIRECTORIES := rootfs swi installer
include $(ONL)/make/subdirs.mk

.PHONY: swi

swi:
	$(MAKE) -C rootfs
	$(MAKE) -C swi

