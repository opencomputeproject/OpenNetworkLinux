.PHONY: onl-buildroot-fit.itb onl-buildroot-fit.its

onl-buildroot-fit.itb:
	$(ONL)/tools/flat-image-tree.py --initrd onl-buildroot-initrd:powerpc,onl-buildroot-initrd-powerpc.cpio.gz --arch powerpc --add-platform all --itb $@

onl-buildroot-fit.its:
	$(ONL)/tools/flat-image-tree.py --initrd onl-buildroot-initrd:powerpc,onl-buildroot-initrd-powerpc.cpio.gz --arch powerpc --add-platform all --its $@

its: onl-buildroot-fit.its
