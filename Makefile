###########################################################
#
# Work in progress.
#
############################################################
# ONL defaults to repo root when setup.env hasn't been sourced (for clean/modclean)
ONL ?= $(CURDIR)

.PHONY: all rebuild modclean clean docker docker-debug docker_check versions relclean

ifneq ($(MAKECMDGOALS),docker)
ifneq ($(MAKECMDGOALS),docker-debug)
ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),modclean)

ifndef ONL
$(error Please source the setup.env script at the root of the ONL tree)
endif

include $(ONL)/make/config.mk

# All available architectures.
ALL_ARCHES := amd64 powerpc armel arm64 armhf

# Build rule for each architecture.
define build_arch_template
$(1) :
	$(MAKE) -C builds/$(1)
endef
$(foreach a,$(ALL_ARCHES),$(eval $(call build_arch_template,$(a))))


# Available build architectures based on the current suite
BUILD_ARCHES_wheezy := amd64 powerpc
BUILD_ARCHES_jessie := amd64 powerpc armel
BUILD_ARCHES_stretch := arm64 amd64 armel armhf

# Build available architectures by default.
.DEFAULT_GOAL := all
all: $(BUILD_ARCHES_$(ONL_DEBIAN_SUITE))


rebuild:
	$(ONLPM) --rebuild-pkg-cache

endif
endif
endif
endif

modclean:
	rm -rf $(ONL)/make/modules/modules.*

clean: modclean
	@echo "Cleaning ONL build artifacts..."
	rm -rf $(ONL)/RELEASE
	rm -rf $(ONL)/.pkg-cache
	find $(ONL)/REPO -mindepth 2 ! -name "Makefile" ! -name ".gitignore" -delete 2>/dev/null || true
	cp -R $(ONL)/sm/build-artifacts/REPO/* $(ONL)/REPO 2>/dev/null || true
	find $(ONL)/packages -name "*.deb" -delete 2>/dev/null || true
	find $(ONL)/packages -name "*.cpio.gz" -delete 2>/dev/null || true
	find $(ONL)/packages -name ".lock" -delete 2>/dev/null || true
	find $(ONL)/packages -name "manifest.json" -delete 2>/dev/null || true
	find $(ONL)/packages -type d -name "BUILD" -exec rm -rf {} + 2>/dev/null || true
	find $(ONL)/packages -type d -name "rootfs-*" -exec sudo rm -rf {} + 2>/dev/null || true
	find $(ONL)/builds -name "*.deb" -delete 2>/dev/null || true
	find $(ONL)/builds -name "*.cpio.gz" -delete 2>/dev/null || true
	find $(ONL)/builds -name ".lock" -delete 2>/dev/null || true
	find $(ONL)/builds -type d -name "rootfs-*" -exec sudo rm -rf {} + 2>/dev/null || true
	@echo "Clean complete!"

ifndef VERSION
VERSION := 9
endif

docker_check:
	@which docker > /dev/null || (echo "*** Docker appears to be missing. Please install docker.io in order to build OpenNetworkLinux." && exit 1)

docker: docker_check
	@docker/tools/onlbuilder -$(VERSION) --isolate --hostname onlbuilder$(VERSION) --pull --autobuild --non-interactive

# create an interactive docker shell, for debugging builds
docker-debug: docker_check
	@docker/tools/onlbuilder -$(VERSION) --isolate --hostname onlbuilder$(VERSION) --pull


versions:
	$(ONL)/tools/make-versions.py --import-file=$(ONL)/tools/onlvi --class-name=OnlVersionImplementation --output-dir $(ONL)/make/versions --force

relclean:
	@find $(ONL)/RELEASE -name "ONL-*" -delete
