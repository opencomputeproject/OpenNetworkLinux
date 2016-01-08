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
#include <onlp/onlp.h>
#include <onlp/sfp.h>
#include <onlp/oids.h>
#include <onlp/thermal.h>
#include <onlp/fan.h>
#include <onlp/psu.h>
#include <onlp/led.h>

/* <auto.start.enum(ALL).source> */
aim_map_si_t onlp_fan_caps_map[] =
{
    { "B2F", ONLP_FAN_CAPS_B2F },
    { "F2B", ONLP_FAN_CAPS_F2B },
    { "SET_RPM", ONLP_FAN_CAPS_SET_RPM },
    { "SET_PERCENTAGE", ONLP_FAN_CAPS_SET_PERCENTAGE },
    { "GET_RPM", ONLP_FAN_CAPS_GET_RPM },
    { "GET_PERCENTAGE", ONLP_FAN_CAPS_GET_PERCENTAGE },
    { NULL, 0 }
};

aim_map_si_t onlp_fan_caps_desc_map[] =
{
    { "None", ONLP_FAN_CAPS_B2F },
    { "None", ONLP_FAN_CAPS_F2B },
    { "None", ONLP_FAN_CAPS_SET_RPM },
    { "None", ONLP_FAN_CAPS_SET_PERCENTAGE },
    { "None", ONLP_FAN_CAPS_GET_RPM },
    { "None", ONLP_FAN_CAPS_GET_PERCENTAGE },
    { NULL, 0 }
};

const char*
onlp_fan_caps_name(onlp_fan_caps_t e)
{
    const char* name;
    if(aim_map_si_i(&name, e, onlp_fan_caps_map, 0)) {
        return name;
    }
    else {
        return "-invalid value for enum type 'onlp_fan_caps'";
    }
}

int
onlp_fan_caps_value(const char* str, onlp_fan_caps_t* e, int substr)
{
    int i;
    AIM_REFERENCE(substr);
    if(aim_map_si_s(&i, str, onlp_fan_caps_map, 0)) {
        /* Enum Found */
        *e = i;
        return 0;
    }
    else {
        return -1;
    }
}

const char*
onlp_fan_caps_desc(onlp_fan_caps_t e)
{
    const char* name;
    if(aim_map_si_i(&name, e, onlp_fan_caps_desc_map, 0)) {
        return name;
    }
    else {
        return "-invalid value for enum type 'onlp_fan_caps'";
    }
}

int
onlp_fan_caps_valid(onlp_fan_caps_t e)
{
    return aim_map_si_i(NULL, e, onlp_fan_caps_map, 0) ? 1 : 0;
}


aim_map_si_t onlp_fan_dir_map[] =
{
    { "B2F", ONLP_FAN_DIR_B2F },
    { "F2B", ONLP_FAN_DIR_F2B },
    { NULL, 0 }
};

aim_map_si_t onlp_fan_dir_desc_map[] =
{
    { "None", ONLP_FAN_DIR_B2F },
    { "None", ONLP_FAN_DIR_F2B },
    { NULL, 0 }
};

const char*
onlp_fan_dir_name(onlp_fan_dir_t e)
{
    const char* name;
    if(aim_map_si_i(&name, e, onlp_fan_dir_map, 0)) {
        return name;
    }
    else {
        return "-invalid value for enum type 'onlp_fan_dir'";
    }
}

int
onlp_fan_dir_value(const char* str, onlp_fan_dir_t* e, int substr)
{
    int i;
    AIM_REFERENCE(substr);
    if(aim_map_si_s(&i, str, onlp_fan_dir_map, 0)) {
        /* Enum Found */
        *e = i;
        return 0;
    }
    else {
        return -1;
    }
}

const char*
onlp_fan_dir_desc(onlp_fan_dir_t e)
{
    const char* name;
    if(aim_map_si_i(&name, e, onlp_fan_dir_desc_map, 0)) {
        return name;
    }
    else {
        return "-invalid value for enum type 'onlp_fan_dir'";
    }
}


aim_map_si_t onlp_fan_mode_map[] =
{
    { "OFF", ONLP_FAN_MODE_OFF },
    { "SLOW", ONLP_FAN_MODE_SLOW },
    { "NORMAL", ONLP_FAN_MODE_NORMAL },
    { "FAST", ONLP_FAN_MODE_FAST },
    { "MAX", ONLP_FAN_MODE_MAX },
    { NULL, 0 }
};

