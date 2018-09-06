ifndef KERNELS
$(error $$KERNELS must be set)
endif

ifndef KMODULES
$(error $$KMODULES must be set)
endif

ifndef ARCH
$(error $$ARCH must be set)
endif

ifndef SUBDIR

ifndef VENDOR
$(error $$VENDOR must be set.)
endif

ifndef BASENAME
$(error $$BASENAME must be set.)
endif

SUBDIR := "onl/$(VENDOR)/$(BASENAME)"

endif

KERNELS := $(KERNELS)-mbuilds:$(ARCH)

ifeq ($(ARCH),amd64)
ARCH := x86_64
else
ifeq ($(ARCH),armel)
ARCH := arm
else
ifeq ($(ARCH),armhf)
ARCH := arm
endif
endif
endif

modules:
	rm -rf lib
	ARCH=$(ARCH) $(ONL)/tools/scripts/kmodbuild.sh "$(KERNELS)" "$(KMODULES)" "$(SUBDIR)" "$(KINCLUDES)"
