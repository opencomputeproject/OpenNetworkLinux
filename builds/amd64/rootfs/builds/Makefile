include $(ONL)/make/config.amd64.mk

#
# Default to include all available amd64 platforms.
# You override this with you own list or yaml file.
#
ifndef PLATFORM_LIST
export PLATFORM_LIST=$(shell onlpm --list-platforms --arch amd64 --csv )
endif

RFS_CONFIG := $(ONL)/builds/any/rootfs/$(ONL_DEBIAN_SUITE)/standard/standard.yml
RFS_DIR := rootfs-amd64.d
RFS_CPIO := rootfs-amd64.cpio.gz
RFS_SQUASH := rootfs-amd64.sqsh

include $(ONL)/make/rfs.mk
