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

#include <onlp/onlp_config.h>


/* <auto.start.xmacro(ALL).define> */
#ifdef ONLP_ASSET_INFO_ENTRY
ONLP_ASSET_INFO_ENTRY(manufacturer,  Manufacturer,  str)
ONLP_ASSET_INFO_ENTRY(date,          Date,          str)
ONLP_ASSET_INFO_ENTRY(part_number,   Part Number,   str)
ONLP_ASSET_INFO_ENTRY(serial_number, Serial Number, str)
ONLP_ASSET_INFO_ENTRY(hardware_revision, Hardware Revision, str)
ONLP_ASSET_INFO_ENTRY(firmware_revision, Firmware Revision, str)
ONLP_ASSET_INFO_ENTRY(cpld_revision, CPLD Revision, str)
ONLP_ASSET_INFO_ENTRY(manufacture_date, Manufacter Date, str)
ONLP_ASSET_INFO_ENTRY(description, Description, str)
ONLP_ASSET_INFO_ENTRY(additional, Additional, str)
#undef ONLP_ASSET_INFO_ENTRY
#endif

#ifdef ONLP_OID_TYPE_ENTRY
ONLP_OID_TYPE_ENTRY(CHASSIS, 1, CHASSIS, chassis)
ONLP_OID_TYPE_ENTRY(MODULE, 2, MODULE, module)
ONLP_OID_TYPE_ENTRY(THERMAL, 3, THERMAL, thermal)
ONLP_OID_TYPE_ENTRY(FAN, 4, FAN, fan)
ONLP_OID_TYPE_ENTRY(PSU, 5, PSU, psu)
ONLP_OID_TYPE_ENTRY(LED, 6, LED, led)
ONLP_OID_TYPE_ENTRY(SFP, 7, SFP, sfp)
ONLP_OID_TYPE_ENTRY(GENERIC, 8, GENERIC, generic)
#undef ONLP_OID_TYPE_ENTRY
#endif
/* <auto.end.xmacro(ALL).define> */

/* <auto.start.xenum(ALL).define> */
#ifdef ONLP_ENUMERATION_ENTRY
ONLP_ENUMERATION_ENTRY(onlp_fan_caps, "")
ONLP_ENUMERATION_ENTRY(onlp_fan_dir, "")
ONLP_ENUMERATION_ENTRY(onlp_led_caps, "")
ONLP_ENUMERATION_ENTRY(onlp_led_mode, "")
ONLP_ENUMERATION_ENTRY(onlp_log_flag, "")
ONLP_ENUMERATION_ENTRY(onlp_oid_json_flag, "")
ONLP_ENUMERATION_ENTRY(onlp_oid_status_flag, "")
ONLP_ENUMERATION_ENTRY(onlp_oid_type, "")
ONLP_ENUMERATION_ENTRY(onlp_oid_type_flag, "")
ONLP_ENUMERATION_ENTRY(onlp_psu_caps, "")
ONLP_ENUMERATION_ENTRY(onlp_psu_type, "")
ONLP_ENUMERATION_ENTRY(onlp_sfp_control, "")
ONLP_ENUMERATION_ENTRY(onlp_sfp_control_flag, "")
ONLP_ENUMERATION_ENTRY(onlp_sfp_type, "")
ONLP_ENUMERATION_ENTRY(onlp_status, "")
ONLP_ENUMERATION_ENTRY(onlp_thermal_caps, "")
ONLP_ENUMERATION_ENTRY(onlp_thermal_threshold, "")
#undef ONLP_ENUMERATION_ENTRY
#endif
/* <auto.end.xenum(ALL).define> */


