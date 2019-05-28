include $(ONL)/make/config.amd64.mk

$(eval $(call onlpm_find_file,KERNEL_3_16,onl-kernel-3.16-lts-x86-64-all:amd64,kernel-3.16-lts-x86_64-all))
$(eval $(call onlpm_find_file,KERNEL_4_9,onl-kernel-4.9-lts-x86-64-all:amd64,kernel-4.9-lts-x86_64-all))
$(eval $(call onlpm_find_file,KERNEL_4_14,onl-kernel-4.14-lts-x86-64-all:amd64,kernel-4.14-lts-x86_64-all))

# All amd64 kernels
KERNELS := $(KERNEL_3_16) $(KERNEL_4_9) $(KERNEL_4_14)

# Loader initrd
$(eval $(call onlpm_find_file,INITRD,onl-loader-initrd:amd64,onl-loader-initrd-amd64.cpio.gz))
$(eval $(call onlpm_find_file,MANIFEST,onl-loader-initrd:amd64,manifest.json))

all:
	mkdir -p files
	cp $(KERNELS) files
	cp $(INITRD) files
	cp $(MANIFEST) files
