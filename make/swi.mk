ifndef ARCH
$(error $$ARCH is not set)
endif

ifndef ROOTFS_PACKAGE
$(error $$ROOTFS_PACKAGE not set.)
endif

ifdef REBUILD_RFS
FORCE_OPTION = --force
endif

LINK_OPTIONS := $(FORCE_OPTION) --link-file $(ROOTFS_PACKAGE):$(ARCH) rootfs-$(ARCH).sqsh . --link-file $(ROOTFS_PACKAGE):$(ARCH) manifest.json .

ifndef FILENAMER
FILENAMER := $(ONL)/tools/filenamer.py
endif

swi: FORCE
	$(ONL_V_at) rm -rf *.swi* manifest.json
	$(ONL_V_at) $(ONLPM) $(LINK_OPTIONS)
	$(ONL_V_at) zip -n rootfs-$(ARCH).sqsh - rootfs-$(ARCH).sqsh manifest.json > $@
	$(ONL_V_at) mv $@ `$(FILENAMER) --type swi --manifest manifest.json $@`
	$(ONL_V_at) for f in `ls *.swi`; do md5sum $$f > $$f.md5sum; done
	$(ONL_V_at) rm -rf rootfs-$(ARCH).sqsh
	$(ONL_V_at) ls *.swi
FORCE:
