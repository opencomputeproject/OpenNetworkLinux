/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright 2014 Accton Technology Corporation.
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
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>
#include <arpa/inet.h>

#include <onlp/platformi/sysi.h>
#include <onlp/platformi/ledi.h>
#include <onlp/platformi/thermali.h>
#include <onlp/platformi/fani.h>
#include <onlp/platformi/psui.h>
#include "platform_lib.h"
#include "x86_64_accton_csp9250_int.h"
#include "x86_64_accton_csp9250_log.h"


#define PREFIX_PATH_ON_CPLD_DEV          "/sys/bus/i2c/devices/"
#define NUM_OF_CPLD                      3
#define FAN_DUTY_CYCLE_MAX         (100)
#define FAN_DUTY_CYCLE_DEFAULT     (32)
#define FAN_DUTY_PLUS_FOR_DIR      (13)
/* Note, all chassis fans share 1 single duty setting. 
 * Here use fan 1 to represent global fan duty value.*/
#define FAN_ID_FOR_SET_FAN_DUTY    (1)
#define CELSIUS_RECORD_NUMBER      (2)  /*Must >= 2*/


#define SYS_MAX_LENGTH             1024
#define SYS_MAC_ADDR_LEN            6
#define SYS_BUFFER_MAC_ADDR_LEN     17
#define SYS_IPMI_FRU_CMD_0  "ipmitool fru print 0"
#define SYS_IPMI_FRU_CMD_1  "ipmitool fru print 1"
#define SYS_IPMI_TMP_FILE_0_RM "rm -f /tmp/sys_bmc_info_0 > /dev/null 2>&1"
#define SYS_IPMI_TMP_FILE_1_RM "rm -f /tmp/sys_bmc_info_0 > /dev/null 2>&1"
#define SYS_IPMI_TMP_FILE_0            "/tmp/sys_bmc_info_0"
#define SYS_IPMI_TMP_FILE_1            "/tmp/sys_bmc_info_1"
#define SYS_NETWORK_MAC_GET            "cat /sys/class/net/ma1/address"
#define SYS_IPMI_TMP_FILE_MAC          "/tmp/sys_network_mac"
#define SYS_IPMI_TMP_FILE_MAC_RM "rm -f /tmp/sys_network_mac > /dev/null 2>&1"

typedef struct fan_ctrl_policy {
   int duty_cycle;        /* In percetage */  
   int step_up_thermal;   /* In mini-Celsius */
   int step_dn_thermal;   /* In mini-Celsius */
} fan_ctrl_policy_t;

static char arr_cplddev_name[NUM_OF_CPLD][10] =
{
 "1-0060",
 "2-0062",
 "3-0064"
};

/*
 * Tlvinf header: Layout of the header for the TlvInfo format
 *
 * See the end of this file for details of this eeprom format
 */
struct __attribute__ ((__packed__)) tlvinfo_header_s {
    char    signature[8];   /* 0x00 - 0x07 EEPROM Tag "TlvInfo" */
    u_int8_t      version;  /* 0x08        Structure version */
    u_int16_t     totallen; /* 0x09 - 0x0A Length of all data which follows */
};
typedef struct tlvinfo_header_s tlvinfo_header_t;

// Header Field Constants
#define SYS_EEPROM_SIZE         256
#define TLV_INFO_ID_STRING      "TlvInfo"
#define TLV_INFO_VERSION        0x01
#define TLV_TOTAL_LEN_MAX       (SYS_EEPROM_SIZE - sizeof(tlvinfo_header_t))

struct __attribute__ ((__packed__)) tlvinfo_tlv_s {
    u_int8_t  type;
    u_int8_t  length;
    u_int8_t  value[0];
};
typedef struct tlvinfo_tlv_s tlvinfo_tlv_t;

/* Maximum length of a TLV value in bytes */
#define TLV_VALUE_MAX_LEN        255

/**
 *  The TLV Types.
 *
 *  Keep these in sync with tlv_code_list in cmd_sys_eeprom.c
 */
