############################################################
# <bsn.cl fy=2013 v=none>
#
#        Copyright 2013, 2014 BigSwitch Networks, Inc.
#
#
#
# </bsn.cl>
############################################################
#
# ONL base buildroot cpio.
#
############################################################
include $(ONL)/make/config.mk

#
# We build for these architectures
#
ARCHS := powerpc x86_64 arm arm64
BUILDROOT_ARCHDIRS := $(foreach a,$(ARCHS),buildroot-$(a))
BUILDROOT_SOURCE := buildroot-mirror

.PHONY: all clean setup $(BUILDROOT_ARCHDIRS)

all: setup $(BUILDROOT_ARCHDIRS)

clean:
	rm -rf $(BUILDROOT_ARCHDIRS)


setup: setup-pyroute2 setup-dnspython setup-libyaml setup-pyyaml setup-jq setup-pyparted
	cp $(wildcard patches/busybox*.patch) $(BUILDROOT_SOURCE)/package/busybox/
	cp $(wildcard patches/kexec*.patch) $(BUILDROOT_SOURCE)/package/kexec/
	cp $(wildcard patches/openssl*.patch) $(BUILDROOT_SOURCE)/package/openssl/
	rm -f $(BUILDROOT_SOURCE)/package/openssl/openssl-do-not-build-docs.patch
	cp $(wildcard patches/uboot*.patch) $(BUILDROOT_SOURCE)/package/uboot-tools/
	sed -i 's%^DOSFSTOOLS_SITE =.*%DOSFSTOOLS_SITE = http://downloads.openwrt.org/sources%' $(BUILDROOT_SOURCE)/package/dosfstools/dosfstools.mk
	sed -i 's%^UEMACS_SITE =.*%UEMACS_SITE = http://www.kernel.org/pub/linux/kernel/uemacs%;s%^UEMACS_SOURCE =.*%UEMACS_SOURCE = em-$$(UEMACS_VERSION).tar.gz%' $(BUILDROOT_SOURCE)/package/uemacs/uemacs.mk
	mkdir -p $(BUILDROOT_ARCHDIRS)
	$(foreach a,$(ARCHS),cp buildroot.config-$(a) buildroot-$(a)/.config ;)

setup-jq:
	mkdir -p $(BUILDROOT_SOURCE)/package/jq
	cp patches/jq.mk $(BUILDROOT_SOURCE)/package/jq/jq.mk
	cp patches/jq.Config.in $(BUILDROOT_SOURCE)/package/jq/Config.in
	sed -i '/[/]jq[/]/d' $(BUILDROOT_SOURCE)/package/Config.in
	sed -i '/[/]yajl[/]/a\source "package/jq/Config.in"' $(BUILDROOT_SOURCE)/package/Config.in

setup-pyroute2:
	mkdir -p $(BUILDROOT_SOURCE)/package/python-pyroute2
	cp patches/python-pyroute2.mk $(BUILDROOT_SOURCE)/package/python-pyroute2/python-pyroute2.mk
	cp patches/python-pyroute2.Config.in $(BUILDROOT_SOURCE)/package/python-pyroute2/Config.in
	sed -i '/[/]python-pyroute2[/]/d' $(BUILDROOT_SOURCE)/package/Config.in
	sed -i '/[/]python-setuptools[/]/a\source "package/python-pyroute2/Config.in"' $(BUILDROOT_SOURCE)/package/Config.in

setup-dnspython:
	mkdir -p $(BUILDROOT_SOURCE)/package/python-dnspython
	cp patches/python-dnspython.mk $(BUILDROOT_SOURCE)/package/python-dnspython/python-dnspython.mk
	cp patches/python-dnspython.Config.in $(BUILDROOT_SOURCE)/package/python-dnspython/Config.in
	sed -i '/[/]python-dnspython[/]/d' $(BUILDROOT_SOURCE)/package/Config.in
	sed -i '/[/]python-setuptools[/]/a\source "package/python-dnspython/Config.in"' $(BUILDROOT_SOURCE)/package/Config.in

setup-pyyaml:
	mkdir -p $(BUILDROOT_SOURCE)/package/python-yaml
	cp patches/python-yaml.mk $(BUILDROOT_SOURCE)/package/python-yaml/python-yaml.mk
	cp patches/python-yaml.Config.in $(BUILDROOT_SOURCE)/package/python-yaml/Config.in
	sed -i '/[/]python-yaml[/]/d' $(BUILDROOT_SOURCE)/package/Config.in
	sed -i '/[/]python-setuptools[/]/a\source "package/python-yaml/Config.in"' $(BUILDROOT_SOURCE)/package/Config.in
	sed -i 's/pyyaml\//pyyaml/g' $(BUILDROOT_SOURCE)/package/python-yaml/python-yaml.mk

setup-libyaml:
	sed -i.bak -e '/LIBYAML_INSTALL_TARGET/d' $(BUILDROOT_SOURCE)/package/libyaml/libyaml.mk
	sed -i.bak -e '/LIBYAML_VERSION/a\LIBYAML_INSTALL_TARGET = YES' $(BUILDROOT_SOURCE)/package/libyaml/libyaml.mk

setup-pyparted:
	mkdir -p $(BUILDROOT_SOURCE)/package/python-pyparted
	cp patches/python-pyparted.mk $(BUILDROOT_SOURCE)/package/python-pyparted/python-pyparted.mk
	cp patches/python-pyparted.Config.in $(BUILDROOT_SOURCE)/package/python-pyparted/Config.in
	cp patches/pyparted*.tar.bz2 $(BUILDROOT_SOURCE)/dl/.
	sed -i '/[/]python-pyparted[/]/d' $(BUILDROOT_SOURCE)/package/Config.in
	sed -i '/[/]python-setuptools[/]/a\source "package/python-pyparted/Config.in"' $(BUILDROOT_SOURCE)/package/Config.in


define buildroot_arch
buildroot-$(1):
	$(MAKE) -C $(BUILDROOT_SOURCE) O=../buildroot-$(1) LD_LIBRARY_PATH=/usr/buildroot/toolchains/$(1)/lib

buildroot-menuconfig-$(1):
	$(MAKE) -C $(BUILDROOT_SOURCE) menuconfig O=../buildroot-$(1)
	cp buildroot-$(1)/.config buildroot.config-$(1)
endef

$(foreach a,$(ARCHS),$(eval $(call buildroot_arch,$(a))))

busybox-menuconfig:
	$(MAKE) -C $(BUILDROOT_SOURCE) busybox-menuconfig O=../buildroot-powerpc
	cp buildroot-powerpc/build/busybox-*/.config busybox.config