aim_map_si_t onlp_fan_mode_desc_map[] =
{
    { "None", ONLP_FAN_MODE_OFF },
    { "None", ONLP_FAN_MODE_SLOW },
    { "None", ONLP_FAN_MODE_NORMAL },
    { "None", ONLP_FAN_MODE_FAST },
    { "None", ONLP_FAN_MODE_MAX },
    { NULL, 0 }
};

const char*
onlp_fan_mode_name(onlp_fan_mode_t e)
{
    const char* name;
    if(aim_map_si_i(&name, e, onlp_fan_mode_map, 0)) {
        return name;
    }
    else {
        return "-invalid value for enum type 'onlp_fan_mode'";
    }
}

int
onlp_fan_mode_value(const char* str, onlp_fan_mode_t* e, int substr)
{
    int i;
    AIM_REFERENCE(substr);
    if(aim_map_si_s(&i, str, onlp_fan_mode_map, 0)) {
        /* Enum Found */
        *e = i;
        return 0;
    }
    else {
        return -1;
    }
}

const char*
onlp_fan_mode_desc(onlp_fan_mode_t e)
{
    const char* name;
    if(aim_map_si_i(&name, e, onlp_fan_mode_desc_map, 0)) {
        return name;
    }
    else {
        return "-invalid value for enum type 'onlp_fan_mode'";
    }
}


aim_map_si_t onlp_fan_status_map[] =
{
    { "PRESENT", ONLP_FAN_STATUS_PRESENT },
    { "FAILED", ONLP_FAN_STATUS_FAILED },
    { "B2F", ONLP_FAN_STATUS_B2F },
    { "F2B", ONLP_FAN_STATUS_F2B },
    { NULL, 0 }
};

aim_map_si_t onlp_fan_status_desc_map[] =
{
    { "None", ONLP_FAN_STATUS_PRESENT },
    { "None", ONLP_FAN_STATUS_FAILED },
    { "None", ONLP_FAN_STATUS_B2F },
    { "None", ONLP_FAN_STATUS_F2B },
    { NULL, 0 }
};

const char*
onlp_fan_status_name(onlp_fan_status_t e)
{
    const char* name;
    if(aim_map_si_i(&name, e, onlp_fan_status_map, 0)) {
        return name;
    }
    else {
        return "-invalid value for enum type 'onlp_fan_status'";
    }
}

int
onlp_fan_status_value(const char* str, onlp_fan_status_t* e, int substr)
{
    int i;
    AIM_REFERENCE(substr);
    if(aim_map_si_s(&i, str, onlp_fan_status_map, 0)) {
        /* Enum Found */
        *e = i;
        return 0;
    }
    else {
        return -1;
    }
}

const char*
onlp_fan_status_desc(onlp_fan_status_t e)
{
    const char* name;
    if(aim_map_si_i(&name, e, onlp_fan_status_desc_map, 0)) {
        return name;
    }
    else {
        return "-invalid value for enum type 'onlp_fan_status'";
    }
}

int
onlp_fan_status_valid(onlp_fan_status_t e)
{
    return aim_map_si_i(NULL, e, onlp_fan_status_map, 0) ? 1 : 0;
}


aim_map_si_t onlp_led_caps_map[] =
{
    { "ON_OFF", ONLP_LED_CAPS_ON_OFF },
    { "RED", ONLP_LED_CAPS_RED },
    { "RED_BLINKING", ONLP_LED_CAPS_RED_BLINKING },
    { "ORANGE", ONLP_LED_CAPS_ORANGE },
    { "ORANGE_BLINKING", ONLP_LED_CAPS_ORANGE_BLINKING },
    { "YELLOW", ONLP_LED_CAPS_YELLOW },
    { "YELLOW_BLINKING", ONLP_LED_CAPS_YELLOW_BLINKING },
    { "GREEN", ONLP_LED_CAPS_GREEN },
    { "GREEN_BLINKING", ONLP_LED_CAPS_GREEN_BLINKING },
    { "BLUE", ONLP_LED_CAPS_BLUE },
    { "BLUE_BLINKING", ONLP_LED_CAPS_BLUE_BLINKING },
    { "PURPLE", ONLP_LED_CAPS_PURPLE },
    { "PURPLE_BLINKING", ONLP_LED_CAPS_PURPLE_BLINKING },
    { "AUTO", ONLP_LED_CAPS_AUTO },
    { NULL, 0 }
};