#define TLV_CODE_PRODUCT_NAME   0x21
#define TLV_CODE_PART_NUMBER    0x22
#define TLV_CODE_SERIAL_NUMBER  0x23
#define TLV_CODE_MAC_BASE       0x24
#define TLV_CODE_MANUF_DATE     0x25
#define TLV_CODE_DEVICE_VERSION 0x26
#define TLV_CODE_LABEL_REVISION 0x27
#define TLV_CODE_PLATFORM_NAME  0x28
#define TLV_CODE_ONIE_VERSION   0x29
#define TLV_CODE_MAC_SIZE       0x2A
#define TLV_CODE_MANUF_NAME     0x2B
#define TLV_CODE_MANUF_COUNTRY  0x2C
#define TLV_CODE_VENDOR_NAME    0x2D
#define TLV_CODE_DIAG_VERSION   0x2E
#define TLV_CODE_SERVICE_TAG    0x2F
#define TLV_CODE_VENDOR_EXT     0xFD
#define TLV_CODE_CRC_32         0xFE



typedef struct onlp_sys_eeprom_dev_s
{   
   int   tcode; 
   int   size;
   char  *tag;   
   char  *content;    	
}onlp_sys_eeprom_dev_t;


onlp_sys_eeprom_dev_t sys_eeprom_table[]=
{
    {TLV_CODE_PRODUCT_NAME,  0,  "Board Part Number     :", NULL},
    {TLV_CODE_PART_NUMBER,   0,  "Chassis Part Number   :", NULL},
    {TLV_CODE_SERIAL_NUMBER, 0,  "Chassis Serial        :", NULL},
    {TLV_CODE_MAC_BASE,      SYS_MAC_ADDR_LEN, "MAC",    "NULL"},
    {TLV_CODE_MANUF_DATE,    0,  "Board Mfg Date        :", NULL},
    {TLV_CODE_MAC_SIZE,      0,  "MAC Range             :", "55"},
    {TLV_CODE_LABEL_REVISION,0,  "Product Version       :", NULL},
    {TLV_CODE_PLATFORM_NAME, 0,  "Product Part Number   :", NULL},
    //{TLV_CODE_ONIE_VERSION,  4,  "Fixed"                  ,"Onie"},
    {TLV_CODE_MANUF_NAME,    0,  "Board Mfg             :", NULL},
    //{TLV_CODE_MANUF_COUNTRY, 2,  "Fixed",                "TW"},
    //{TLV_CODE_VENDOR_NAME,   8,  "Fixed",                "Edgecore"},
    //{TLV_CODE_DIAG_VERSION,  4,  "Fixed",                "diag"}
};


extern uint32_t
onlp_crc32(uint32_t crc, const void *buf, int size);

const char*
onlp_sysi_platform_get(void)
{
    return "x86-64-accton-csp9250-r0";
}

char *ptr=NULL, *ptr_start=NULL, *ptr_end=NULL;

