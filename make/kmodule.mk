ifndef KERNELS
$(error $$KERNELS must be set)
endif

ifndef KMODULES
$(error $$KMODULES must be set)
endif

ifndef PLATFORM
$(error $$PLATFORM must be set)
endif

modules:
	$(ONL)/tools/scripts/kmodbuild.sh "$(KERNELS)" "$(KMODULES)" $(PLATFORM)
