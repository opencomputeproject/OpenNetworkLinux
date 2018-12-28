include $(ONL)/make/config.armhf.mk

#
# Default to include all available powerpc platforms.
# You override this with you own list or yaml file.
#
ifndef PLATFORM_LIST
export PLATFORM_LIST=$(shell onlpm --list-platforms --arch armhf --csv )
endif

RFS_CONFIG := $(ONL)/builds/any/rootfs/$(ONL_DEBIAN_SUITE)/standard/standard.yml
RFS_DIR := rootfs-armhf.d
RFS_CPIO := rootfs-armhf.cpio.gz
RFS_SQUASH := rootfs-armhf.sqsh

include $(ONL)/make/rfs.mk
