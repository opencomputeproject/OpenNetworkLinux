KERNEL_BUILD_DIR := $(abspath $(dir $(lastword $(MAKEFILE_LIST))))
KERNEL_ARCH := arm
KERNEL_LTS_VERSION := 4.14
KERNEL_CONFIG := arm-iproc-all
#NO_MODULE_BUILDS := 1
include $(ONL)/packages/base/any/kernels/lts/builds/Makefile
