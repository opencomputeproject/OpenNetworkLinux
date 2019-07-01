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

MODULE := libonlp-module
include $(BUILDER)/standardinit.mk

DEPENDMODULES := AIM onlplib onlp cjson cjson_util sff IOF timer_wheel OS uCli ELS

LIBONLP_PLATFORM_DEFAULTS := ../onlp-platform-defaults/$(BUILD_DIR)/bin/libonlp-platform-defaults.so
LIBONLP_PLATFORM := ../onlp-platform/$(BUILD_DIR)/bin/libonlp-platform.so

include $(BUILDER)/dependmodules.mk

SHAREDLIB := libonlp.so
$(SHAREDLIB)_TARGETS := $(ALL_TARGETS)
LDFLAGS += -Wl,--version-script=$(ONL)/make/onlp.version
include $(BUILDER)/so.mk

.DEFAULT_GOAL := sharedlibs

GLOBAL_CFLAGS += -DAIM_CONFIG_INCLUDE_CTOR_DTOR=1
GLOBAL_CFLAGS += -DAIM_CONFIG_INCLUDE_MODULES_INIT=1
GLOBAL_CFLAGS += -DONLP_CONFIG_API_LOCK_GLOBAL_SHARED=1
GLOBAL_CFLAGS += -DONLP_CONFIG_INCLUDE_SHLOCK_GLOBAL_INIT=1
GLOBAL_CFLAGS += -DAIM_CONFIG_INCLUDE_PVS_SYSLOG=1
GLOBAL_CFLAGS += -DAIM_CONFIG_INCLUDE_DAEMONIZE=1
GLOBAL_CFLAGS += -DONLP_CONFIG_INCLUDE_UCLI=1
GLOBAL_CFLAGS += -DUCLI_CONFIG_INCLUDE_ELS_LOOP=1

GLOBAL_CFLAGS += -fPIC
GLOBAL_LINK_LIBS += -lpthread -ledit $(LIBONLP_PLATFORM) $(LIBONLP_PLATFORM_DEFAULTS)

include $(BUILDER)/targets.mk

.PHONY: deb
deb:
	$(MAKE) -C deb
