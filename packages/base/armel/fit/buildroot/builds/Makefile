.PHONY: onl-buildroot-fit.itb onl-buildroot-fit.its

onl-buildroot-fit.itb:
	$(ONL)/tools/flat-image-tree.py --initrd onl-buildroot-initrd:armel,onl-buildroot-initrd-armel.cpio.gz --arch armel --add-platform all --itb $@

onl-buildroot-fit.its:
	$(ONL)/tools/flat-image-tree.py --initrd onl-buildroot-initrd:armel,onl-buildroot-initrd-armel.cpio.gz --arch armel --add-platform all --its $@

its: onl-buildroot-fit.its