aim_map_si_t onlp_led_caps_desc_map[] =
{
    { "None", ONLP_LED_CAPS_ON_OFF },
    { "None", ONLP_LED_CAPS_RED },
    { "None", ONLP_LED_CAPS_RED_BLINKING },
    { "None", ONLP_LED_CAPS_ORANGE },
    { "None", ONLP_LED_CAPS_ORANGE_BLINKING },
    { "None", ONLP_LED_CAPS_YELLOW },
    { "None", ONLP_LED_CAPS_YELLOW_BLINKING },
    { "None", ONLP_LED_CAPS_GREEN },
    { "None", ONLP_LED_CAPS_GREEN_BLINKING },
    { "None", ONLP_LED_CAPS_BLUE },
    { "None", ONLP_LED_CAPS_BLUE_BLINKING },
    { "None", ONLP_LED_CAPS_PURPLE },
    { "None", ONLP_LED_CAPS_PURPLE_BLINKING },
    { "None", ONLP_LED_CAPS_AUTO },
    { NULL, 0 }
};

const char*
onlp_led_caps_name(onlp_led_caps_t e)
{
    const char* name;
    if(aim_map_si_i(&name, e, onlp_led_caps_map, 0)) {
        return name;
    }
    else {
        return "-invalid value for enum type 'onlp_led_caps'";
    }
}

int
onlp_led_caps_value(const char* str, onlp_led_caps_t* e, int substr)
{
    int i;
    AIM_REFERENCE(substr);
    if(aim_map_si_s(&i, str, onlp_led_caps_map, 0)) {
        /* Enum Found */
        *e = i;
        return 0;
    }
    else {
        return -1;
    }
}

const char*
onlp_led_caps_desc(onlp_led_caps_t e)
{
    const char* name;
    if(aim_map_si_i(&name, e, onlp_led_caps_desc_map, 0)) {
        return name;
    }
    else {
        return "-invalid value for enum type 'onlp_led_caps'";
    }
}

int
onlp_led_caps_valid(onlp_led_caps_t e)
{
    return aim_map_si_i(NULL, e, onlp_led_caps_map, 0) ? 1 : 0;
}


aim_map_si_t onlp_led_mode_map[] =
{
    { "OFF", ONLP_LED_MODE_OFF },
    { "ON", ONLP_LED_MODE_ON },
    { "BLINKING", ONLP_LED_MODE_BLINKING },
    { "RED", ONLP_LED_MODE_RED },
    { "RED_BLINKING", ONLP_LED_MODE_RED_BLINKING },
    { "ORANGE", ONLP_LED_MODE_ORANGE },
    { "ORANGE_BLINKING", ONLP_LED_MODE_ORANGE_BLINKING },
    { "YELLOW", ONLP_LED_MODE_YELLOW },
    { "YELLOW_BLINKING", ONLP_LED_MODE_YELLOW_BLINKING },
    { "GREEN", ONLP_LED_MODE_GREEN },
    { "GREEN_BLINKING", ONLP_LED_MODE_GREEN_BLINKING },
    { "BLUE", ONLP_LED_MODE_BLUE },
    { "BLUE_BLINKING", ONLP_LED_MODE_BLUE_BLINKING },
    { "PURPLE", ONLP_LED_MODE_PURPLE },
    { "PURPLE_BLINKING", ONLP_LED_MODE_PURPLE_BLINKING },
    { "AUTO", ONLP_LED_MODE_AUTO },
    { NULL, 0 }
};

aim_map_si_t onlp_led_mode_desc_map[] =
{
    { "None", ONLP_LED_MODE_OFF },
    { "None", ONLP_LED_MODE_ON },
    { "None", ONLP_LED_MODE_BLINKING },
    { "None", ONLP_LED_MODE_RED },
    { "None", ONLP_LED_MODE_RED_BLINKING },
    { "None", ONLP_LED_MODE_ORANGE },
    { "None", ONLP_LED_MODE_ORANGE_BLINKING },
    { "None", ONLP_LED_MODE_YELLOW },
    { "None", ONLP_LED_MODE_YELLOW_BLINKING },
    { "None", ONLP_LED_MODE_GREEN },
    { "None", ONLP_LED_MODE_GREEN_BLINKING },
    { "None", ONLP_LED_MODE_BLUE },
    { "None", ONLP_LED_MODE_BLUE_BLINKING },
    { "None", ONLP_LED_MODE_PURPLE },
    { "None", ONLP_LED_MODE_PURPLE_BLINKING },
    { "None", ONLP_LED_MODE_AUTO },
    { NULL, 0 }
};