int
onlp_sysi_onie_data_get(uint8_t** data, int* size)
{
    int byte=0,i=0;
    int index=0, new_tlv_len=0;   
    uint8_t* eeprom = aim_zmalloc(SYS_EEPROM_SIZE);
    char mac_addr[SYS_MAC_ADDR_LEN]={0};
	char cmd[SYS_MAX_LENGTH/4]={0};
	uint8_t  read_buf[SYS_MAX_LENGTH] = {0};	
    tlvinfo_header_t *eeprom_hdr = (tlvinfo_header_t *) eeprom;
    tlvinfo_tlv_t    *eeprom_tlv;
    tlvinfo_tlv_t    * eeprom_crc;
    unsigned int       calc_crc;
  
    
    memset(eeprom, 0x0, SYS_EEPROM_SIZE);
    strcpy(eeprom_hdr->signature, TLV_INFO_ID_STRING);
	eeprom_hdr->version = TLV_INFO_VERSION;
	eeprom_hdr->totallen =htons(0);
    
    system(SYS_IPMI_TMP_FILE_0_RM);
    system(SYS_IPMI_TMP_FILE_MAC_RM);
    snprintf(cmd , (SYS_MAX_LENGTH/4)-1, "%s > %s", SYS_NETWORK_MAC_GET, SYS_IPMI_TMP_FILE_MAC);
    system(cmd);
    if(onlp_file_read(read_buf, SYS_BUFFER_MAC_ADDR_LEN, &byte, SYS_IPMI_TMP_FILE_MAC)!=ONLP_STATUS_OK)
    {
       return ONLP_STATUS_E_INTERNAL;
    }
    sscanf((char*)read_buf, "%02x:%02x:%02x:%02x:%02x:%02x", (int*)&mac_addr[0], (int*)&mac_addr[1], (int*)&mac_addr[2], (int*)&mac_addr[3], (int*)&mac_addr[4], (int*)&mac_addr[5]);   
    
    snprintf(cmd , (SYS_MAX_LENGTH/4)-1, "%s > %s", SYS_IPMI_FRU_CMD_0, SYS_IPMI_TMP_FILE_0);
    system(cmd);	
	
	if(onlp_file_read(read_buf, SYS_MAX_LENGTH, &byte, SYS_IPMI_TMP_FILE_0)!=ONLP_STATUS_OK)
    {       
       return ONLP_STATUS_E_INTERNAL;
    }
   
    index +=sizeof(tlvinfo_header_t);
    
    for(i=0; i < sizeof(sys_eeprom_table)/sizeof(onlp_sys_eeprom_dev_t); i++)
    {
        if(sys_eeprom_table[i].size!=0)
        {
            eeprom_tlv = (tlvinfo_tlv_t *) &eeprom[index];
            eeprom_tlv->type = sys_eeprom_table[i].tcode;
            if(sys_eeprom_table[i].tcode==TLV_CODE_MAC_BASE)
            {
                eeprom_tlv->length = SYS_MAC_ADDR_LEN;
                memcpy(eeprom_tlv->value, mac_addr, eeprom_tlv->length);
                
            }
            else
            {
                eeprom_tlv->length = sys_eeprom_table[i].size;
                memcpy(eeprom_tlv->value, sys_eeprom_table[i].content, eeprom_tlv->length);
            }
            index+=sizeof(tlvinfo_tlv_t);
            index+=eeprom_tlv->length;
        }
        else
        {
            ptr=strstr((char*)read_buf, sys_eeprom_table[i].tag);
            if(ptr!=NULL)
            {
                ptr_end = strchr(ptr, '\n');
                ptr_start = strchr(ptr, ':');
                if(ptr_start !=NULL && ptr_end!=NULL)
                {
                    new_tlv_len=ptr_end-ptr_start-2;
                    eeprom_tlv = (tlvinfo_tlv_t *) &eeprom[index];
                    eeprom_tlv->type = sys_eeprom_table[i].tcode;
                    eeprom_tlv->length = new_tlv_len;
                    memcpy(eeprom_tlv->value, ptr_start+2, new_tlv_len);
                    index+=sizeof(tlvinfo_tlv_t);
                    index+=new_tlv_len;
                    
                }
            }
        }
        
    }
   
    eeprom_hdr->totallen = htons(index - sizeof(tlvinfo_header_t) +sizeof(tlvinfo_tlv_t)+4);
    
    eeprom_crc = (tlvinfo_tlv_t *) &eeprom[index];
    eeprom_crc->type = TLV_CODE_CRC_32;
    eeprom_crc->length = 4;
 
    /* Calculate the checksum */
    calc_crc = onlp_crc32(0, (void *)eeprom,
		     sizeof(tlvinfo_header_t) +
		     htons(eeprom_hdr->totallen) - 4);
    eeprom_crc->value[0] = (calc_crc >> 24) & 0xFF;
    eeprom_crc->value[1] = (calc_crc >> 16) & 0xFF;
    eeprom_crc->value[2] = (calc_crc >>  8) & 0xFF;
    eeprom_crc->value[3] = (calc_crc >>  0) & 0xFF;
    *size = SYS_EEPROM_SIZE;
    *data = eeprom;
    
    return ONLP_STATUS_OK;   
}

