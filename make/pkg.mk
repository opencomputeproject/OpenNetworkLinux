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
ARCHES := amd64 powerpc arm all
endif

pkgall:
	$(ONL_V_at) MAKE=$(MAKE) onlpm.py --build all --arches $(ARCHES)


clean:
	$(ONL_V_at) MAKE=$(MAKE) onlpm.py --clean all --arches $(ARCHES)

rebuild:
	$(ONL_V_at) MAKE=$(MAKE) onlpm.py --rebuild-pkg-cache

#
# Check all package declarations.
#
check:
	$(ONL_V_at) MAKE=$(MAKE) onlpm.py --packagedirs=`pwd` --no-pkg-cache

subdir:
	$(ONL_V_at) MAKE=$(MAKE) onlpm.py --packagedirs=`pwd` --build all --no-pkg-cache


#
# Package construction only (no build step)
#
pkg:
	$(ONL_V_at) MAKE=$(MAKE) NOBUILD=1 onlpm.py --build all --arches $(ARCHES)



#
# Generate a rule for all available packages
#
ALL_PACKAGES := $(shell onlpm.py --list-all)
define package_build_template
package-$(1):
	$(ONL_V_at) onlpm.py --build $(2)

endef
$(foreach p,$(ALL_PACKAGES),$(eval $(call package_build_template,$(subst :,_,$(p)),$(p))))
