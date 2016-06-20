############################################################
# <bsn.cl fy=2015 v=onl>
#
#           Copyright 2015 Big Switch Networks, Inc.
#           Copyright 2016 Freescale Semiconductor, Inc.
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
# Default 3.18.25 configuration for powerpc64 platforms.
#
############################################################
THIS_DIR := $(abspath $(dir $(lastword $(MAKEFILE_LIST))))
include $(ONL)/make/config.mk

ifndef K_TARGET_DIR
K_TARGET_DIR := $(THIS_DIR)
endif

include ../../kconfig.mk
K_CONFIG := powerpc64-all.config
K_BUILD_TARGET := uImage
K_COPY_SRC := vmlinux.bin.gz
ifndef K_COPY_DST
K_COPY_DST := kernel-3.18.25-powerpc64.bin.gz
endif

export ARCH=powerpc

DTS_LIST := powerpc-nxp-t2080rdb-r0

include $(ONL)/make/kbuild.mk
