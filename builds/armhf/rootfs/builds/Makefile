include $(ONL)/make/config.armhf.mk

ifndef PLATFORM_LIST
export PLATFORM_LIST=$(shell onlpm --list-platforms --arch armhf --csv )
endif

RFS_CONFIG := $(ONL)/builds/any/rootfs/$(ONL_DEBIAN_SUITE)/standard/standard.yml

include $(ONL)/make/rfs.mk