const char*
onlp_led_mode_name(onlp_led_mode_t e)
{
    const char* name;
    if(aim_map_si_i(&name, e, onlp_led_mode_map, 0)) {
        return name;
    }
    else {
        return "-invalid value for enum type 'onlp_led_mode'";
    }
}

int
onlp_led_mode_value(const char* str, onlp_led_mode_t* e, int substr)
{
    int i;
    AIM_REFERENCE(substr);
    if(aim_map_si_s(&i, str, onlp_led_mode_map, 0)) {
        /* Enum Found */
        *e = i;
        return 0;
    }
    else {
        return -1;
    }
}

const char*
onlp_led_mode_desc(onlp_led_mode_t e)
{
    const char* name;
    if(aim_map_si_i(&name, e, onlp_led_mode_desc_map, 0)) {
        return name;
    }
    else {
        return "-invalid value for enum type 'onlp_led_mode'";
    }
}

int
onlp_led_mode_valid(onlp_led_mode_t e)
{
    return aim_map_si_i(NULL, e, onlp_led_mode_map, 0) ? 1 : 0;
}


aim_map_si_t onlp_led_status_map[] =
{
    { "PRESENT", ONLP_LED_STATUS_PRESENT },
    { "FAILED", ONLP_LED_STATUS_FAILED },
    { "ON", ONLP_LED_STATUS_ON },
    { NULL, 0 }
};

aim_map_si_t onlp_led_status_desc_map[] =
{
    { "None", ONLP_LED_STATUS_PRESENT },
    { "None", ONLP_LED_STATUS_FAILED },
    { "None", ONLP_LED_STATUS_ON },
    { NULL, 0 }
};

const char*
onlp_led_status_name(onlp_led_status_t e)
{
    const char* name;
    if(aim_map_si_i(&name, e, onlp_led_status_map, 0)) {
        return name;
    }
    else {
        return "-invalid value for enum type 'onlp_led_status'";
    }
}

int
onlp_led_status_value(const char* str, onlp_led_status_t* e, int substr)
{
    int i;
    AIM_REFERENCE(substr);
    if(aim_map_si_s(&i, str, onlp_led_status_map, 0)) {
        /* Enum Found */
        *e = i;
        return 0;
    }
    else {
        return -1;
    }
}

const char*
onlp_led_status_desc(onlp_led_status_t e)
{
    const char* name;
    if(aim_map_si_i(&name, e, onlp_led_status_desc_map, 0)) {
        return name;
    }
    else {
        return "-invalid value for enum type 'onlp_led_status'";
    }
}

int
onlp_led_status_valid(onlp_led_status_t e)
{
    return aim_map_si_i(NULL, e, onlp_led_status_map, 0) ? 1 : 0;
}


aim_map_si_t onlp_oid_type_map[] =
{
    { "SYS", ONLP_OID_TYPE_SYS },
    { "THERMAL", ONLP_OID_TYPE_THERMAL },
    { "FAN", ONLP_OID_TYPE_FAN },
    { "PSU", ONLP_OID_TYPE_PSU },
    { "LED", ONLP_OID_TYPE_LED },
    { "MODULE", ONLP_OID_TYPE_MODULE },
    { "RTC", ONLP_OID_TYPE_RTC },
    { NULL, 0 }
};

aim_map_si_t onlp_oid_type_desc_map[] =
{
    { "None", ONLP_OID_TYPE_SYS },
    { "None", ONLP_OID_TYPE_THERMAL },
    { "None", ONLP_OID_TYPE_FAN },
    { "None", ONLP_OID_TYPE_PSU },
    { "None", ONLP_OID_TYPE_LED },
    { "None", ONLP_OID_TYPE_MODULE },
    { "None", ONLP_OID_TYPE_RTC },
    { NULL, 0 }
};

const char*
onlp_oid_type_name(onlp_oid_type_t e)
{
    const char* name;
    if(aim_map_si_i(&name, e, onlp_oid_type_map, 0)) {
        return name;
    }
    else {
        return "-invalid value for enum type 'onlp_oid_type'";
    }
}

int
onlp_oid_type_value(const char* str, onlp_oid_type_t* e, int substr)
{
    int i;
    AIM_REFERENCE(substr);
    if(aim_map_si_s(&i, str, onlp_oid_type_map, 0)) {
        /* Enum Found */
        *e = i;
        return 0;
    }
    else {
        return -1;
    }
}

