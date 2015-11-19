.PHONY: onl-loader-fit.itb onl-loader-fit.its

onl-loader-fit.itb:
	$(ONL)/tools/flat-image-tree.py --initrd onl-loader-initrd:powerpc,onl-loader-initrd-powerpc.cpio.gz --add-platform initrd --itb $@

onl-loader-fit.its:
	$(ONL)/tools/flat-image-tree.py --initrd onl-loader-initrd:powerpc,onl-loader-initrd-powerpc.cpio.gz --add-platform initrd --its $@

its: onl-loader-fit.its



