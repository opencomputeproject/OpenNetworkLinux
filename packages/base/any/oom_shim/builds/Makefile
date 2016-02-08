include $(ONL)/make/any.mk

ifndef TOOLCHAIN
$(error Must define $$TOOLCHAIN to use this Makefile)
endif

ifndef RELEASE
$(warn Assuming RELEASE is wheezy -- FIXME)
RELEASE=wheezy
endif

PACKAGE=oom_shim

OUTDIR=$(ONL)/packages/base/$(ARCH)/$(PACKAGE)/builds/BUILD/$(RELEASE)/$(TOOLCHAIN)

VPATH=$(ONL)/packages/base/any/$(PACKAGE)/src

$(OUTDIR)/liboom_south.so: oom_shim.c
	@mkdir -p $(OUTDIR) || true
	@$(CC) -o $(OUTDIR)/liboom_south.so -shared -fPIC $+

.PHONY: deb
deb:
	$(MAKE) -C deb

clean:
	rm -rf $(OUTDIR)
