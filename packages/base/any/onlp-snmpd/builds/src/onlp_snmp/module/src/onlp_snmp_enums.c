/**************************************************************************//**
 *
 *
 *
 *****************************************************************************/
#include <onlp_snmp/onlp_snmp_config.h>
#include <onlp_snmp/onlp_snmp_sensor_oids.h>

/* <auto.start.enum(ALL).source> */
aim_map_si_t onlp_snmp_fan_flow_type_map[] =
{
    { "unknown", ONLP_SNMP_FAN_FLOW_TYPE_UNKNOWN },
    { "b2f", ONLP_SNMP_FAN_FLOW_TYPE_B2F },
    { "f2b", ONLP_SNMP_FAN_FLOW_TYPE_F2B },
    { NULL, 0 }
};

aim_map_si_t onlp_snmp_fan_flow_type_desc_map[] =
{
    { "None", ONLP_SNMP_FAN_FLOW_TYPE_UNKNOWN },
    { "None", ONLP_SNMP_FAN_FLOW_TYPE_B2F },
    { "None", ONLP_SNMP_FAN_FLOW_TYPE_F2B },
    { NULL, 0 }
};

const char*
onlp_snmp_fan_flow_type_name(onlp_snmp_fan_flow_type_t e)
{
    const char* name;
    if(aim_map_si_i(&name, e, onlp_snmp_fan_flow_type_map, 0)) {
        return name;
    }
    else {
        return "-invalid value for enum type 'onlp_snmp_fan_flow_type'";
    }
}

int
onlp_snmp_fan_flow_type_value(const char* str, onlp_snmp_fan_flow_type_t* e, int substr)
{
    int i;
    AIM_REFERENCE(substr);
    if(aim_map_si_s(&i, str, onlp_snmp_fan_flow_type_map, 0)) {
        /* Enum Found */
        *e = i;
        return 0;
    }
    else {
        return -1;
    }
}

const char*
onlp_snmp_fan_flow_type_desc(onlp_snmp_fan_flow_type_t e)
{
    const char* name;
    if(aim_map_si_i(&name, e, onlp_snmp_fan_flow_type_desc_map, 0)) {
        return name;
    }
    else {
        return "-invalid value for enum type 'onlp_snmp_fan_flow_type'";
    }
}

int
onlp_snmp_fan_flow_type_valid(onlp_snmp_fan_flow_type_t e)
{
    return aim_map_si_i(NULL, e, onlp_snmp_fan_flow_type_map, 0) ? 1 : 0;
}


aim_map_si_t onlp_snmp_sensor_status_map[] =
{
    { "missing", ONLP_SNMP_SENSOR_STATUS_MISSING },
    { "good", ONLP_SNMP_SENSOR_STATUS_GOOD },
    { "failed", ONLP_SNMP_SENSOR_STATUS_FAILED },
    { "unplugged", ONLP_SNMP_SENSOR_STATUS_UNPLUGGED },
    { NULL, 0 }
};

aim_map_si_t onlp_snmp_sensor_status_desc_map[] =
{
    { "None", ONLP_SNMP_SENSOR_STATUS_MISSING },
    { "None", ONLP_SNMP_SENSOR_STATUS_GOOD },
    { "None", ONLP_SNMP_SENSOR_STATUS_FAILED },
    { "None", ONLP_SNMP_SENSOR_STATUS_UNPLUGGED },
    { NULL, 0 }
};

const char*
onlp_snmp_sensor_status_name(onlp_snmp_sensor_status_t e)
{
    const char* name;
    if(aim_map_si_i(&name, e, onlp_snmp_sensor_status_map, 0)) {
        return name;
    }
    else {
        return "-invalid value for enum type 'onlp_snmp_sensor_status'";
    }
}

int
onlp_snmp_sensor_status_value(const char* str, onlp_snmp_sensor_status_t* e, int substr)
{
    int i;
    AIM_REFERENCE(substr);
    if(aim_map_si_s(&i, str, onlp_snmp_sensor_status_map, 0)) {
        /* Enum Found */
        *e = i;
        return 0;
    }
    else {
        return -1;
    }
}

const char*
onlp_snmp_sensor_status_desc(onlp_snmp_sensor_status_t e)
{
    const char* name;
    if(aim_map_si_i(&name, e, onlp_snmp_sensor_status_desc_map, 0)) {
        return name;
    }
    else {
        return "-invalid value for enum type 'onlp_snmp_sensor_status'";
    }
}

int
onlp_snmp_sensor_status_valid(onlp_snmp_sensor_status_t e)
{
    return aim_map_si_i(NULL, e, onlp_snmp_sensor_status_map, 0) ? 1 : 0;
}


