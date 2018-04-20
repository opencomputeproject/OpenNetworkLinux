KERNEL_BUILD_DIR := $(abspath $(dir $(lastword $(MAKEFILE_LIST))))
KERNEL_ARCH := x86_64
KERNEL_LTS_VERSION := 4.14
KERNEL_CONFIG := x86_64-all
include $(ONL)/packages/base/any/kernels/lts/builds/Makefile

setup::
	dpkg -l libelf-dev > /dev/null 2>&1 || sudo apt-get install libelf-dev
