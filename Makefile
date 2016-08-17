###########################################################
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

all: amd64 ppc arm arm64
	$(MAKE) -C REPO build-clean

onl-amd64 onl-x86 x86 x86_64 amd64: packages_base_all
	$(MAKE) -C packages/base/amd64/kernels
	$(MAKE) -C packages/base/amd64/initrds
	$(MAKE) -C packages/base/amd64/onlp
	$(MAKE) -C packages/base/amd64/onlp-snmpd
	$(MAKE) -C packages/base/amd64/faultd
	$(MAKE) -C builds/amd64

onl-ppc ppc: packages_base_all
	$(MAKE) -C packages/base/powerpc/kernels
	$(MAKE) -C packages/base/powerpc/initrds
	$(MAKE) -C packages/base/powerpc/onlp
	$(MAKE) -C packages/base/powerpc/onlp-snmpd
	$(MAKE) -C packages/base/powerpc/faultd
	$(MAKE) -C packages/base/powerpc/fit
	$(MAKE) -C builds/powerpc

ifdef ONL_DEBIAN_SUITE_jessie

arm_toolchain_check:
	@which arm-linux-gnueabi-gcc || (/bin/echo -e "*\n* ERROR\n*\n* This container does not support building for the ARM architecture.\n* Please use opennetworklinux/onlbuilder8:1.2 later.\n*" && exit 1)

arm64_toolchain_check:
	@which aarch64-linux-gnu-gcc || (/bin/echo -e "*\n* ERROR\n*\n* This container does not support building for the ARM64 architecture.\n* Please use opennetworklinux/onlbuilder8:1.5 or later.\n*" && exit 1)

onl-arm arm: arm_toolchain_check packages_base_all
	$(MAKE) -C packages/base/armel/kernels
	$(MAKE) -C packages/base/armel/initrds
	$(MAKE) -C packages/base/armel/onlp
	$(MAKE) -C packages/base/armel/onlp-snmpd
	$(MAKE) -C packages/base/armel/faultd
	$(MAKE) -C packages/base/armel/fit
	$(MAKE) -C builds/armel

onl-arm64 arm64: arm64_toolchain_check packages_base_all
	$(MAKE) -C packages/base/arm64/kernels
	$(MAKE) -C packages/base/arm64/initrds
	$(MAKE) -C packages/base/arm64/onlp
	$(MAKE) -C packages/base/arm64/onlp-snmpd
	$(MAKE) -C packages/base/arm64/faultd
	$(MAKE) -C packages/base/arm64/fit
	$(MAKE) -C builds/arm64

else

onl-arm arm:
	@/bin/echo -e "*\n* Warning\n*\n* ARM Architecture support is only available in Jessie builds. Please use onbuilder -8.\n*"

onl-arm64 arm64:
	@/bin/echo -e "*\n* Warning\n*\n* ARM64 Architecture support is only available in Jessie builds. Please use onbuilder -8.\n*"

endif


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
	$(ONL)/tools/make-versions.py --import-file=$(ONL)/tools/onlvi --class-name=OnlVersionImplementation --output-dir $(ONL)/make/versions --force

relclean:
	@find $(ONL)/RELEASE -name "ONL-*" -delete
