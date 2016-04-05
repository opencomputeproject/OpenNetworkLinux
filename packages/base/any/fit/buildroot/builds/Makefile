ifndef ARCH
$(error $$ARCH not set)
endif

.PHONY: onl-buildroot-fit.itb onl-buildroot-fit.its

onl-buildroot-fit.itb:
	$(ONL)/tools/flat-image-tree.py --initrd onl-buildroot-initrd:$(ARCH),onl-buildroot-initrd-$(ARCH).cpio.gz --arch $(ARCH) --add-platform all --itb $@

onl-buildroot-fit.its:
	$(ONL)/tools/flat-image-tree.py --initrd onl-buildroot-initrd:$(ARCH),onl-buildroot-initrd-$(ARCH).cpio.gz --arch $(ARCH) --add-platform all --its $@

its: onl-buildroot-fit.its
