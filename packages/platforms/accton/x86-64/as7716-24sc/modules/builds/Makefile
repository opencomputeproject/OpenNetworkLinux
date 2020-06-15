KERNELS := onl-kernel-4.14-lts-x86-64-all:amd64 onl-kernel-5.4-lts-x86-64-all:amd64
KMODULES := $(wildcard *.c)
VENDOR := accton
BASENAME := x86-64-accton-as7716-24sc
ARCH := x86_64
#TODO load symbols of accton_i2c_cpld driver
export KBUILD_MODPOST_WARN=1
include $(ONL)/make/kmodule.mk
