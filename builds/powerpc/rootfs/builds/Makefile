include $(ONL)/make/config.powerpc.mk

ifndef PLATFORM_LIST
export PLATFORM_LIST=$(shell onlpm --list-platforms --arch powerpc --csv )
endif

RFS_CONFIG := $(ONL)/builds/any/rootfs/$(ONL_DEBIAN_SUITE)/standard/standard.yml

include $(ONL)/make/rfs.mk