const char*
onlp_oid_type_desc(onlp_oid_type_t e)
{
    const char* name;
    if(aim_map_si_i(&name, e, onlp_oid_type_desc_map, 0)) {
        return name;
    }
    else {
        return "-invalid value for enum type 'onlp_oid_type'";
    }
}

int
onlp_oid_type_valid(onlp_oid_type_t e)
{
    return aim_map_si_i(NULL, e, onlp_oid_type_map, 0) ? 1 : 0;
}


aim_map_si_t onlp_psu_caps_map[] =
{
    { "AC", ONLP_PSU_CAPS_AC },
    { "DC12", ONLP_PSU_CAPS_DC12 },
    { "DC48", ONLP_PSU_CAPS_DC48 },
    { "VIN", ONLP_PSU_CAPS_VIN },
    { "VOUT", ONLP_PSU_CAPS_VOUT },
    { "IIN", ONLP_PSU_CAPS_IIN },
    { "IOUT", ONLP_PSU_CAPS_IOUT },
    { "PIN", ONLP_PSU_CAPS_PIN },
    { "POUT", ONLP_PSU_CAPS_POUT },
    { NULL, 0 }
};

aim_map_si_t onlp_psu_caps_desc_map[] =
{
    { "None", ONLP_PSU_CAPS_AC },
    { "None", ONLP_PSU_CAPS_DC12 },
    { "None", ONLP_PSU_CAPS_DC48 },
    { "None", ONLP_PSU_CAPS_VIN },
    { "None", ONLP_PSU_CAPS_VOUT },
    { "None", ONLP_PSU_CAPS_IIN },
    { "None", ONLP_PSU_CAPS_IOUT },
    { "None", ONLP_PSU_CAPS_PIN },
    { "None", ONLP_PSU_CAPS_POUT },
    { NULL, 0 }
};

const char*
onlp_psu_caps_name(onlp_psu_caps_t e)
{
    const char* name;
    if(aim_map_si_i(&name, e, onlp_psu_caps_map, 0)) {
        return name;
    }
    else {
        return "-invalid value for enum type 'onlp_psu_caps'";
    }
}

int
onlp_psu_caps_value(const char* str, onlp_psu_caps_t* e, int substr)
{
    int i;
    AIM_REFERENCE(substr);
    if(aim_map_si_s(&i, str, onlp_psu_caps_map, 0)) {
        /* Enum Found */
        *e = i;
        return 0;
    }
    else {
        return -1;
    }
}

const char*
onlp_psu_caps_desc(onlp_psu_caps_t e)
{
    const char* name;
    if(aim_map_si_i(&name, e, onlp_psu_caps_desc_map, 0)) {
        return name;
    }
    else {
        return "-invalid value for enum type 'onlp_psu_caps'";
    }
}

int
onlp_psu_caps_valid(onlp_psu_caps_t e)
{
    return aim_map_si_i(NULL, e, onlp_psu_caps_map, 0) ? 1 : 0;
}


aim_map_si_t onlp_psu_status_map[] =
{
    { "PRESENT", ONLP_PSU_STATUS_PRESENT },
    { "FAILED", ONLP_PSU_STATUS_FAILED },
    { "UNPLUGGED", ONLP_PSU_STATUS_UNPLUGGED },
    { NULL, 0 }
};

aim_map_si_t onlp_psu_status_desc_map[] =
{
    { "None", ONLP_PSU_STATUS_PRESENT },
    { "None", ONLP_PSU_STATUS_FAILED },
    { "None", ONLP_PSU_STATUS_UNPLUGGED },
    { NULL, 0 }
};

const char*
onlp_psu_status_name(onlp_psu_status_t e)
{
    const char* name;
    if(aim_map_si_i(&name, e, onlp_psu_status_map, 0)) {
        return name;
    }
    else {
        return "-invalid value for enum type 'onlp_psu_status'";
    }
}

int
onlp_psu_status_value(const char* str, onlp_psu_status_t* e, int substr)
{
    int i;
    AIM_REFERENCE(substr);
    if(aim_map_si_s(&i, str, onlp_psu_status_map, 0)) {
        /* Enum Found */
        *e = i;
        return 0;
    }
    else {
        return -1;
    }
}

const char*
onlp_psu_status_desc(onlp_psu_status_t e)
{
    const char* name;
    if(aim_map_si_i(&name, e, onlp_psu_status_desc_map, 0)) {
        return name;
    }
    else {
        return "-invalid value for enum type 'onlp_psu_status'";
    }
}

