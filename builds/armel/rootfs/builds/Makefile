include $(ONL)/make/config.armel.mk

#
# Default to include all available powerpc platforms.
# You override this with you own list or yaml file.
#
export PLATFORM_LIST=$(shell onlpm --list-platforms --arch armel --csv )

RFS_CONFIG := $(ONL)/builds/any/rootfs/$(ONL_DEBIAN_SUITE)/standard/standard.yml
RFS_DIR := rootfs-armel.d
RFS_CPIO := rootfs-armel.cpio.gz
RFS_SQUASH := rootfs-armel.sqsh

include $(ONL)/make/rfs.mk
