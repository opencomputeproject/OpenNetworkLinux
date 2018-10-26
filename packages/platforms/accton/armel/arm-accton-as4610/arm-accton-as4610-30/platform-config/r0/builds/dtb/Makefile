INCLUDES=include .
KERNEL := onl-kernel-4.14-lts-arm-iproc-all:armel
DTS_LIST := arm-accton-as4610.dts
VPATH := ../../../../../src
include $(ONL)/make/dtbs.mk

#
# The 4610 DTS relies on the common arm devices tree includes. These are linked here from the kernel package.
#
setup::
	onlpm --link-dir onl-kernel-4.14-lts-arm-iproc-all:armel /usr/share/onl/packages/armel/onl-kernel-4.14-lts-arm-iproc-all/mbuilds/arch/arm/boot/dts dts
	onlpm --link-dir onl-kernel-4.14-lts-arm-iproc-all:armel /usr/share/onl/packages/armel/onl-kernel-4.14-lts-arm-iproc-all/mbuilds/include include

setup-clean::
	#rm -rf dts include
