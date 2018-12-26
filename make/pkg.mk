############################################################
#
# pkg.mk
#
# Default rules for building packages.
#
# Build all package groups in the current subtree.
#
############################################################
include $(ONL)/make/config.mk

#
# Invoke onlpm to build all packages in the current
# directory tree.
#
ifndef ARCHES
ARCHES := amd64 powerpc armel armhf arm64 all
endif

ONLPM_ENVIRONMENT	= \
  MAKE=$(MAKE) \
  # THIS LINE INTENTIONALLY LEFT BLANK

ifneq ($(VERBOSE),1)
else
ONLPM_ENVIRONMENT += ONLPM_VERBOSE=1
endif

pkgall:
	$(ONL_V_GEN) $(ONLPM_ENVIRONMENT) $(ONLPM) $(ONLPM_OPTS) --build all --arches $(ARCHES)


clean:
	$(ONL_V_GEN) $(ONLPM_ENVIRONMENT) $(ONLPM) $(ONLPM_OPTS) --clean all --arches $(ARCHES)

rebuild:
	$(ONL_V_GEN) $(ONLPM_ENVIRONMENT) $(ONLPM) $(ONLPM_OPTS) --rebuild-pkg-cache

#
# Check all package declarations.
#
check:
	$(ONL_V_GEN) $(ONLPM_ENVIRONMENT) $(ONLPM) $(ONLPM_OPTS) --packagedirs=`pwd` --no-pkg-cache

subdir:
	$(ONL_V_GEN) $(ONLPM_ENVIRONMENT) $(ONLPM) $(ONLPM_OPTS) --packagedirs=`pwd` --build all --no-pkg-cache


#
# Package construction only (no build step)
#
pkg:
	$(ONL_V_GEN) $(ONLPM_ENVIRONMENT) NOBUILD=1 $(ONLPM) $(ONLPM_OPTS) --build all --arches $(ARCHES)



#
# Generate a rule for all available packages
#
ALL_PACKAGES := $(shell $(ONLPM_ENVIRONMENT) $(ONLPM) $(ONLPM_OPTS) --list-all)
define package_build_template
package-$(1):
	$(ONL_V_GEN) $(ONLPM_ENVIRONMENT) $(ONLPM) $(ONLPM_OPTS) --build $(2)

endef
$(foreach p,$(ALL_PACKAGES),$(eval $(call package_build_template,$(subst :,_,$(p)),$(p))))
