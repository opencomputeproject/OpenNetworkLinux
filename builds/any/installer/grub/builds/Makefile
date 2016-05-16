ifndef ARCH
$(error $$ARCH not set)
endif

ONLPLATFORM = python $(ONL)/tools/onlplatform.py
PLATFORMS := $(shell $(ONLPM) --platform-manifest onl-loader-initrd:$(ARCH))

MKSHAR = $(ONL)/tools/mkshar
MKSHAR_OPTS = --lazy --unzip-pad --fixup-perms autoperms.sh
MKSHAR_PERMS = autoperms.sh

# Hardcoded to match ONL File naming conventions.
include $(ONL)/make/version-onl.mk
INSTALLER_NAME=$(FNAME_PRODUCT_VERSION)_ONL-OS_$(FNAME_BUILD_ID)_$(UARCH)_INSTALLER

ifeq ($(ARCH), amd64)
INSTALLER_ARCH = x86_64
else
INSTALLER_ARCH = $(ARCH)
endif

__installer: __installer_platform_files __installer_swi_files
	$(ONL_V_at)rm -rf *INSTALLER* *.md5sum
	$(ONL_V_at)cp /dev/null installer.sh
	$(ONL_V_at): ;\
	set -e ;\
	if $(ONL_V_P); then set -x; fi ;\
	set dummy *.cpio.gz; initrd="$$2" ;\
	sed \
	  -e 's^@ONLVERSION@^$(VERSION_STRING)^g' \
	  -e "s^@INITRD_ARCHIVE@^$$initrd^g" \
	  -e 's^@INITRD_OFFSET@^^g' \
	  -e 's^@INITRD_SIZE@^^g' \
	  -e 's^@ARCH@^$(INSTALLER_ARCH)^g' \
	  $(ONL)/builds/any/installer/installer.sh.in \
	>> installer.sh
	$(ONL_V_at)echo "PAYLOAD_FOLLOWS" >> installer.sh
	$(ONL_V_at)cp /dev/null $(MKSHAR_PERMS)
	$(ONL_V_at)cp $(ONL)/make/version-onl.sh .
	$(ONL_V_at)echo "#!/bin/sh" >> $(MKSHAR_PERMS)
	$(ONL_V_at)echo "set -e" >> $(MKSHAR_PERMS)
	$(ONL_V_at)echo "set -x" >> $(MKSHAR_PERMS)
	$(MKSHAR) $(MKSHAR_OPTS) "$(INSTALLER_NAME)" $(ONL)/tools/scripts/sfx.sh.in installer.sh kernel-* onl-loader-initrd-* *.swi version-onl.sh boot-config
	$(ONL_V_at)rm -rf installer.sh kernel-* onl-loader-initrd-* $(ZTN_MANIFEST) *.swi version-onl.sh autoperms.sh
	md5sum "$(INSTALLER_NAME)" | awk '{ print $$1 }' > "$(INSTALLER_NAME).md5sum"

__installer_platform_files:
	$(ONL_V_GEN): ;\
	set -e ;\
	if $(ONL_V_P); then set -x; fi ;\
	l="$(PLATFORMS)"; for p in $$l; do \
	  src=$$($(ONLPLATFORM) $$p $(ARCH) kernel 2>/dev/null) || : ;\
	  if test "$$src"; then \
	    dst=$${src##*/} ;\
	    if test "$dst" -ot Makefile; then \
	      : ;\
	    else \
	      echo "Staging $$dst for $$p" ;\
	      cp "$$src" "$$dst" ;\
	    fi ;\
	  fi ;\
	  src=$$($(ONLPLATFORM) $$p $(ARCH) initrd 2>/dev/null) || : ;\
	  if test "$$src"; then \
	    dst=$${src##*/} ;\
	    if test "$dst" -ot Makefile; then \
	      : ;\
	    else \
	      echo "Staging $$dst for $$p" ;\
	      cp "$$src" "$$dst" ;\
	    fi ;\
	  fi ;\
	done ;\
	:

ifndef NO_SWI
__installer_swi_files:
	$(ONL_V_GEN): ;\
	set -e ;\
	if $(ONL_V_P); then set -x; fi ;\
	swidir=$$(mktemp -d $(PWD)/swi-d-XXXXXX) ;\
	$(ONLPM) --extract-dir onl-swi:$(ARCH) $$swidir ;\
	mv $$swidir/usr/share/onl/packages/$(ARCH)/onl-swi/*.swi . ;\
	rm -fr $$swidir ;\
	:
else
__installer_swi_files:
	$(ONL_V_GEN):
endif

shar installer: installer

clean:
	rm -f *.swi *.installer $(notdir $(KERNELS)) *initrd*.cpio.gz
