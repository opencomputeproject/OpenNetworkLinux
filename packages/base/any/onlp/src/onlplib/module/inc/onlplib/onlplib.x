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
 *
 ***********************************************************/

/* <auto.start.xmacro(ALL).define> */
#ifdef ONIE_TLV_ENTRY
ONIE_TLV_ENTRY(product_name,     Product Name,     0x21, str)
ONIE_TLV_ENTRY(part_number,      Part Number,      0x22, str)
ONIE_TLV_ENTRY(serial_number,    Serial Number,    0x23, str)
ONIE_TLV_ENTRY(mac,              MAC,              0x24, mac)
ONIE_TLV_ENTRY(manufacture_date, Manufacture Date, 0x25, str)
ONIE_TLV_ENTRY(device_version,   Device Version,   0x26, byte)
ONIE_TLV_ENTRY(label_revision,   Label Revision,   0x27, str)
ONIE_TLV_ENTRY(platform_name,    Platform Name,    0x28, str)
ONIE_TLV_ENTRY(onie_version,     ONIE Version,     0x29, str)
ONIE_TLV_ENTRY(mac_range,        MAC Range,        0x2A, int16)
ONIE_TLV_ENTRY(manufacturer,     Manufacturer,     0x2B, str)
ONIE_TLV_ENTRY(country_code,     Country Code,     0x2C, str)
ONIE_TLV_ENTRY(vendor,           Vendor,           0x2D, str)
ONIE_TLV_ENTRY(diag_version,     Diag Version,     0x2E, str)
ONIE_TLV_ENTRY(service_tag,      Service Tag,      0x2F, str)
#undef ONIE_TLV_ENTRY
#endif
/* <auto.end.xmacro(ALL).define> */

/* <--auto.start.xenum(ALL).define> */
/* <auto.end.xenum(ALL).define> */


