############################################################
#
# Open Network Linux
#
############################################################
include $(ONL)/make/config.mk
export TOOLCHAIN := x86_64-linux-gnu
export CROSS_COMPILER := $(TOOLCHAIN)-
export ARCH := amd64
export UARCH := AMD64
export ARCH_BOOT := grub
export __$(ARCH)__ := 1
