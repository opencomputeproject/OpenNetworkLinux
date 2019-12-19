# Generate Kbuild dynamically
OBJLIST=$(addsuffix .o, $(basename $(wildcard *.c)))
$(shell rm -f Kbuild)
$(foreach f, $(OBJLIST), $(file >> Kbuild, obj-m += $(f)))
# Settings used by ONL
KERNELS := onl-kernel-4.14-lts-x86-64-all:amd64
KMODULES :=  $(ONL)/packages/platforms/wnc/x86-64/x86-64-wnc-rseb-w1-32/modules/builds
VENDOR := wnc
BASENAME := x86-64-wnc-rseb-w1-32
ARCH := x86_64
include $(ONL)/make/kmodule.mk
