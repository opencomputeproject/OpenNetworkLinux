############################################################
# <bsn.cl fy=2015 v=onl>
#
#           Copyright 2015 Big Switch Networks, Inc.
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
THIS_DIR := $(abspath $(dir $(lastword $(MAKEFILE_LIST))))
include $(ONL)/make/config.mk

export ARCH := x86_64
ifndef K_TARGET_DIR
K_TARGET_DIR := $(THIS_DIR)
endif

include ../../kconfig.mk
K_CONFIG := x86_64-all.config
K_BUILD_TARGET := bzImage
K_COPY_SRC := arch/x86/boot/bzImage
ifndef K_COPY_DST
K_COPY_DST := kernel-4.14-lts-x86_64-all
endif

include $(ONL)/make/kbuild.mk
