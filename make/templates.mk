define onlpm_find_file
$(1) := $$(shell $(ONLPM) --find-file $(2) $(3))
ifeq ($$($(1)),)
$$(error $(2):$(3) not found)
endif
endef
