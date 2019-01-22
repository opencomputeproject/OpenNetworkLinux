INCLUDES=include .
KERNEL := onl-kernel-4.14-lts-armhf-iproc-all:armhf
DTS_LIST := arm-accton-as4610.dts
VPATH := $(ONL)/packages/platforms/accton/armxx/arm-accton-as4610
include $(ONL)/make/dtbs.mk

#
# The 4610 DTS relies on the common arm devices tree includes. These are linked here from the kernel package.
#
setup::
	onlpm --link-dir onl-kernel-4.14-lts-armhf-iproc-all:armhf /usr/share/onl/packages/armhf/onl-kernel-4.14-lts-armhf-iproc-all/mbuilds/arch/arm/boot/dts dts
	onlpm --link-dir onl-kernel-4.14-lts-armhf-iproc-all:armhf /usr/share/onl/packages/armhf/onl-kernel-4.14-lts-armhf-iproc-all/mbuilds/include include

setup-clean::
	rm -f dts include