aim_map_si_t onlp_snmp_psu_type_map[] =
{
    { "unknown", ONLP_SNMP_PSU_TYPE_UNKNOWN },
    { "ac", ONLP_SNMP_PSU_TYPE_AC },
    { "dc12", ONLP_SNMP_PSU_TYPE_DC12 },
    { "dc48", ONLP_SNMP_PSU_TYPE_DC48 },
    { NULL, 0 }
};

aim_map_si_t onlp_snmp_psu_type_desc_map[] =
{
    { "None", ONLP_SNMP_PSU_TYPE_UNKNOWN },
    { "None", ONLP_SNMP_PSU_TYPE_AC },
    { "None", ONLP_SNMP_PSU_TYPE_DC12 },
    { "None", ONLP_SNMP_PSU_TYPE_DC48 },
    { NULL, 0 }
};

const char*
onlp_snmp_psu_type_name(onlp_snmp_psu_type_t e)
{
    const char* name;
    if(aim_map_si_i(&name, e, onlp_snmp_psu_type_map, 0)) {
        return name;
    }
    else {
        return "-invalid value for enum type 'onlp_snmp_psu_type'";
    }
}

int
onlp_snmp_psu_type_value(const char* str, onlp_snmp_psu_type_t* e, int substr)
{
    int i;
    AIM_REFERENCE(substr);
    if(aim_map_si_s(&i, str, onlp_snmp_psu_type_map, 0)) {
        /* Enum Found */
        *e = i;
        return 0;
    }
    else {
        return -1;
    }
}

const char*
onlp_snmp_psu_type_desc(onlp_snmp_psu_type_t e)
{
    const char* name;
    if(aim_map_si_i(&name, e, onlp_snmp_psu_type_desc_map, 0)) {
        return name;
    }
    else {
        return "-invalid value for enum type 'onlp_snmp_psu_type'";
    }
}

int
onlp_snmp_psu_type_valid(onlp_snmp_psu_type_t e)
{
    return aim_map_si_i(NULL, e, onlp_snmp_psu_type_map, 0) ? 1 : 0;
}


aim_map_si_t onlp_snmp_sensor_type_map[] =
{
    { "temp", ONLP_SNMP_SENSOR_TYPE_TEMP },
    { "fan", ONLP_SNMP_SENSOR_TYPE_FAN },
    { "psu", ONLP_SNMP_SENSOR_TYPE_PSU },
    { "led", ONLP_SNMP_SENSOR_TYPE_LED },
    { "misc", ONLP_SNMP_SENSOR_TYPE_MISC },
    { "max", ONLP_SNMP_SENSOR_TYPE_MAX },
    { NULL, 0 }
};

aim_map_si_t onlp_snmp_sensor_type_desc_map[] =
{
    { "None", ONLP_SNMP_SENSOR_TYPE_TEMP },
    { "None", ONLP_SNMP_SENSOR_TYPE_FAN },
    { "None", ONLP_SNMP_SENSOR_TYPE_PSU },
    { "None", ONLP_SNMP_SENSOR_TYPE_LED },
    { "None", ONLP_SNMP_SENSOR_TYPE_MISC },
    { "None", ONLP_SNMP_SENSOR_TYPE_MAX },
    { NULL, 0 }
};

const char*
onlp_snmp_sensor_type_name(onlp_snmp_sensor_type_t e)
{
    const char* name;
    if(aim_map_si_i(&name, e, onlp_snmp_sensor_type_map, 0)) {
        return name;
    }
    else {
        return "-invalid value for enum type 'onlp_snmp_sensor_type'";
    }
}

int
onlp_snmp_sensor_type_value(const char* str, onlp_snmp_sensor_type_t* e, int substr)
{
    int i;
    AIM_REFERENCE(substr);
    if(aim_map_si_s(&i, str, onlp_snmp_sensor_type_map, 0)) {
        /* Enum Found */
        *e = i;
        return 0;
    }
    else {
        return -1;
    }
}

const char*
onlp_snmp_sensor_type_desc(onlp_snmp_sensor_type_t e)
{
    const char* name;
    if(aim_map_si_i(&name, e, onlp_snmp_sensor_type_desc_map, 0)) {
        return name;
    }
    else {
        return "-invalid value for enum type 'onlp_snmp_sensor_type'";
    }
}

int
onlp_snmp_sensor_type_valid(onlp_snmp_sensor_type_t e)
{
    return aim_map_si_i(NULL, e, onlp_snmp_sensor_type_map, 0) ? 1 : 0;
}

/* <auto.end.enum(ALL).source> */

