ifndef ARCH
$(error $$ARCH must be set)
endif

.PHONY: onl-loader-fit.itb onl-loader-fit.its

onl-loader-fit.itb:
	$(ONL)/tools/flat-image-tree.py --initrd onl-loader-initrd:$(ARCH),onl-loader-initrd-$(ARCH).cpio.gz --arch $(ARCH) --add-platform initrd --itb $@
	$(ONLPM) --copy-file onl-loader-initrd:$(ARCH) manifest.json .

onl-loader-fit.its:
	$(ONL)/tools/flat-image-tree.py --initrd onl-loader-initrd:$(ARCH),onl-loader-initrd-$(ARCH).cpio.gz --arch $(ARCH) --add-platform initrd --its $@

its: onl-loader-fit.its