int
onlp_sysi_oids_get(onlp_oid_t* table, int max)
{
    int i;
    onlp_oid_t* e = table;
    memset(table, 0, max*sizeof(onlp_oid_t));
    
    /* 5 Thermal sensors on the chassis */
    for (i = 1; i <= CHASSIS_THERMAL_COUNT; i++) {
        *e++ = ONLP_THERMAL_ID_CREATE(i);
    }

    /* 5 LEDs on the chassis */
    for (i = 1; i <= CHASSIS_LED_COUNT; i++) {
        *e++ = ONLP_LED_ID_CREATE(i);
    }

    /* 2 PSUs on the chassis */
    for (i = 1; i <= CHASSIS_PSU_COUNT; i++) {
        *e++ = ONLP_PSU_ID_CREATE(i);
    }

    /* 6 Fans on the chassis */
    for (i = 1; i <= CHASSIS_FAN_COUNT; i++) {
        *e++ = ONLP_FAN_ID_CREATE(i);
    }

    return 0;
}

int
onlp_sysi_platform_info_get(onlp_platform_info_t* pi)
{
    int   i, v[NUM_OF_CPLD]={0};
	
    for (i = 0; i < NUM_OF_CPLD; i++) {
        v[i] = 0;
		
        if(onlp_file_read_int(v+i, "%s%s/version", PREFIX_PATH_ON_CPLD_DEV, arr_cplddev_name[i]) < 0) {
            return ONLP_STATUS_E_INTERNAL;
        }
    }
    pi->cpld_versions = aim_fstrdup("%d.%d.%d", v[0], v[1], v[2]);
	
    return 0;
}

void
onlp_sysi_platform_info_free(onlp_platform_info_t* pi)
{
    aim_free(pi->cpld_versions);
}

/* Thermal plan:
 * $TMP = (CPU_core + LM75_1+ LM75_2 + LM75_3 + LM75_4)/5
 * 1. If any FAN failed, set all the other fans as full speed, 100%.
 * 2. If any sensor is high than 45 degrees, set fan speed to duty 62.5%.
 * 3. If any sensor is high than 50 degrees, set fan speed to duty 100%.
 * 4. When $TMP >= 40 C, set fan speed to duty 62.5%.
 * 5. When $TMP >= 45 C, set fan speed to duty 100%.
 * 6. When $TMP <  35 C, set fan speed to duty 31.25%. 
 * 7. Direction factor, when B2F, duty + 12.5%.
 *
 * Note, all chassis fans share 1 single duty setting.
 */
fan_ctrl_policy_t  fan_ctrl_policy_avg[] = {
{FAN_DUTY_CYCLE_MAX     ,   45000,  INT_MIN},
{63                     ,   40000,  INT_MIN},
{32                     , INT_MAX,  35000},
};

fan_ctrl_policy_t  fan_ctrl_policy_single[] = {
{FAN_DUTY_CYCLE_MAX     ,   50000,  INT_MIN},
{63                     ,   45000,  INT_MIN},
};

struct fan_control_data_s {
   int duty_cycle; 
   int dir_plus;
   int mc_avg_pre[CELSIUS_RECORD_NUMBER];
   int mc_high_pre[CELSIUS_RECORD_NUMBER];
   
} fan_control_data_pre = 
{
    .duty_cycle = FAN_DUTY_CYCLE_DEFAULT,
    .dir_plus = 0,
    .mc_avg_pre = {INT_MIN+1, INT_MIN},    /*init as thermal rising to avoid full speed.*/
    .mc_high_pre = {INT_MIN+1, INT_MIN},    /*init as thermal rising to avoid full speed.*/
    
};

