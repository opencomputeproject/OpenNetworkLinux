include $(ONL)/make/config.mk

required_packages:
ifdef REQUIRED_PACKAGES
	$(ONLPM) --require $(REQUIRED_PACKAGES)
endif
