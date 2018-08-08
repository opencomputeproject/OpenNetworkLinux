# -*- Makefile -*-
############################################################
# <bsn.cl fy=2014 v=onl>
#
#           Copyright 2014 BigSwitch Networks, Inc.
#
# Licensed under the Eclipse Public License, Version 1.0 (the
# "License"); you may not use this file except in compliance
# with the License. You may obtain a copy of the License at
#
#        http://www.eclipse.org/legal/epl-v10.html
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
# either express or implied. See the License for the specific
# language governing permissions and limitations under the
# License.
#
# </bsn.cl>
############################################################
#
#
############################################################
include $(ONL)/make/any.mk

MODULE := oom_shim
include $(BUILDER)/standardinit.mk

DEPENDMODULES := AIM onlplib oom_shim cjson cjson_util IOF timer_wheel OS uCli
DEPENDMODULE_HEADERS := onlp sff

LIBONLP := $(shell $(ONLPM) --find-file onlp:$(ARCH) libonlp.so)

include $(BUILDER)/dependmodules.mk

SHAREDLIB := liboom-south.so
$(SHAREDLIB)_TARGETS := $(ALL_TARGETS)
include $(BUILDER)/so.mk

.DEFAULT_GOAL := sharedlibs

GLOBAL_CFLAGS += -DAIM_CONFIG_INCLUDE_CTOR_DTOR=1
GLOBAL_CFLAGS += -DAIM_CONFIG_INCLUDE_MODULES_INIT=1
GLOBAL_CFLAGS += -DONLP_CONFIG_API_LOCK_GLOBAL_SHARED=1
GLOBAL_CFLAGS += -DONLP_CONFIG_INCLUDE_SHLOCK_GLOBAL_INIT=1
GLOBAL_CFLAGS += -DUCLI_CONFIG_INCLUDE_ELS_LOOP=1
GLOBAL_CFLAGS += -DONLP_CONFIG_INCLUDE_UCLI=1

GLOBAL_CFLAGS += -fPIC
GLOBAL_LINK_LIBS += -lpthread $(LIBONLP)

include $(BUILDER)/targets.mk

.PHONY: deb
deb:
	$(MAKE) -C deb
