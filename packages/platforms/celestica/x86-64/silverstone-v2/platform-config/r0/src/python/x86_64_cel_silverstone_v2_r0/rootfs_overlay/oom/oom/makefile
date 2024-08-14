#-------------------------------------------------------------------------------
#>
#
# Copyright (C) 2015-2016 Don Bollinger <don@thebollingers.org>
# Copyright (C) 2016 Carlos Cardenas <carlos@cumulusnetworks.com>
#
# SPDX-License-Identifier:     MIT
#
# Builds the Common Module Interface library
#
# make [SHIM={file, ethtool, aardvark, sysfs}] [V=] <target>
# -> Select the SHIM you want, default is 'file'
# -> V is for verbose.  If V is not zero, makefile is verbose.
#
#<

# Don't move this, it must be in FRONT of any included makefiles
THIS_MAKEFILE = $(realpath $(firstword $(MAKEFILE_LIST)))

# Allow users to override any ?= variables early
-include local.make

#-------------------------------------------------------------------------------
#
# Setup
#

SHELL   = bash
V ?= 0
Q = @
ifneq ($V,0)
    Q =
endif

#-------------------------------------------------------------------------------
#
# help (the default target)
#

.SUFFIXES:

PHONY += help
help:
	$(Q) sed -n -e "/^#>/,/^#</{s/^#[ <>]*//;s/\.PHONY *://;p}" $(THIS_MAKEFILE)
	$(Q) echo ""
	$(Q) echo "TARGETS"
	$(Q) for I in $(sort $(PHONY)); do echo "    $$I"; done
	$(Q) echo ""

#-------------------------------------------------------------------------------
#
# local vars
#

LIBDIR      = $(abspath ./lib)
STAMPDIR    = $(abspath ./stamps)
MODULEDIR   = $(abspath ./module_data)

PROJECT_STAMP = $(STAMPDIR)/stamp-project
project-stamp: $(PROJECT_STAMP)
$(PROJECT_STAMP):
	$(Q) mkdir -pv $(LIBDIR) $(STAMPDIR)
	$(Q) touch $@

MOD_INTERNAL_STAMP = $(STAMPDIR)/internal-stamp
MOD_SOUTH_STAMP    = $(STAMPDIR)/southbound-stamp
MOD_DRIVER_STAMP   = $(STAMPDIR)/driver-stamp
MOD_DATA_STAMP     = $(STAMPDIR)/data-stamp
#-------------------------------------------------------------------------------
#
# vars -- Usually specified on command line
#
SHIM      ?= file

# Verify shim type
ifneq ($(filter-out file ethtool aardvark sysfs, $(SHIM)),)
    $(warning Unsupported southbound interface: $(SHIM))
    $(error Supported interfaces: file ethtool aardvark sysfs)
endif


#-------------------------------------------------------------------------------
#
# top level targets
#

PHONY += all library driver test-data clean

all: library driver test-data
	$(Q) echo "=== Finished making $@ ==="

library: $(PROJECT_STAMP) $(MOD_INTERNAL_STAMP) $(MOD_SOUTH_STAMP)
	$(Q) echo "=== Finished making $@ ==="

$(MOD_INTERNAL_STAMP): oom_internal.c oom_internal.h
	$(Q) rm -f $@
	$(Q) echo "==== Compiling Internal Pieces ===="
	$(Q) gcc -Wall -fPIC -c oom_internal.c
	$(Q) touch $@

$(MOD_SOUTH_STAMP): oom_south.h oom_$(SHIM).c aardvark.c
	$(Q) rm -f $@
	$(Q) echo "==== Compiling Library Shim: $(SHIM) ===="
	$(Q) gcc -Wall -fPIC -c oom_$(SHIM).c
ifeq ($(SHIM), aardvark)
		$(Q) cp -f aardvark.dll $(LIBDIR)
		$(Q) gcc -Wall -c aardvark.c
		$(Q) gcc -shared oom_internal.o aardvark.o oom_$(SHIM).o \
				-o $(LIBDIR)/oom_south.so
else
		$(Q) gcc -shared oom_internal.o oom_$(SHIM).o -o $(LIBDIR)/oom_south.so
endif
	$(Q) touch $@

driver: library $(MOD_DRIVER_STAMP)
$(MOD_DRIVER_STAMP): oomsouth_driver.c
	$(Q) rm -f $@
	$(Q) echo "==== Compiling Driver Shim: $(SHIM) ===="
	$(Q) gcc -Wall -c oomsouth_driver.c
ifeq ($(SHIM), aardvark)
	$(Q) gcc oomsouth_driver.o oom_internal.o oom_$(SHIM).o \
		aardvark.o -o oomsouth_driver.exe
else
	$(Q) gcc oomsouth_driver.o oom_internal.o oom_$(SHIM).o \
		-o oomsouth_driver.exe
endif
	$(Q) touch $@

test-data: $(MOD_DATA_STAMP)
$(MOD_DATA_STAMP): $(PROJECT_STAMP)
	$(Q) rm -f $@
	$(Q) echo "==== Copying Module Data ===="
	$(Q) cp $(MODULEDIR)/MUP0WB0_EEPROM_20160108_192637.txt $(MODULEDIR)/0.A0
	$(Q) cp $(MODULEDIR)/MUP0WB0_FCCABY_20160108_192637.txt $(MODULEDIR)/0.pages
	$(Q) cp $(MODULEDIR)/MUQ1BZB_EEPROM_20160108_192514.txt $(MODULEDIR)/2.A0
	$(Q) cp $(MODULEDIR)/MUQ1BZB_FCCABY_20160108_192514.txt $(MODULEDIR)/2.pages
	$(Q) cp $(MODULEDIR)/qsfpdatafile.txt $(MODULEDIR)/5.A0
	$(Q) cp $(MODULEDIR)/qsfp28datafile.txt $(MODULEDIR)/4.A0
	$(Q) touch $@

clean:
	$(Q) rm -rf $(LIBDIR) $(STAMPDIR)
	$(Q) rm -f *.pyc *.o oomsouth_driver.exe
	$(Q) rm -f $(MODULEDIR)/*.A0 $(MODULEDIR)/*.pages
	$(Q) echo "=== Finished making $@ ==="

.PHONY: $(PHONY)
