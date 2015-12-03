include $(ONL)/make/config.powerpc.mk

#
# Default to include all available powerpc platforms.
# You override this with you own list or yaml file.
#
export PLATFORM_LIST=$(shell onlpm --list-platforms --arch powerpc --csv )

RFS_CONFIG := $(ONL)/builds/any/rootfs/standard/standard.yml
RFS_DIR := rootfs-powerpc.d
RFS_CPIO := rootfs-powerpc.cpio.gz
RFS_SQUASH := rootfs-powerpc.sqsh

include $(ONL)/make/rfs.mk


