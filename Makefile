############################################################
#
# Rudimentary work in progress.
#
############################################################
include $(ONL)/make/config.mk

all: amd64 ppc

onl-amd64 onl-x86 x86 x86_64 amd64:
	$(MAKE) -C packages/base ARCHES=amd64,all
	$(MAKE) -C builds/amd64/rootfs
	$(MAKE) -C builds/amd64/swi
	$(MAKE) -C builds/amd64/installer/legacy

onl-ppc ppc:
	$(MAKE) -C packages/base ARCHES=powerpc,all
	$(MAKE) -C builds/powerpc/rootfs
	$(MAKE) -C builds/powerpc/swi
	$(MAKE) -C builds/powerpc/installer/legacy

rpc rebuild:
	$(ONLPM) --rebuild-pkg-cache




