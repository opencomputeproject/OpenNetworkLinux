include $(ONL)/make/config.amd64.mk

# All amd64 kernels -> $FILES
$(eval $(call onlpm_find_file_add,X,onl-kernel-3.16-lts-x86-64-all:amd64,kernel-3.16-lts-x86_64-all,FILES))
$(eval $(call onlpm_find_file_add,X,onl-kernel-4.9-lts-x86-64-all:amd64,kernel-4.9-lts-x86_64-all,FILES))
$(eval $(call onlpm_find_file_add,X,onl-kernel-4.14-lts-x86-64-all:amd64,kernel-4.14-lts-x86_64-all,FILES))
$(eval $(call onlpm_find_file_add,X,onl-kernel-4.19-lts-x86-64-all:amd64,kernel-4.19-lts-x86_64-all,FILES))
$(eval $(call onlpm_find_file_add,X,onl-kernel-5.4-lts-x86-64-all:amd64,kernel-5.4-lts-x86_64-all,FILES))

# Loader initrd and manifest -> $FILES
$(eval $(call onlpm_find_file_add,X,onl-loader-initrd:amd64,onl-loader-initrd-amd64.cpio.gz,FILES))
$(eval $(call onlpm_find_file_add,X,onl-loader-initrd:amd64,manifest.json,FILES))

all:
	mkdir -p files
	cp $(FILES) files
