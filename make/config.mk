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
export BUILD_DIR_BASE=BUILD


# Generate manifest if necessary
export MODULEMANIFEST := $(shell $(ONL)/tools/scripts/manifest.sh $(ONL) $(BUILDER))

#
# Default make options.
#
ONL_MAKE_FLAGS += --no-print-directory -s
ONL_MAKE := $(MAKE) $(ONL_MAKE_FLAGS)
ONL_V_at := @



