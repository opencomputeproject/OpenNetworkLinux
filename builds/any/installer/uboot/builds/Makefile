ifndef ARCH
$(error $$ARCH not set)
endif

ONLPLATFORM = python $(ONL)/tools/onlplatform.py
PLATFORMS := $(shell $(ONLPM) --platform-manifest onl-loader-initrd:$(ARCH))

MKSHAR = $(ONL)/tools/mkshar
MKSHAR_OPTS = --lazy --unzip-pad --fixup-perms autoperms.sh
MKSHAR_PERMS = autoperms.sh

VONLDIR = $(ONL)/packages/base/all/vendor-config-onl
PYFIT = $(VONLDIR)/src/bin/pyfit
PYFIT_ENVIRONMENT = PYTHONPATH=$(VONLDIR)/src/python

# Hardcoded to match ONL File naming conventions.
include $(ONL)/make/version-onl.mk
INSTALLER_NAME=$(FNAME_PRODUCT_VERSION)_ONL-OS_$(FNAME_BUILD_ID)_$(UARCH)_INSTALLER

# default fit image can be used as the canonical location for the initrd
FIT_IMAGE_ALL := $(shell $(ONLPM) --find-file onl-loader-fit:$(ARCH) onl-loader-fit.itb)
INITRD := $(shell $(ONLPM) --find-file onl-loader-initrd:$(ARCH) onl-loader-initrd-$(ARCH).cpio.gz)
INITRD_BOUNDS := $(shell $(PYFIT_ENVIRONMENT) $(PYFIT) -v offset $(FIT_IMAGE_ALL) --initrd)

__installer: installer.sh __installer_fit_files __installer_platform_files __installer_swi_files
	$(ONL_V_at)rm -rf *INSTALLER* *.md5sum
	$(ONL_V_at)cp /dev/null $(MKSHAR_PERMS)
	$(ONL_V_at)cp $(ONL)/make/version-onl.sh .
	$(ONL_V_at)echo "#!/bin/sh" >> $(MKSHAR_PERMS)
	$(ONL_V_at)echo "set -e" >> $(MKSHAR_PERMS)
	$(ONL_V_at)echo "set -x" >> $(MKSHAR_PERMS)
	$(MKSHAR) $(MKSHAR_OPTS) "$(INSTALLER_NAME)" $(ONL)/tools/scripts/sfx.sh.in installer.sh *.swi *.itb version-onl.sh boot-config
	$(ONL_V_at)rm -rf installer.sh *.itb *.swi version-onl.sh autoperms.sh
	$(ONL_V_at)md5sum "$(INSTALLER_NAME)" | awk '{ print $$1 }' > "$(INSTALLER_NAME).md5sum"

installer.sh: Makefile $(ONL)/builds/any/installer/installer.sh.in
	$(ONL_V_GEN)cp /dev/null $@
	$(ONL_V_at): ;\
	set -e ;\
	if $(ONL_V_P); then set -x; fi ;\
	if test "$(INITRD_BOUNDS)"; then \
	  a="$(FIT_IMAGE_ALL)"; a=$${a##*/} ;\
	else \
	  a="$(INITRD)"; i=$${a##*/} ;\
	fi ;\
	set dummy $(INITRD_BOUNDS); start=$$2; end=$$3; sz=$$(($$end - $$start + 1)) ;\
	sed \
	  -e 's^@ONLVERSION@^$(VERSION_STRING)^g' \
	  -e "s^@INITRD_ARCHIVE@^$${a}^g" \
	  -e "s^@INITRD_OFFSET@^$$start^g" \
	  -e "s^@INITRD_SIZE@^$$sz^g" \
	  -e 's^@ARCH@^$(ARCH)^g' \
	  $(ONL)/builds/any/installer/installer.sh.in \
	>> $@
	$(ONL_V_at)echo "PAYLOAD_FOLLOWS" >> $@

__installer_fit_files:
	$(ONL_V_GEN): ;\
	set -e ;\
	if $(ONL_V_P); then set -x; fi ;\
	src=$(FIT_IMAGE_ALL) ;\
	dst=$${src##*/} ;\
	if test "$$dst" -nt Makefile; then \
	  : ;\
	else \
	  echo "Staging $$dst" ;\
	  cp $$src $$dst ;\
	fi ;\
	:

##############################
#
# optionally include custom itb files for each platform
#
##############################

__installer_platform_files:
	$(ONL_V_GEN): ;\
	set -e ;\
	if $(ONL_V_P); then set -x; fi ;\
	l="$(PLATFORMS)"; for p in $$l; do \
	  echo "Looking for an ITB specific to $$p, ignore errors..." ;\
	  src=$$($(ONLPLATFORM) $$p $(ARCH) itb) 2>/dev/null || : ;\
	  if test "$$src"; then :; else continue; fi ;\
	  dst=$${src##*/} ;\
	  echo "Found $$dst" ;\
	  if test "$$dst" -nt Makefile; then continue; fi ;\
	  echo "Staging $$dst for $$p" ;\
	  cp "$$src" "$$dst" ;\
	done ;\
	:

__installer_swi_files:
ifndef NO_SWI
	$(ONL_V_GEN): ;\
	set -e ;\
	if $(ONL_V_P); then set -x; fi ;\
	swidir=$$(mktemp -d $(PWD)/swi-d-XXXXXX) ;\
	$(ONLPM) --extract-dir onl-swi:$(ARCH) $$swidir ;\
	mv $$swidir/usr/share/onl/packages/$(ARCH)/onl-swi/*.swi . ;\
	rm -fr $$swidir ;\
	:
else
	$(ONL_V_GEN):
endif

shar installer: installer

clean:
	rm -f *.swi *.installer *.cpio.gz
