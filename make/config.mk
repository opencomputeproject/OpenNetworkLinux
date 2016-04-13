############################################################
#
# Open Network Linux Configuration Makefile
#
############################################################
ifndef ONL
  $(error $$ONL is not defined)
endif

ifndef ONLPM
  ONLPM := $(ONL)/tools/onlpm.py
endif

ifndef BUILDER
  # Builder is here
  export BUILDER := $(ONL)/sm/infra/builder/unix
endif

#
# The default build directory for all infra:builder compilations is the following.
#
ifndef ONL_DEBIAN_SUITE
$(error "The $$ONL_DEBIAN_SUITE value is not set. Please source $$ONL/setup.env")
endif
export ONL_DEBIAN_SUITE_$(ONL_DEBIAN_SUITE)=1

export BUILD_DIR_BASE=BUILD/$(ONL_DEBIAN_SUITE)

# Generate manifest if necessary
export MODULEMANIFEST := $(shell $(BUILDER)/tools/mmg.py --dirs $(ONL) $(ONLPM_OPTION_PACKAGEDIRS) --out $(ONL)/make/module-manifest.mk --only-if-missing make)

#
# Default make options.
#
ifeq ("$(origin V)", "command line")
VERBOSE := $(V)
endif

ifneq ($(VERBOSE),1)

# quiet settings
ONL_V_P := false
ONL_V_at := @
ONL_V_GEN = @set -e; echo GEN $@;

else

# verbose settings
ONL_V_P := :

endif

ifneq ($(VERBOSE),1)

ONL_MAKE_FLAGS += --no-print-directory -s

else

ONL_MAKE_FLAGS += V=1

endif

ONL_MAKE := $(MAKE) $(ONL_MAKE_FLAGS)

#
# Some build and autogen tools require these settings.
#
export SUBMODULE_INFRA := $(ONL)/sm/infra
export SUBMODULE_BIGCODE := $(ONL)/sm/bigcode


