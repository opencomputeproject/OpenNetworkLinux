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

onl-amd64 onl-x86 x86 x86_64 amd64: packages_base_all
	$(MAKE) -C packages/base/amd64/kernels
	$(MAKE) -C packages/base/amd64/initrds
	$(MAKE) -C packages/base/amd64/onlp
	$(MAKE) -C packages/base/amd64/onlp-snmpd
	$(MAKE) -C packages/base/amd64/faultd
	$(MAKE) -C builds/amd64/rootfs
	$(MAKE) -C builds/amd64/swi
	$(MAKE) -C builds/amd64/installer/legacy

onl-ppc ppc: packages_base_all
	$(MAKE) -C packages/base/powerpc/kernels
	$(MAKE) -C packages/base/powerpc/initrds
	$(MAKE) -C packages/base/powerpc/onlp
	$(MAKE) -C packages/base/powerpc/onlp-snmpd
	$(MAKE) -C packages/base/powerpc/faultd
	$(MAKE) -C packages/base/powerpc/fit
	$(MAKE) -C builds/powerpc/rootfs
	$(MAKE) -C builds/powerpc/swi
	$(MAKE) -C builds/powerpc/installer/legacy


onl-arm arm: packages_base_all
	@which arm-linux-gnueabi-gcc || (echo -n " \n * This container does not support building for the ARM architecture. Please use the Jessie onlbuilder8:1.2 container or later." && echo -n && exit 1)
	$(MAKE) -C packages/base/armel/kernels
	$(MAKE) -C packages/base/armel/initrds
	$(MAKE) -C packages/base/armel/onlp
	$(MAKE) -C packages/base/armel/onlp-snmpd
	$(MAKE) -C packages/base/armel/faultd
	$(MAKE) -C packages/base/armel/fit
	$(MAKE) -C builds/armel/rootfs
	$(MAKE) -C builds/armel/swi

packages_base_all:
	$(MAKE) -C packages/base/all

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


versions:
	$(ONL)/tools/make-versions.py --import-file=$(ONL)/tools/onlvi --class-name=OnlVersionImplementation --output-dir $(ONL)/make --force