int
onlp_psu_status_valid(onlp_psu_status_t e)
{
    return aim_map_si_i(NULL, e, onlp_psu_status_map, 0) ? 1 : 0;
}


aim_map_si_t onlp_sfp_control_map[] =
{
    { "RESET", ONLP_SFP_CONTROL_RESET },
    { "RESET_STATE", ONLP_SFP_CONTROL_RESET_STATE },
    { "RX_LOS", ONLP_SFP_CONTROL_RX_LOS },
    { "TX_FAULT", ONLP_SFP_CONTROL_TX_FAULT },
    { "TX_DISABLE", ONLP_SFP_CONTROL_TX_DISABLE },
    { "LP_MODE", ONLP_SFP_CONTROL_LP_MODE },
    { "POWER_OVERRIDE", ONLP_SFP_CONTROL_POWER_OVERRIDE },
    { NULL, 0 }
};

aim_map_si_t onlp_sfp_control_desc_map[] =
{
    { "None", ONLP_SFP_CONTROL_RESET },
    { "None", ONLP_SFP_CONTROL_RESET_STATE },
    { "None", ONLP_SFP_CONTROL_RX_LOS },
    { "None", ONLP_SFP_CONTROL_TX_FAULT },
    { "None", ONLP_SFP_CONTROL_TX_DISABLE },
    { "None", ONLP_SFP_CONTROL_LP_MODE },
    { "None", ONLP_SFP_CONTROL_POWER_OVERRIDE },
    { NULL, 0 }
};

const char*
onlp_sfp_control_name(onlp_sfp_control_t e)
{
    const char* name;
    if(aim_map_si_i(&name, e, onlp_sfp_control_map, 0)) {
        return name;
    }
    else {
        return "-invalid value for enum type 'onlp_sfp_control'";
    }
}

int
onlp_sfp_control_value(const char* str, onlp_sfp_control_t* e, int substr)
{
    int i;
    AIM_REFERENCE(substr);
    if(aim_map_si_s(&i, str, onlp_sfp_control_map, 0)) {
        /* Enum Found */
        *e = i;
        return 0;
    }
    else {
        return -1;
    }
}

const char*
onlp_sfp_control_desc(onlp_sfp_control_t e)
{
    const char* name;
    if(aim_map_si_i(&name, e, onlp_sfp_control_desc_map, 0)) {
        return name;
    }
    else {
        return "-invalid value for enum type 'onlp_sfp_control'";
    }
}


aim_map_si_t onlp_sfp_control_flag_map[] =
{
    { "RESET", ONLP_SFP_CONTROL_FLAG_RESET },
    { "RESET_STATE", ONLP_SFP_CONTROL_FLAG_RESET_STATE },
    { "RX_LOS", ONLP_SFP_CONTROL_FLAG_RX_LOS },
    { "TX_FAULT", ONLP_SFP_CONTROL_FLAG_TX_FAULT },
    { "TX_DISABLE", ONLP_SFP_CONTROL_FLAG_TX_DISABLE },
    { "LP_MODE", ONLP_SFP_CONTROL_FLAG_LP_MODE },
    { "POWER_OVERRIDE", ONLP_SFP_CONTROL_FLAG_POWER_OVERRIDE },
    { NULL, 0 }
};

aim_map_si_t onlp_sfp_control_flag_desc_map[] =
{
    { "None", ONLP_SFP_CONTROL_FLAG_RESET },
    { "None", ONLP_SFP_CONTROL_FLAG_RESET_STATE },
    { "None", ONLP_SFP_CONTROL_FLAG_RX_LOS },
    { "None", ONLP_SFP_CONTROL_FLAG_TX_FAULT },
    { "None", ONLP_SFP_CONTROL_FLAG_TX_DISABLE },
    { "None", ONLP_SFP_CONTROL_FLAG_LP_MODE },
    { "None", ONLP_SFP_CONTROL_FLAG_POWER_OVERRIDE },
    { NULL, 0 }
};

const char*
onlp_sfp_control_flag_name(onlp_sfp_control_flag_t e)
{
    const char* name;
    if(aim_map_si_i(&name, e, onlp_sfp_control_flag_map, 0)) {
        return name;
    }
    else {
        return "-invalid value for enum type 'onlp_sfp_control_flag'";
    }
}

