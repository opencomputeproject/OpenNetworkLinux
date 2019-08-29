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

.DEFAULT_GOAL := onlps

MODULE := onlps
include $(BUILDER)/standardinit.mk

DEPENDMODULES := $(DEPENDMODULES) AIM IOF onlp onlplib $(PLATFORM_MODULE) $(EXTRA_MODULES) sff cjson cjson_util timer_wheel OS uCli ELS onlp_platform_defaults

include $(BUILDER)/dependmodules.mk

BINARY := onlps
$(BINARY)_LIBRARIES := $(LIBRARY_TARGETS)
include $(BUILDER)/bin.mk

GLOBAL_CFLAGS += -DAIM_CONFIG_AIM_MAIN_FUNCTION=onlpdump_main
GLOBAL_CFLAGS += -DAIM_CONFIG_INCLUDE_MODULES_INIT=1
GLOBAL_CFLAGS += -DAIM_CONFIG_INCLUDE_MAIN=1
GLOBAL_CFLAGS += -DUCLI_CONFIG_INCLUDE_ELS_LOOP=1
GLOBAL_CFLAGS += -DONLP_CONFIG_INCLUDE_UCLI=1
GLOBAL_LINK_LIBS += -lpthread -lm -ledit

include $(BUILDER)/targets.mk
