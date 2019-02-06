ifndef ARCH
$(error $$ARCH is not set)
endif

ifndef ROOTFS_PACKAGE
$(error $$ROOTFS_PACKAGE not set.)
endif

ifdef REBUILD_RFS
FORCE_OPTION = --force
endif

ifndef SWI_WORKDIR
SWI_WORKDIR := $(ONL_DEBIAN_SUITE)
endif


LINK_OPTIONS := $(FORCE_OPTION) --link-file $(ROOTFS_PACKAGE):$(ARCH) rootfs-$(ARCH).sqsh . --link-file $(ROOTFS_PACKAGE):$(ARCH) manifest.json .

ifndef FILENAMER
FILENAMER := $(ONL)/tools/filenamer.py
endif

swi: FORCE clean
	mkdir $(SWI_WORKDIR) && cd $(SWI_WORKDIR) && $(ONLPM) $(LINK_OPTIONS)
ifdef ONL_PRE_SWITOOL_CMDS
	$(ONL_V_at) $(ONL_PRE_SWITOOL_CMDS)
endif
	$(ONL_V_at) cd $(SWI_WORKDIR) && $(ONL)/tools/switool.py --create $(ONL_SWITOOL_EXTRA_ARGS) --rootfs rootfs-$(ARCH).sqsh --manifest manifest.json $@
	$(ONL_V_at) cd $(SWI_WORKDIR) && mv $@ `$(FILENAMER) --type swi --manifest manifest.json $@`
	$(ONL_V_at) cd $(SWI_WORKDIR) && for f in `ls *.swi`; do md5sum $$f > $$f.md5sum; done
	$(ONL_V_at) rm -rf $(SWI_WORKDIR)/rootfs-$(ARCH).sqsh
	$(ONL_V_at) ls $(SWI_WORKDIR)/*.swi
FORCE:

clean:
	rm -rf $(SWI_WORKDIR)
