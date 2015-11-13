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
#
# Default 3.2.65-1+deb7u2 configuration for PowerPC e500v platforms.
#
############################################################
THIS_DIR := $(abspath $(dir $(lastword $(MAKEFILE_LIST))))
include $(ONL)/make/config.mk

ifndef K_TARGET_DIR
K_TARGET_DIR := $(THIS_DIR)
endif

include ../../kconfig.mk
K_CONFIG := powerpc-e500v-all.config
K_BUILD_TARGET := uImage
K_COPY_SRC := vmlinux.bin.gz
ifndef K_COPY_DST
K_COPY_DST := kernel-3.2-deb7-powerpc-e500-all.bin.gz
endif

export ARCH=powerpc
DTS_LIST := powerpc-quanta-lb9-r0 powerpc-quanta-ly2-r0 powerpc-accton-as4600-54t-r0 powerpc-accton-as5610-52x-r0 powerpc-dni-7448-r0 powerpc-dell-s4810-p2020-r0

include $(ONL)/make/kbuild.mk

