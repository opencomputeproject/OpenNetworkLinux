include $(ONL)/make/config.mk
ifndef TOOLCHAIN
$(error $$TOOLCHAIN must be specified.)
else
export TOOLCHAIN
endif

ifndef ARCH
$(error $$ARCH must be specified.)
else
export ARCH
endif

