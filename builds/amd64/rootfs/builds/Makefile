include $(ONL)/make/config.amd64.mk

ifndef PLATFORM_LIST
export PLATFORM_LIST=$(shell onlpm --list-platforms --arch amd64 --csv )
endif

RFS_CONFIG := $(ONL)/builds/any/rootfs/$(ONL_DEBIAN_SUITE)/standard/standard.yml

include $(ONL)/make/rfs.mk
