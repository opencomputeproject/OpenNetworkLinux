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
#
############################################################
include $(ONL)/make/config.amd64.mk

.DEFAULT_GOAL := onlpdump

MODULE := onlpdump
include $(BUILDER)/standardinit.mk

DEPENDMODULES := AIM IOF onlp x86_64_quanta_ly4r quanta_sys_eeprom onlplib onlp_platform_defaults sff cjson cjson_util timer_wheel OS

include $(BUILDER)/dependmodules.mk

BINARY := onlpdump
$(BINARY)_LIBRARIES := $(LIBRARY_TARGETS)
include $(BUILDER)/bin.mk

GLOBAL_CFLAGS += -DAIM_CONFIG_AIM_MAIN_FUNCTION=onlpdump_main
GLOBAL_CFLAGS += -DAIM_CONFIG_INCLUDE_MODULES_INIT=1
GLOBAL_CFLAGS += -DAIM_CONFIG_INCLUDE_MAIN=1
GLOBAL_LINK_LIBS += -lpthread -lm

include $(BUILDER)/targets.mk