static int 
sysi_check_fan(uint32_t *fan_dir){
    int i, present;
    
    for (i = 1; i <= CHASSIS_FAN_COUNT; i++)
    {
        onlp_fan_info_t fan_info;

        if (onlp_fani_info_get(ONLP_FAN_ID_CREATE(i), &fan_info) != ONLP_STATUS_OK) {
            AIM_LOG_ERROR("Unable to get fan(%d) status\r\n", i);
            return ONLP_STATUS_E_INTERNAL;
        }

        present = fan_info.status & ONLP_FAN_STATUS_PRESENT;     
        if ((fan_info.status & ONLP_FAN_STATUS_FAILED) || !present) {
            AIM_LOG_WARN("Fan(%d) is not working, set the other fans as full speed\r\n", i);
            int ret = onlp_fani_percentage_set(
                    ONLP_FAN_ID_CREATE(FAN_ID_FOR_SET_FAN_DUTY), FAN_DUTY_CYCLE_MAX);
            if (ret != ONLP_STATUS_OK) 
                return ret; 
            else    
                return ONLP_STATUS_E_MISSING;                    
        }
        
        /* Get fan direction (Only get the first one since all fan direction are the same)
         */
        if (i == 1) {
            *fan_dir = fan_info.status & (ONLP_FAN_STATUS_F2B|ONLP_FAN_STATUS_B2F);
        }        
    }
    
    return ONLP_STATUS_OK;
}    

static int
sysi_get_fan_duty(int *cur_duty_cycle){
    int   fd, len;
    char  buf[10] = {0};
    char  *node = FAN_NODE(fan_duty_cycle_percentage);
    
    /* Get current fan duty*/
    fd = open(node, O_RDONLY);
    if (fd == -1){
        AIM_LOG_ERROR("Unable to open fan speed control node (%s)", node);
        return ONLP_STATUS_E_INTERNAL;
    }

    len = read(fd, buf, sizeof(buf));
    close(fd);
    if (len <= 0) {
        AIM_LOG_ERROR("Unable to read fan speed from (%s)", node);
        return ONLP_STATUS_E_INTERNAL;
    }
    *cur_duty_cycle = atoi(buf);
    
    return ONLP_STATUS_OK;    
}    

static int 
sysi_get_thermal_sum(int *mcelsius){
    onlp_thermal_info_t thermal_info;
    int i; 

    *mcelsius = 0;
    for (i = 1; i <= CHASSIS_THERMAL_COUNT; i++) {
        if (onlp_thermali_info_get(ONLP_THERMAL_ID_CREATE(i), &thermal_info) 
            != ONLP_STATUS_OK) {
            AIM_LOG_ERROR("Unable to read thermal status");
            return ONLP_STATUS_E_INTERNAL;
        }
        *mcelsius += thermal_info.mcelsius;

        DEBUG_PRINT("Thermal %d: %d \n ", i, thermal_info.mcelsius);
        
    }
    
    return ONLP_STATUS_OK;    
    
}

static int 
sysi_get_highest_thermal(int *mcelsius){
    onlp_thermal_info_t thermal_info;
    int i, highest; 

    highest = 0;
    for (i = 1; i <= CHASSIS_THERMAL_COUNT; i++) {
        if (onlp_thermali_info_get(ONLP_THERMAL_ID_CREATE(i), &thermal_info) 
            != ONLP_STATUS_OK) {
            AIM_LOG_ERROR("Unable to read thermal status");
            return ONLP_STATUS_E_INTERNAL;
        }
        highest = (thermal_info.mcelsius > highest)? 
                thermal_info.mcelsius : highest;
    }
    *mcelsius = highest;
    return ONLP_STATUS_OK;    
}

