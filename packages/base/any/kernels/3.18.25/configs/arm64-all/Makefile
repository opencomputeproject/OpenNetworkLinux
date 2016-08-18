############################################################
#
# Default 3.18.25 configuration for arm64 platforms.
#
############################################################
THIS_DIR := $(abspath $(dir $(lastword $(MAKEFILE_LIST))))
include $(ONL)/make/config.mk

ifndef K_TARGET_DIR
K_TARGET_DIR := $(THIS_DIR)
endif

MODSYNCLIST_EXTRA = arch/arm64/include arch/arm64/Makefile arch/arm64/lib
K_PATCH_SERIES=series.arm64

include ../../kconfig.mk
K_CONFIG := arm64-all.config
K_BUILD_TARGET := Image Image.gz arm64-nxp-ls2080ardb-r0.dtb
K_COPY_SRC := arch/arm64/boot/Image
K_COPY_GZIP := 1
ifndef K_COPY_DST
K_COPY_DST := kernel-3.18.25-arm64-all.bin.gz
endif

export ARCH=arm64
DTS_LIST := arm64-nxp-ls2080ardb-r0

include $(ONL)/make/kbuild.mk
