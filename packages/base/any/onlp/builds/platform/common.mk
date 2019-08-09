ifndef PLATFORM
$(error $$PLATFORM not set)
endif

ifndef ARCH
    # Determine ARCH from platform name
    ifeq ($(findstring x86-64-,$(PLATFORM)),x86-64-)
        ARCH := amd64
    endif

    ifeq ($(findstring powerpc-,$(PLATFORM)),powerpc-)
        ARCH := powerpc
    endif

    ifeq ($(findstring arm-,$(PLATFORM)),arm-)
        ARCH := armel
    endif

    ifeq ($(findstring arm64-,$(PLATFORM)),arm64-)
        ARCH := arm64
    endif
endif

ifndef ARCH
$(error $$ARCH not set and the architecture cannot be determined from the given platform name ($(PLATFORM)))
endif

ifndef PLATFORM_MODULE
PLATFORM_MODULE := $(subst -,_,$(PLATFORM))
endif

include $(ONL)/make/config.$(ARCH).mk