/* Anaylze thermal changing history to judge if the change is a stable trend. */
static int _is_thermal_a_trend(int *mc_history){
    int i, trend, trended;

    if (mc_history == NULL) {
        AIM_LOG_ERROR("Unable to get history of thermal\n");
        return 0;
    }
        
    /* Get heat up/down trend. */
    trend = 0;
    for (i = 0; i < CELSIUS_RECORD_NUMBER; i++) {    
        if (( mc_history[i+1] < mc_history[i])){  
            trend++;
        }else if (( mc_history[i+1] > mc_history[i])){    
            trend--;   
        }            
    }
    
    trended = (abs(trend) >= ((CELSIUS_RECORD_NUMBER+1)/2))? 1:0;
#if (DEBUG_MODE == 1)
    DEBUG_PRINT("[INFO]%s#%d, trended: %d, UP/DW: %d mcelsius:", 
            __func__, __LINE__, trended, trend );    
    for (i = 0; i <= CELSIUS_RECORD_NUMBER; i++) { 
        DEBUG_PRINT(" %d =>", mc_history[i]);
    }
    DEBUG_PRINT("%c\n", ' ');
#endif

    /*For more than half changes are same direction, it's a firm trend.*/
    return trended;
}


/* Decide duty by highest value of thermal sensors.*/
static int 
sysi_get_duty_by_highest(int *duty_cycle){
    int i, ret, maxtrix_len;
    int new_duty_cycle = 0 ;
    int mc_history[CELSIUS_RECORD_NUMBER+1] = {0};
    int *mcelsius_pre_p = &mc_history[1];
    int *mcelsius_now_p = &mc_history[0];    
    
    /* Fill up mcelsius array, 
     * [0] is current temperature, others are history.
     */
    ret = sysi_get_highest_thermal(mcelsius_now_p);
    if(ONLP_STATUS_OK != ret){
        return ret;
    }
    memcpy (mcelsius_pre_p, fan_control_data_pre.mc_high_pre, 
        sizeof(fan_control_data_pre.mc_high_pre));
    
    DEBUG_PRINT("[INFO]%s#%d, highest mcelsius:%d!\n", 
                __func__, __LINE__, *mcelsius_now_p);
    
    /* Shift records to the right */
    for (i = 0; i < CELSIUS_RECORD_NUMBER; i++) { 
        fan_control_data_pre.mc_high_pre[i] = mc_history[i];        
    }

    /* Only change duty on consecutive heat rising or falling.*/    
    maxtrix_len = AIM_ARRAYSIZE(fan_ctrl_policy_single);

    /* Only change duty when the thermal changing are firm. */    
    if (_is_thermal_a_trend(mc_history))
    {
        int matched = 0;
    	for (i = 0; i < maxtrix_len; i++) {
            if ((*mcelsius_now_p > fan_ctrl_policy_single[i].step_up_thermal)) {
        	    new_duty_cycle = fan_ctrl_policy_single[i].duty_cycle;
        	    matched = !matched;
        	    break;
        	}
        }
/*        if (!matched) {
            DEBUG_PRINT("%s#%d, celsius(%d) falls into undefined range!!\n", 
                    __func__, __LINE__, *mcelsius_now_p);
    	}    	*/
    }
    *duty_cycle = new_duty_cycle;
    return ONLP_STATUS_OK;
}

