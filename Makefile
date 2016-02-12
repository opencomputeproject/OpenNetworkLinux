############################################################
#
# Work in progress.
#
############################################################
ifneq ($(MAKECMDGOALS),docker)
ifneq ($(MAKECMDGOALS),docker-debug)

ifndef ONL
$(error Please source the setup.env script at the root of the ONL tree)
endif

include $(ONL)/make/config.mk

all: amd64 ppc
	$(MAKE) -C REPO build-clean

onl-amd64 onl-x86 x86 x86_64 amd64:
	$(MAKE) -C packages/base/amd64/kernels
	$(MAKE) -C packages/base/amd64/initrds
	$(MAKE) -C packages/base/amd64/onlp
	$(MAKE) -C packages/base/amd64/onlp-snmpd
	$(MAKE) -C packages/base/amd64/faultd
	$(MAKE) -C builds/amd64/rootfs
	$(MAKE) -C builds/amd64/swi
	$(MAKE) -C builds/amd64/installer/legacy

onl-ppc ppc:
	$(MAKE) -C packages/base/powerpc/kernels
	$(MAKE) -C packages/base/powerpc/initrds
	$(MAKE) -C packages/base/powerpc/onlp
	$(MAKE) -C packages/base/powerpc/onlp-snmpd
	$(MAKE) -C packages/base/powerpc/faultd
	$(MAKE) -C builds/powerpc/rootfs
	$(MAKE) -C builds/powerpc/swi
	$(MAKE) -C builds/powerpc/installer/legacy

rpc rebuild:
	$(ONLPM) --rebuild-pkg-cache

endif
endif


.PHONY: docker

ifndef VERSION
VERSION:=7
endif

docker_check:
	@which docker > /dev/null || (echo "*** Docker appears to be missing. Please install docker.io in order to build OpenNetworkLinux." && exit 1)

docker: docker_check
	@docker/tools/onlbuilder -$(VERSION) --isolate --hostname onlbuilder$(VERSION) --pull --autobuild --non-interactive

# create an interative docker shell, for debugging builds
docker-debug: docker_check
	@docker/tools/onlbuilder -$(VERSION) --isolate --hostname onlbuilder$(VERSION) --pull
