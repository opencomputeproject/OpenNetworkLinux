/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *        Copyright 2014, 2015 Big Switch Networks, Inc.
 *
 * Licensed under the Eclipse Public License, Version 1.0 (the
 * "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 *
 *        http://www.eclipse.org/legal/epl-v10.html
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific
 * language governing permissions and limitations under the
 * License.
 *
 * </bsn.cl>
 ************************************************************
 *
 *
 ***********************************************************/
#include <onlp/platformi/sfpi.h>
#include "onlp_platform_defaults_int.h"
#include "onlp_platform_defaults_log.h"

__ONLP_DEFAULTI_IMPLEMENTATION_OPTIONAL(onlp_sfpi_sw_init(void));
__ONLP_DEFAULTI_IMPLEMENTATION_OPTIONAL(onlp_sfpi_hw_init(uint32_t flags));
__ONLP_DEFAULTI_IMPLEMENTATION_OPTIONAL(onlp_sfpi_sw_denit(void));
__ONLP_DEFAULTI_IMPLEMENTATION(onlp_sfpi_bitmap_get(onlp_sfp_bitmap_t* bmap));
__ONLP_DEFAULTI_IMPLEMENTATION(onlp_sfpi_type_get(int port, onlp_sfp_type_t* rv));
__ONLP_DEFAULTI_IMPLEMENTATION(onlp_sfpi_is_present(int port));
__ONLP_DEFAULTI_IMPLEMENTATION(onlp_sfpi_presence_bitmap_get(onlp_sfp_bitmap_t* dst));
__ONLP_DEFAULTI_IMPLEMENTATION(onlp_sfpi_rx_los_bitmap_get(onlp_sfp_bitmap_t* dst));
__ONLP_DEFAULTI_IMPLEMENTATION(onlp_sfpi_post_insert(int port, sff_info_t* sff_info));
__ONLP_DEFAULTI_IMPLEMENTATION(onlp_sfpi_port_map(int port, int* rport));
__ONLP_DEFAULTI_IMPLEMENTATION(onlp_sfpi_denit(void));
__ONLP_DEFAULTI_VIMPLEMENTATION(onlp_sfpi_debug(int port, aim_pvs_t* pvs));
__ONLP_DEFAULTI_IMPLEMENTATION(onlp_sfpi_ioctl(int port, va_list vargs));
__ONLP_DEFAULTI_IMPLEMENTATION(onlp_sfpi_control_supported(int port, onlp_sfp_control_t control, int* rv));
__ONLP_DEFAULTI_IMPLEMENTATION(onlp_sfpi_control_set(int port, onlp_sfp_control_t control, int value));
__ONLP_DEFAULTI_IMPLEMENTATION(onlp_sfpi_control_get(int port, onlp_sfp_control_t control, int* value));
__ONLP_DEFAULTI_IMPLEMENTATION(onlp_sfpi_dev_read(int port, int devaddr, int addr, uint8_t* dst, int len));
__ONLP_DEFAULTI_IMPLEMENTATION(onlp_sfpi_dev_write(int port, int devaddr, int addr, uint8_t* dst, int len));
__ONLP_DEFAULTI_IMPLEMENTATION(onlp_sfpi_dev_readb(int port, int devaddr, int addr));
__ONLP_DEFAULTI_IMPLEMENTATION(onlp_sfpi_dev_writeb(int port, int devaddr, int addr, uint8_t value));
__ONLP_DEFAULTI_IMPLEMENTATION(onlp_sfpi_dev_readw(int port, int devaddr, int addr));
__ONLP_DEFAULTI_IMPLEMENTATION(onlp_sfpi_dev_writew(int port, int devaddr, int addr, uint16_t value));
__ONLP_DEFAULTI_IMPLEMENTATION(onlp_sfpi_hdr_get(onlp_oid_t oid, onlp_oid_hdr_t* hdr));
__ONLP_DEFAULTI_IMPLEMENTATION(onlp_sfpi_info_get(onlp_oid_t port, onlp_sfp_info_t* info));
