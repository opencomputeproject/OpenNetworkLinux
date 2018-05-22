############################################################
#
# Open Network Linux
#
############################################################
include $(ONL)/make/config.mk
export TOOLCHAIN := powerpc-linux-gnu
export CROSS_COMPILER := $(TOOLCHAIN)-
export ARCH := powerpc
export UARCH := PPC
export ARCH_BOOT := uboot
export __$(ARCH)__ := 1
