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

%.dtb: %.dts
	$(ONL)/tools/dtc -I dts -O dtb -o $@ $<

.DEFAULT_GOAL := $(DTB_LIST)

clean:
	rm -rf *.dtb
