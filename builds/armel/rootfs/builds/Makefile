include $(ONL)/make/config.armel.mk

ifndef PLATFORM_LIST
export PLATFORM_LIST=$(shell onlpm --list-platforms --arch armel --csv )
endif

RFS_CONFIG := $(ONL)/builds/any/rootfs/$(ONL_DEBIAN_SUITE)/standard/standard.yml

include $(ONL)/make/rfs.mk