int
onlp_sfp_control_flag_value(const char* str, onlp_sfp_control_flag_t* e, int substr)
{
    int i;
    AIM_REFERENCE(substr);
    if(aim_map_si_s(&i, str, onlp_sfp_control_flag_map, 0)) {
        /* Enum Found */
        *e = i;
        return 0;
    }
    else {
        return -1;
    }
}

const char*
onlp_sfp_control_flag_desc(onlp_sfp_control_flag_t e)
{
    const char* name;
    if(aim_map_si_i(&name, e, onlp_sfp_control_flag_desc_map, 0)) {
        return name;
    }
    else {
        return "-invalid value for enum type 'onlp_sfp_control_flag'";
    }
}

int
onlp_sfp_control_flag_valid(onlp_sfp_control_flag_t e)
{
    return aim_map_si_i(NULL, e, onlp_sfp_control_flag_map, 0) ? 1 : 0;
}


aim_map_si_t onlp_status_map[] =
{
    { "OK", ONLP_STATUS_OK },
    { "E_UNSUPPORTED", ONLP_STATUS_E_UNSUPPORTED },
    { "E_MISSING", ONLP_STATUS_E_MISSING },
    { "E_INVALID", ONLP_STATUS_E_INVALID },
    { "E_INTERNAL", ONLP_STATUS_E_INTERNAL },
    { "E_PARAM", ONLP_STATUS_E_PARAM },
    { NULL, 0 }
};

aim_map_si_t onlp_status_desc_map[] =
{
    { "None", ONLP_STATUS_OK },
    { "None", ONLP_STATUS_E_UNSUPPORTED },
    { "None", ONLP_STATUS_E_MISSING },
    { "None", ONLP_STATUS_E_INVALID },
    { "None", ONLP_STATUS_E_INTERNAL },
    { "None", ONLP_STATUS_E_PARAM },
    { NULL, 0 }
};

const char*
onlp_status_name(onlp_status_t e)
{
    const char* name;
    if(aim_map_si_i(&name, e, onlp_status_map, 0)) {
        return name;
    }
    else {
        return "-invalid value for enum type 'onlp_status'";
    }
}

int
onlp_status_value(const char* str, onlp_status_t* e, int substr)
{
    int i;
    AIM_REFERENCE(substr);
    if(aim_map_si_s(&i, str, onlp_status_map, 0)) {
        /* Enum Found */
        *e = i;
        return 0;
    }
    else {
        return -1;
    }
}

const char*
onlp_status_desc(onlp_status_t e)
{
    const char* name;
    if(aim_map_si_i(&name, e, onlp_status_desc_map, 0)) {
        return name;
    }
    else {
        return "-invalid value for enum type 'onlp_status'";
    }
}

int
onlp_status_valid(onlp_status_t e)
{
    return aim_map_si_i(NULL, e, onlp_status_map, 0) ? 1 : 0;
}


aim_map_si_t onlp_thermal_caps_map[] =
{
    { "GET_TEMPERATURE", ONLP_THERMAL_CAPS_GET_TEMPERATURE },
    { "GET_WARNING_THRESHOLD", ONLP_THERMAL_CAPS_GET_WARNING_THRESHOLD },
    { "GET_ERROR_THRESHOLD", ONLP_THERMAL_CAPS_GET_ERROR_THRESHOLD },
    { "GET_SHUTDOWN_THRESHOLD", ONLP_THERMAL_CAPS_GET_SHUTDOWN_THRESHOLD },
    { NULL, 0 }
};

aim_map_si_t onlp_thermal_caps_desc_map[] =
{
    { "None", ONLP_THERMAL_CAPS_GET_TEMPERATURE },
    { "None", ONLP_THERMAL_CAPS_GET_WARNING_THRESHOLD },
    { "None", ONLP_THERMAL_CAPS_GET_ERROR_THRESHOLD },
    { "None", ONLP_THERMAL_CAPS_GET_SHUTDOWN_THRESHOLD },
    { NULL, 0 }
};

const char*
onlp_thermal_caps_name(onlp_thermal_caps_t e)
{
    const char* name;
    if(aim_map_si_i(&name, e, onlp_thermal_caps_map, 0)) {
        return name;
    }
    else {
        return "-invalid value for enum type 'onlp_thermal_caps'";
    }
}

