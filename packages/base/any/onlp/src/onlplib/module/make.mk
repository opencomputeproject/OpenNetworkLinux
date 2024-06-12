############################################################
# <bsn.cl fy=2014 v=onl>
#
#        Copyright 2014, 2015 Big Switch Networks, Inc.
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

THIS_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
onlplib_INCLUDES := -I $(THIS_DIR)inc
onlplib_INTERNAL_INCLUDES := -I $(THIS_DIR)src
onlplib_DEPENDMODULE_ENTRIES := init:onlplib

GLOBAL_LINK_LIBS += -lcurl
ifeq ($(ONL_DEBIAN_SUITE),buster)
GLOBAL_CFLAGS += -DONLPLIB_CONFIG_I2C_INCLUDE_SMBUS=1
GLOBAL_LINK_LIBS += -li2c
endif
