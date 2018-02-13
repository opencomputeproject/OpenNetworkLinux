############################################################
# <bsn.cl fy=2016 v=onl>
#
#        Copyright 2016 NXP Semiconductor, Inc.
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
arm64_nxp_layerscape_INCLUDES := -I $(THIS_DIR)inc
arm64_nxp_layerscape_INTERNAL_INCLUDES := -I $(THIS_DIR)src
arm64_nxp_layerscape_DEPENDMODULE_ENTRIES := init:arm64_nxp_layerscape ucli:arm64_nxp_layerscape

