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

# init system options sysvinit, systemd. default is sysvinit
ifndef INIT
  export INIT := sysvinit
endif

# Use the new module database tool to resolve dependencies dynamically.
ifndef BUILDER_MODULE_DATABASE
export BUILDER_MODULE_DATABASE := $(ONL)/make/modules/modules.json
endif

# Regenerate the module manifest if necessary.
ifndef BUILDER_MODULE_DATABASE_ROOT
BUILDER_MODULE_DATABASE_ROOT := $(ONL)
endif

ifndef BUILDER_MODULE_MANIFEST
BUILDER_MODULE_MANIFEST := $(ONL)/make/modules/modules.mk
endif

export MODULEMANIFEST := $(shell $(BUILDER)/tools/modtool.py --db $(BUILDER_MODULE_DATABASE) --dbroot $(BUILDER_MODULE_DATABASE_ROOT) --make-manifest $(BUILDER_MODULE_MANIFEST))

# Generate versions if necessary.
$(shell $(ONL)/tools/make-versions.py --import-file=$(ONL)/tools/onlvi --class-name=OnlVersionImplementation --output-dir $(ONL)/make/versions)


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

include $(ONL)/make/templates.mk
