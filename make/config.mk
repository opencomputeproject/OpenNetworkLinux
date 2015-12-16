############################################################
#
# Open Network Linux Configuration Makefile
#
############################################################
ifndef ONL
  $(error $$ONL is not defined)
endif

ifndef ONLPM_PY
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

export BUILD_DIR_BASE=BUILD/$(ONL_DEBIAN_SUITE)


# Generate manifest if necessary
export MODULEMANIFEST := $(shell $(BUILDER)/tools/mmg.py --dirs $(ONL) $(ONLPM_OPTION_PACKAGEDIRS) --out $(ONL)/make/module-manifest.mk --only-if-missing make)

#
# Default make options.
#
ONL_MAKE_FLAGS += --no-print-directory -s
ONL_MAKE := $(MAKE) $(ONL_MAKE_FLAGS)
ONL_V_at := @
