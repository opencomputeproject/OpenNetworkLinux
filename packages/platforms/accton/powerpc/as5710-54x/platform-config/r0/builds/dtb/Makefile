include $(ONL)/make/dtbs.mk

#
# The 5710 DTS relies on the common FSL devices trees. These are linked here from the kernel package.
#
setup::
	onlpm --link-dir onl-kernel-3.8.13-powerpc-e500mc:powerpc /usr/share/onl/packages/powerpc/onl-kernel-3.8.13-powerpc-e500mc/mbuilds/source/arch/powerpc/boot/dts/fsl fsl

clean::
	rm -rf fsl
