############################################################
#
# Common DTB build rules.
#
############################################################
include $(ONL)/make/config.mk

ifndef DTS_LIST
DTS_LIST := $(wildcard *.dts)
endif

ifndef DTB_LIST
DTB_LIST := $(patsubst %.dts,%.dtb,$(DTS_LIST))
endif

ifndef DTC
  ifdef KERNEL
    DTC := $(shell $(ONLPM) --find-file $(KERNEL) dtc)
    ifeq ($(DTC),)
      $(error No device tree compiler.)
    endif
  else
    DTC := $(ONL)/tools/dtc
  endif
endif

%.dtb: %.dts
	cpp -nostdinc -undef -x assembler-with-cpp $(foreach inc,$(INCLUDES),-I$(inc) ) $< > $(notdir $<).i
	$(DTC) $(foreach inc,$(INCLUDES),-i$(inc) ) $(DTC_OPTIONS) -I dts -O dtb -o $@ $(notdir $<).i
	rm $(notdir $<).i

.DEFAULT_GOAL := dtbs

dtbs: $(DTB_LIST)
	echo $(DTB_LIST) $(VPATH)
	$(MAKE) setup-clean

$(DTB_LIST): setup

clean::
setup::
setup-clean::