int
onlp_thermal_caps_value(const char* str, onlp_thermal_caps_t* e, int substr)
{
    int i;
    AIM_REFERENCE(substr);
    if(aim_map_si_s(&i, str, onlp_thermal_caps_map, 0)) {
        /* Enum Found */
        *e = i;
        return 0;
    }
    else {
        return -1;
    }
}

const char*
onlp_thermal_caps_desc(onlp_thermal_caps_t e)
{
    const char* name;
    if(aim_map_si_i(&name, e, onlp_thermal_caps_desc_map, 0)) {
        return name;
    }
    else {
        return "-invalid value for enum type 'onlp_thermal_caps'";
    }
}

int
onlp_thermal_caps_valid(onlp_thermal_caps_t e)
{
    return aim_map_si_i(NULL, e, onlp_thermal_caps_map, 0) ? 1 : 0;
}


aim_map_si_t onlp_thermal_status_map[] =
{
    { "PRESENT", ONLP_THERMAL_STATUS_PRESENT },
    { "FAILED", ONLP_THERMAL_STATUS_FAILED },
    { NULL, 0 }
};

aim_map_si_t onlp_thermal_status_desc_map[] =
{
    { "None", ONLP_THERMAL_STATUS_PRESENT },
    { "None", ONLP_THERMAL_STATUS_FAILED },
    { NULL, 0 }
};

const char*
onlp_thermal_status_name(onlp_thermal_status_t e)
{
    const char* name;
    if(aim_map_si_i(&name, e, onlp_thermal_status_map, 0)) {
        return name;
    }
    else {
        return "-invalid value for enum type 'onlp_thermal_status'";
    }
}

int
onlp_thermal_status_value(const char* str, onlp_thermal_status_t* e, int substr)
{
    int i;
    AIM_REFERENCE(substr);
    if(aim_map_si_s(&i, str, onlp_thermal_status_map, 0)) {
        /* Enum Found */
        *e = i;
        return 0;
    }
    else {
        return -1;
    }
}

const char*
onlp_thermal_status_desc(onlp_thermal_status_t e)
{
    const char* name;
    if(aim_map_si_i(&name, e, onlp_thermal_status_desc_map, 0)) {
        return name;
    }
    else {
        return "-invalid value for enum type 'onlp_thermal_status'";
    }
}

int
onlp_thermal_status_valid(onlp_thermal_status_t e)
{
    return aim_map_si_i(NULL, e, onlp_thermal_status_map, 0) ? 1 : 0;
}


aim_map_si_t onlp_thermal_threshold_map[] =
{
    { "WARNING_DEFAULT", ONLP_THERMAL_THRESHOLD_WARNING_DEFAULT },
    { "ERROR_DEFAULT", ONLP_THERMAL_THRESHOLD_ERROR_DEFAULT },
    { "SHUTDOWN_DEFAULT", ONLP_THERMAL_THRESHOLD_SHUTDOWN_DEFAULT },
    { NULL, 0 }
};

aim_map_si_t onlp_thermal_threshold_desc_map[] =
{
    { "None", ONLP_THERMAL_THRESHOLD_WARNING_DEFAULT },
    { "None", ONLP_THERMAL_THRESHOLD_ERROR_DEFAULT },
    { "None", ONLP_THERMAL_THRESHOLD_SHUTDOWN_DEFAULT },
    { NULL, 0 }
};

const char*
onlp_thermal_threshold_name(onlp_thermal_threshold_t e)
{
    const char* name;
    if(aim_map_si_i(&name, e, onlp_thermal_threshold_map, 0)) {
        return name;
    }
    else {
        return "-invalid value for enum type 'onlp_thermal_threshold'";
    }
}

int
onlp_thermal_threshold_value(const char* str, onlp_thermal_threshold_t* e, int substr)
{
    int i;
    AIM_REFERENCE(substr);
    if(aim_map_si_s(&i, str, onlp_thermal_threshold_map, 0)) {
        /* Enum Found */
        *e = i;
        return 0;
    }
    else {
        return -1;
    }
}

const char*
onlp_thermal_threshold_desc(onlp_thermal_threshold_t e)
{
    const char* name;
    if(aim_map_si_i(&name, e, onlp_thermal_threshold_desc_map, 0)) {
        return name;
    }
    else {
        return "-invalid value for enum type 'onlp_thermal_threshold'";
    }
}

int
onlp_thermal_threshold_valid(onlp_thermal_threshold_t e)
{
    return aim_map_si_i(NULL, e, onlp_thermal_threshold_map, 0) ? 1 : 0;
}

/* <auto.end.enum(ALL).source> */

