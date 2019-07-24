
#
# Finds a file in a package and assigns it to the given variable.
# Produces an evaluation-time error if the file could not be found.
#
# Example:
#
# $(eval $(call onlpm_find_file,KERNEL_4_14,onl-kernel-4.14-lts-x86-64-all:amd64,kernel-4.14-lts-x86_64-all))
#
define onlpm_find_file
$(1) := $$(shell $(ONLPM) --find-file $(2) $(3))
ifeq ($$($(1)),)
$$(error $(2):$(3) not found)
endif
endef

#
# The --find-dir equivalent.
#
define onlpm_find_dir
$(1) := $$(shell $(ONLPM) --find-dir $(2) $(3))
ifeq ($$($(1)),)
$$(error $(2):$(3) not found)
endif
endef


#
# Just like onlpm_find_{file,dir} but also add the result to another variable for easier aggregation.
#
define onlpm_find_file_add
$(1) := $$(shell $(ONLPM) --find-file $(2) $(3))
ifeq ($$($(1)),)
$$(error $(2):$(3) not found)
endif
$(4) := $$($(4)) $$($(1))
endef

define onlpm_find_dir_add
$(1) := $$(shell $(ONLPM) --find-file $(2) $(3))
ifeq ($$($(1)),)
$$(error $(2):$(3) not found)
endif
$(4) := $$($(4)) $$($(1))
endef