/* Decide duty by average value of thermal sensors.*/
static int 
sysi_get_duty_by_average(int *duty_cycle){
    int i, mcelsius_avg, ret, maxtrix_len;
    int new_duty_cycle=0;
    int mc_history[CELSIUS_RECORD_NUMBER+1] = {0};
    int *mcelsius_pre_p = &mc_history[1];
    int *mcelsius_now_p = &mc_history[0];    
    
    /* Fill up mcelsius array, 
     * [0] is current temperature, others are history.
     */
    *mcelsius_now_p = 0; 
    ret = sysi_get_thermal_sum(mcelsius_now_p);
    if(ONLP_STATUS_OK != ret){
        return ret;
    }
    mcelsius_avg = (*mcelsius_now_p)/CHASSIS_THERMAL_COUNT;

    memcpy (mcelsius_pre_p, fan_control_data_pre.mc_avg_pre, 
        sizeof(fan_control_data_pre.mc_avg_pre));

    DEBUG_PRINT("[INFO]%s#%d, mcelsius:%d!\n", __func__, __LINE__, mcelsius_avg);    

    /* Shift records to the right */
    for (i = 0; i < CELSIUS_RECORD_NUMBER; i++) { 
        fan_control_data_pre.mc_avg_pre[i] = mc_history[i];        
    }

    /* Only change duty on consecutive heat rising or falling.*/    
    maxtrix_len = AIM_ARRAYSIZE(fan_ctrl_policy_avg);    

    /* Only change duty when the thermal changing are firm. */
    if (_is_thermal_a_trend(mc_history))
    {
        int matched = 0;
    	for (i = 0; i < maxtrix_len; i++) {
            if ((mcelsius_avg >= fan_ctrl_policy_avg[i].step_up_thermal)) {
        	    new_duty_cycle = fan_ctrl_policy_avg[i].duty_cycle;
        	    matched = !matched;
        	    break;
        	}
        }
    	for (i = maxtrix_len-1; i>=0; i--) {        
    	    if ((mcelsius_avg < fan_ctrl_policy_avg[i].step_dn_thermal)) {
        	    new_duty_cycle = fan_ctrl_policy_avg[i].duty_cycle;
        	    matched = !matched;            	    
        	    break;
            }            	    
    	}
        /*if (!matched) {
            DEBUG_PRINT("%s#%d, celsius(%d) falls into undefined range!!\n", 
                    __func__, __LINE__, mcelsius_avg);
    	} */   	
    }
    
    *duty_cycle = new_duty_cycle;
    return ONLP_STATUS_OK;
}

int
onlp_sysi_platform_manage_fans(void)
{
    uint32_t fan_dir;
    int ret;
    int cur_duty_cycle, new_duty_cycle, tmp;    
    int direct_addon = 0;
    onlp_oid_t fan_duty_oid = ONLP_FAN_ID_CREATE(FAN_ID_FOR_SET_FAN_DUTY);
    
    /**********************************************************
     * Decision 1: Set fan as full speed if any fan is failed.
     **********************************************************/
    ret = sysi_check_fan(&fan_dir);
    if(ONLP_STATUS_OK != ret){
        return ret;
    }

    if (fan_dir & ONLP_FAN_STATUS_B2F) {
        direct_addon = FAN_DUTY_PLUS_FOR_DIR;
    }
    
    /**********************************************************
     * Decision 2: If no matched fan speed is found from the policy,
     *             use FAN_DUTY_CYCLE_MIN as default speed
     **********************************************************/
    ret = sysi_get_fan_duty(&cur_duty_cycle);
    if(ONLP_STATUS_OK != ret){
        return ret;
    }
	
    /**********************************************************
     * Decision 3: Decide new fan speed depend on fan direction and temperature
     **********************************************************/
    ret = sysi_get_duty_by_average(&new_duty_cycle);
    if (ONLP_STATUS_OK != ret){
        return ret;
    }    
    ret = sysi_get_duty_by_highest(&tmp);
    if (ONLP_STATUS_OK != ret){
        return ret;
    }
    
    new_duty_cycle = (tmp > new_duty_cycle)? tmp : new_duty_cycle;
    if (new_duty_cycle == 0)
    {
        new_duty_cycle = fan_control_data_pre.duty_cycle;
    } else {
        fan_control_data_pre.duty_cycle = new_duty_cycle; 
    }        
    fan_control_data_pre.dir_plus = direct_addon;    
    DEBUG_PRINT("[INFO]%s#%d, new duty: %d = %d + %d (%d)!\n", __func__, __LINE__, 
    new_duty_cycle + direct_addon, new_duty_cycle, direct_addon, cur_duty_cycle);
    
    new_duty_cycle += direct_addon;
    new_duty_cycle = (new_duty_cycle > FAN_DUTY_CYCLE_MAX)?
         FAN_DUTY_CYCLE_MAX : new_duty_cycle;
       
	if (new_duty_cycle == cur_duty_cycle) {
        /* Duty cycle does not change, just return */
	    return ONLP_STATUS_OK;
	}

    return onlp_fani_percentage_set(fan_duty_oid, new_duty_cycle);
}

int
onlp_sysi_platform_manage_leds(void)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

