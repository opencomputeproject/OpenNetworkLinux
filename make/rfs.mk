############################################################
#
# Common rules for invoking the onlrfs script.
#
# See $(ONL)/tools/onlrfs.py
#
############################################################

ifndef ARCH
$(error $$ARCH must be specified)
endif

ifndef RFS_CONFIG
$(error $$RFS_CONFIG must be set to the RFS yaml configuration file)
endif

ifndef RFS_WORKDIR
RFS_WORKDIR := $(ONL_DEBIAN_SUITE)
endif

ifndef RFS_DIR
RFS_DIR := $(RFS_WORKDIR)/rootfs-$(ARCH).d
endif

ifndef RFS_CPIO
RFS_CPIO := $(RFS_WORKDIR)/rootfs-$(ARCH).cpio.gz
endif

ifndef RFS_SQUASH
RFS_SQUASH := $(RFS_WORKDIR)/rootfs-$(ARCH).sqsh
endif

RFS_COMMAND := $(ONL)/tools/onlrfs.py --arch $(ARCH) --config $(RFS_CONFIG) --dir $(RFS_DIR)

ifdef RFS_CPIO
RFS_COMMAND += --cpio $(RFS_CPIO)
endif

ifdef RFS_SQUASH
RFS_COMMAND += --squash $(RFS_SQUASH)
endif

ifndef RFS_MANIFEST
RFS_MANIFEST := etc/onl/rootfs/manifest.json
endif

LOCAL_MANIFEST := $(RFS_WORKDIR)/manifest.json

RFS: clean
	$(ONL_V_at) $(RFS_COMMAND)
	$(ONL_V_at) [ -f $(RFS_DIR)/$(RFS_MANIFEST) ] && sudo cp $(RFS_DIR)/$(RFS_MANIFEST) $(LOCAL_MANIFEST)

clean:
	$(ONL_V_at) sudo rm -rf $(RFS_WORKDIR)

show-packages:
	$(ONL_V_at) $(RFS_COMMAND) --show-packages

build-packages:
	$(ONL_V_at) $(RFS_COMMAND) --only-build-packages
