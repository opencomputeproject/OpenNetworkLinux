include $(ONL)/make/dtbs.mk

#
# The 4610 DTS relies on the common arm devices tree includes. These are linked here from the kernel package.
#
setup::
	onlpm --link-dir onl-kernel-3.2-lts-arm-iproc-all:armel /usr/share/onl/packages/armel/onl-kernel-3.2-lts-arm-iproc-all/mbuilds/arch/arm/boot/dts dts

clean::
	rm -rf fsl
