# Generate Kbuild dynamically
OBJLIST=$(addsuffix .o, $(basename $(wildcard *.c)))
$(shell rm -f Kbuild)
$(foreach f, $(OBJLIST), $(file >> Kbuild, obj-m += $(f)))
# Settings used by ONL
KERNELS := onl-kernel-4.14-lts-x86-64-all:amd64
KMODULES := $(ONL)/packages/platforms/mitac/x86-64/x86-64-mitac-ly1200-b32h0-c3/modules/builds
VENDOR := mitac
BASENAME := x86-64-mitac-ly1200-b32h0-c3
ARCH := x86_64
include $(ONL)/make/kmodule.mk
