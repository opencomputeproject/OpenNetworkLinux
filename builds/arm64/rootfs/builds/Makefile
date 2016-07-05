include $(ONL)/make/config.arm64.mk

#
# Default to include all available powerpc platforms.
# You override this with you own list or yaml file.
#
export PLATFORM_LIST=$(shell onlpm --list-platforms --arch arm64 --csv )

RFS_CONFIG := $(ONL)/builds/any/rootfs/$(ONL_DEBIAN_SUITE)/standard/standard.yml
RFS_DIR := rootfs-arm64.d
RFS_CPIO := rootfs-arm64.cpio.gz
RFS_SQUASH := rootfs-arm64.sqsh

include $(ONL)/make/rfs.mk
