############################################################
# <bsn.cl fy=2014 v=onl>
#
#           Copyright 2014-2018 BigSwitch Networks, Inc.
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
include $(ONL)/packages/base/any/onlp/builds/platform/common.mk

MODULE := libonlp-$(PLATFORM)
include $(BUILDER)/standardinit.mk

DEPENDMODULES := AIM IOF onlplib $(DEPENDMODULES) $(PLATFORM_MODULE) $(EXTRA_MODULES)
DEPENDMODULE_HEADERS := $(DEPENDMODULE_HEADERS) sff

include $(BUILDER)/dependmodules.mk

SHAREDLIB := libonlp-$(PLATFORM).so
$(SHAREDLIB)_TARGETS := $(ALL_TARGETS)
include $(BUILDER)/so.mk
.DEFAULT_GOAL := $(SHAREDLIB)

GLOBAL_CFLAGS += -I$(onlp_BASEDIR)/module/inc
GLOBAL_CFLAGS += -DAIM_CONFIG_INCLUDE_MODULES_INIT=1
GLOBAL_CFLAGS += -fPIC
GLOBAL_LINK_LIBS += -lpthread

include $(BUILDER)/targets.mk
