#include <linux/module.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/slab.h>

#include "switch_psu_driver.h"
#include "switch_led_driver.h"

#define SWITCH_S3IP_PSU_VERSION "0.0.0.1"

unsigned int loglevel = 0;

struct psu_attribute{
    struct attribute attr;
    ssize_t (*show)(struct kobject *obj, struct psu_attribute *attr, char *buf);
    ssize_t (*store)(struct kobject *obj, struct psu_attribute *attr, const char *buf, size_t count);
};

struct psu_drivers_t *cb_func = NULL;


/* For s3ip */
extern struct kobject *switch_kobj;
static struct kobject *psu_kobj;
static struct kobject *psu_index_kobj[PSU_TOTAL_NUM];
static struct kobject *temp_index_kobj[PSU_TOTAL_NUM][PSU_TOTAL_TEMP_NUM];

enum psu_attrs {
    DEBUG,
    LOGLEVEL,
    PSU_NUM,
    PSU_MODEL_NAME,
    PSU_SERIAL_NUMBER,
    PSU_DATE,
    PSU_VENDOR,
    PSU_HARDWARE_VERSION,
    PSU_ALARM,
    PSU_ALARM_THRESHOLD_CURR,
    PSU_ALARM_THRESHOLD_VOL,
    PSU_PART_NUMBER,
    PSU_MAX_OUTPUT_POWER,
    PSU_IN_CURR,
    PSU_IN_VOL,
    PSU_IN_POWER,
    PSU_IN_STATUS,
    PSU_OUT_CURR,
    PSU_OUT_VOL,
    PSU_OUT_POWER,
    PSU_OUT_STATUS,
    PSU_PRESENT,
    PSU_LED_STATUS,
    PSU_TYPE,
    PSU_NUM_TEMP_SENSORS,
    PSU_FAN_SPEED,
    PSU_FAN_RATIO,
    DIRECTION,
    NUM_PSU_ATTR,
};

enum temp_attrs {
    TEMP_ALIAS,
    TEMP_TYPE,
    TEMP_MAX,
    TEMP_MAX_HYST,
    TEMP_MIN,
    TEMP_INPUT,
    NUM_TEMP_ATTR,
};

int get_psu_index(struct kobject *kobj)
{
    int retval=0;
    unsigned int psu_index;
    char psu_index_str[2] = {0};

#ifdef C11_ANNEX_K
    if(memcpy_s(psu_index_str, 2, (kobject_name(kobj)+sizeof(PSU_NAME_STRING)-1), 2) != 0)
    {
        return -ENOMEM;
    }
#else
    memcpy(psu_index_str, (kobject_name(kobj)+sizeof(PSU_NAME_STRING)-1), 2);
#endif

    retval = kstrtoint(psu_index_str, 10, &psu_index);
    if(retval == 0)
    {
        PSU_DEBUG("psu_index:%d \n", psu_index);
    }
    else
    {
        PSU_ERR("Error:%d, psu_index:%s \n", retval, psu_index_str);
        return -EINVAL;
    }

    if(!(psu_index <= PSU_TOTAL_NUM))
        return -EINVAL;

    return psu_index;
}

int get_psu_temp_index(struct kobject *kobj)
{
    int retval=0;
    unsigned int psu_temp_index;
    char psu_temp_index_str[2] = {0};

#ifdef C11_ANNEX_K
    if(memcpy_s(psu_temp_index_str, 2, (kobject_name(kobj)+sizeof(TEMP_NAME_STRING)-1), 2) != 0)
    {
        return -ENOMEM;
    }
#else
    memcpy(psu_temp_index_str, (kobject_name(kobj)+sizeof(TEMP_NAME_STRING)-1), 2);
#endif

    retval = kstrtoint(psu_temp_index_str, 10, &psu_temp_index);
    if(retval == 0)
    {
        PSU_DEBUG("psu_temp_index:%d \n", psu_temp_index);
    }
    else
    {
        PSU_ERR("Error:%d, psu_temp_index:%s \n", retval, psu_temp_index_str);
        return -EINVAL;
    }

    if((psu_temp_index <= 0) && (psu_temp_index > PSU_TOTAL_TEMP_NUM))
        return -EINVAL;

    return psu_temp_index;
}

static ssize_t s3ip_debug_help(struct kobject *kobj, struct psu_attribute *attr, char *buf)
{
    int retval=0;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    retval = cb_func->debug_help(buf);
    if(retval < 0)
    {
        ERROR_RETURN_NA(retval);
    }
    else
    {
        return retval;
    }
}

static ssize_t s3ip_debug(struct kobject *kobj, struct psu_attribute *attr, const char *buf, size_t count)
{
    return count;
}

static ssize_t s3ip_get_loglevel(struct kobject *kobj, struct psu_attribute *attr, char *buf)
{
#ifdef C11_ANNEX_K
    return sprintf_s(buf, PAGE_SIZE, "%d\n", loglevel);
#else
    return sprintf(buf, "%d\n", loglevel);
#endif
}

static ssize_t s3ip_set_loglevel(struct kobject *kobj, struct psu_attribute *attr, const char *buf, size_t count)
{
    int retval=0;
    long lev;

    if(cb_func == NULL)
        return -1;

    retval = kstrtol(buf, 0, &lev);
    if(retval == 0)
    {
        PSU_DEBUG("lev:%ld \n", lev);
    }
    else
    {
        PSU_ERR("Error:%d, lev:%s \n", retval, buf);
        return -1;
    }

    loglevel = (unsigned int)lev;

    cb_func->set_loglevel(lev);

    return count;
}

static ssize_t s3ip_psu_get_num(struct kobject *kobj, struct psu_attribute *attr, char *buf)
{
#ifdef C11_ANNEX_K
    return sprintf_s(buf, PAGE_SIZE, "%d\n", PSU_TOTAL_NUM);
#else
    return sprintf(buf, "%d\n", PSU_TOTAL_NUM);
#endif

}

static ssize_t s3ip_psu_get_direction(struct kobject *kobj, struct psu_attribute *attr, char *buf)
{
#ifdef C11_ANNEX_K
    return sprintf_s(buf, PAGE_SIZE, "%d\n", PSU_FAN_DIRECT_B2F);
#else
    return sprintf(buf, "%d\n", PSU_FAN_DIRECT_B2F);
#endif

}

static ssize_t s3ip_psu_get_psu_model_name(struct kobject *kobj, struct psu_attribute *attr, char *buf)
{
    int retval=0;
    char str_buff[PSU_MAX_STR_BUFF_LENGTH] = {0};
    int psu_index = 0;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    psu_index = get_psu_index(kobj);
    if(psu_index < 0)
    {
        PSU_ERR("Get psu index failed.\n");
        ERROR_RETURN_NA(-EINVAL);
    }

    retval = cb_func->get_mfr_info(psu_index, PMBUS_MFR_MODEL, str_buff);

    if(retval < 0)
    {
        ERROR_RETURN_NA(retval);
    }
    else
    {
#ifdef C11_ANNEX_K
        return sprintf_s(buf, PAGE_SIZE, "%s\n", str_buff);
#else
	    return sprintf(buf, "%s\n", str_buff);
#endif

    }
}

static ssize_t s3ip_psu_get_serial_number(struct kobject *kobj, struct psu_attribute *attr, char *buf)
{
    int retval=0;
    char str_buff[PSU_MAX_STR_BUFF_LENGTH] = {0};
    int psu_index = 0;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    psu_index = get_psu_index(kobj);
    if(psu_index < 0)
    {
        PSU_ERR("Get psu index failed.\n");
        ERROR_RETURN_NA(-EINVAL);
    }

    retval = cb_func->get_mfr_info(psu_index, PMBUS_MFR_SERIAL, str_buff);

    if(retval < 0)
    {
        ERROR_RETURN_NA(retval);
    }
    else
    {
#ifdef C11_ANNEX_K
        return sprintf_s(buf, PAGE_SIZE, "%s\n", str_buff);
#else
	    return sprintf(buf, "%s\n", str_buff);
#endif

    }
}

static ssize_t s3ip_psu_get_date(struct kobject *kobj, struct psu_attribute *attr, char *buf)
{
    int retval=0;
    char str_buff[PSU_MAX_STR_BUFF_LENGTH] = {0};
    int psu_index = 0;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    psu_index = get_psu_index(kobj);
    if(psu_index < 0)
    {
        PSU_ERR("Get psu index failed.\n");
        ERROR_RETURN_NA(-EINVAL);
    }

    retval = cb_func->get_mfr_info(psu_index, PMBUS_MFR_DATE, str_buff);

    if(retval < 0)
    {
        ERROR_RETURN_NA(retval);
    }
    else
    {
#ifdef C11_ANNEX_K
        return sprintf_s(buf, PAGE_SIZE, "%s\n", str_buff);
#else
	    return sprintf(buf, "%s\n", str_buff);
#endif

    }
}

static ssize_t s3ip_psu_get_vendor(struct kobject *kobj, struct psu_attribute *attr, char *buf)
{
    int retval=0;
    char str_buff[PSU_MAX_STR_BUFF_LENGTH] = {0};
    int psu_index = 0;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    psu_index = get_psu_index(kobj);
    if(psu_index < 0)
    {
        PSU_ERR("Get psu index failed.\n");
        ERROR_RETURN_NA(-EINVAL);
    }

    retval = cb_func->get_mfr_info(psu_index, PMBUS_MFR_ID, str_buff);

    if(retval < 0)
    {
        ERROR_RETURN_NA(retval);
    }
    else
    {
#ifdef C11_ANNEX_K
        return sprintf_s(buf, PAGE_SIZE, "%s\n", str_buff);
#else
	    return sprintf(buf, "%s\n", str_buff);
#endif

    }
}

static ssize_t s3ip_psu_get_hardware_version(struct kobject *kobj, struct psu_attribute *attr, char *buf)
{
    int retval=0;
    char str_buff[PSU_MAX_STR_BUFF_LENGTH] = {0};
    int psu_index = 0;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    psu_index = get_psu_index(kobj);
    if(psu_index < 0)
    {
        PSU_ERR("Get psu index failed.\n");
        ERROR_RETURN_NA(-EINVAL);
    }

    retval = cb_func->get_mfr_info(psu_index, PMBUS_MFR_REVISION, str_buff);

    if(retval < 0)
    {
        ERROR_RETURN_NA(retval);
    }
    else
    {
#ifdef C11_ANNEX_K
        return sprintf_s(buf, PAGE_SIZE, "%s\n", str_buff);
#else
	    return sprintf(buf, "%s\n", str_buff);
#endif

    }
}

static ssize_t s3ip_psu_get_alarm(struct kobject *kobj, struct psu_attribute *attr, char *buf)
{
    int psu_index = 0;
    unsigned int alarm_value = 0;
    
    int ret;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    psu_index = get_psu_index(kobj);
    if(psu_index < 0)
    {
        PSU_ERR("Get psu index failed.\n");
        ERROR_RETURN_NA(-EINVAL);
    }

    ret = cb_func->get_psu_pok_alarm(psu_index-1, &alarm_value);
    if(ret < 0)
    {
        ERROR_RETURN_NA(ret);
    }
#ifdef C11_ANNEX_K
    return sprintf_s(buf, PAGE_SIZE, "0x%x\n", alarm_value);
#else
    return sprintf(buf, "0x%x\n", alarm_value);
#endif

}

static ssize_t s3ip_psu_get_alarm_threshold_curr(struct kobject *kobj, struct psu_attribute *attr, char *buf)
{
    int retval=0;
    long threshold_curr = 0;
    int psu_index = 0;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    psu_index = get_psu_index(kobj);
    if(psu_index < 0)
    {
        PSU_ERR("Get psu index failed.\n");
        ERROR_RETURN_NA(-EINVAL);
    }

    retval = cb_func->get_alarm_threshold_curr(psu_index, &threshold_curr);

    if(retval == false)
    {
        ERROR_RETURN_NA(retval);
    }
    else
    {
#ifdef C11_ANNEX_K
        return sprintf_s(buf, PSU_MAX_STR_BUFF_LENGTH, "%ld\n", threshold_curr);
#else
	    return sprintf(buf, "%ld\n", threshold_curr);
#endif

    }
}

static ssize_t s3ip_psu_get_alarm_threshold_vol(struct kobject *kobj, struct psu_attribute *attr, char *buf)
{
    int retval=0;
    long threshold_vol = 0;
    int psu_index = 0;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    psu_index = get_psu_index(kobj);
    if(psu_index < 0)
    {
        PSU_ERR("Get psu index failed.\n");
        ERROR_RETURN_NA(-EINVAL);
    }

    retval = cb_func->get_alarm_threshold_vol(psu_index, &threshold_vol);

    if(retval == false)
    {
        ERROR_RETURN_NA(retval);
    }
    else
    {
#ifdef C11_ANNEX_K
        return sprintf_s(buf, PSU_MAX_STR_BUFF_LENGTH, "%ld\n", threshold_vol);
#else
	    return sprintf(buf, "%ld\n", threshold_vol);
#endif

    }
}

static ssize_t s3ip_psu_get_part_number(struct kobject *kobj, struct psu_attribute *attr, char *buf)
{
    int retval=0;
    char str_buff[PSU_MAX_STR_BUFF_LENGTH] = {0};
    int psu_index = 0;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    psu_index = get_psu_index(kobj);
    if(psu_index < 0)
    {
        PSU_ERR("Get psu index failed.\n");
        ERROR_RETURN_NA(-EINVAL);
    }

    retval = cb_func->get_mfr_info(psu_index, PMBUS_PART_NUM, str_buff);

    if(retval < 0)
    {
        ERROR_RETURN_NA(retval);
    }
    else
    {
#ifdef C11_ANNEX_K
        return sprintf_s(buf, PAGE_SIZE, "%s\n", str_buff);
#else
	    return sprintf(buf, "%s\n", str_buff);
#endif

    }
}

static ssize_t s3ip_psu_get_max_output_power(struct kobject *kobj, struct psu_attribute *attr, char *buf)
{
    int retval=0;
    long max_output_power = 0;
    int psu_index = 0;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    psu_index = get_psu_index(kobj);
    if(psu_index < 0)
    {
        PSU_ERR("Get psu index failed.\n");
        ERROR_RETURN_NA(-EINVAL);
    }

    retval = cb_func->get_max_output_power(psu_index, &max_output_power);

    if(retval == false)
    {
        ERROR_RETURN_NA(retval);
    }
    else
    {
#ifdef C11_ANNEX_K
        return sprintf_s(buf, PSU_MAX_STR_BUFF_LENGTH, "%ld\n", max_output_power);
#else
	    return sprintf(buf, "%ld\n", max_output_power);
#endif

    }
}

static ssize_t s3ip_psu_get_in_curr(struct kobject *kobj, struct psu_attribute *attr, char *buf)
{
    int retval=0;
    char str_buff[PSU_MAX_STR_BUFF_LENGTH] = {0};
    int psu_index = 0;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    psu_index = get_psu_index(kobj);
    if(psu_index < 0)
    {
        PSU_ERR("Get psu index failed.\n");
        ERROR_RETURN_NA(-EINVAL);
    }

    retval = cb_func->get_mfr_info(psu_index, PMBUS_READ_IIN, str_buff);

    if(retval < 0)
    {
        ERROR_RETURN_NA(retval);
    }
    else
    {
#ifdef C11_ANNEX_K
        return sprintf_s(buf, PSU_MAX_STR_BUFF_LENGTH, "%s\n", str_buff);
#else
	    return sprintf(buf, "%s\n", str_buff);
#endif

    }
}

static ssize_t s3ip_psu_get_in_vol(struct kobject *kobj, struct psu_attribute *attr, char *buf)
{
    int retval=0;
    char str_buff[PSU_MAX_STR_BUFF_LENGTH] = {0};
    int psu_index = 0;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    psu_index = get_psu_index(kobj);
    if(psu_index < 0)
    {
        PSU_ERR("Get psu index failed.\n");
        ERROR_RETURN_NA(-EINVAL);
    }

    retval = cb_func->get_mfr_info(psu_index, PMBUS_READ_VIN, str_buff);

    if(retval < 0)
    {
        ERROR_RETURN_NA(retval);
    }
    else
    {
#ifdef C11_ANNEX_K
        return sprintf_s(buf, PSU_MAX_STR_BUFF_LENGTH, "%s\n", str_buff);
#else
	    return sprintf(buf, "%s\n", str_buff);
#endif

    }
}

static ssize_t s3ip_psu_get_in_power(struct kobject *kobj, struct psu_attribute *attr, char *buf)
{
    int retval=0;
    char str_buff[PSU_MAX_STR_BUFF_LENGTH] = {0};
    int psu_index = 0;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    psu_index = get_psu_index(kobj);
    if(psu_index < 0)
    {
        PSU_ERR("Get psu index failed.\n");
        ERROR_RETURN_NA(-EINVAL);
    }

    retval = cb_func->get_mfr_info(psu_index, PMBUS_READ_PIN, str_buff);

    if(retval < 0)
    {
        ERROR_RETURN_NA(retval);
    }
    else
    {
#ifdef C11_ANNEX_K
        return sprintf_s(buf, PSU_MAX_STR_BUFF_LENGTH, "%s\n", str_buff);
#else
	    return sprintf(buf, "%s\n", str_buff);
#endif

    }
}

static ssize_t s3ip_psu_get_in_status(struct kobject *kobj, struct psu_attribute *attr, char *buf)
{
    int retval=0;
    int psu_index = 0;
    unsigned int value = 0;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    psu_index = get_psu_index(kobj);
    if(psu_index < 0)
    {
        PSU_ERR("Get psu index failed.\n");
        ERROR_RETURN_NA(-EINVAL);
    }

    retval = cb_func->get_psu_out_pok(psu_index-1, &value);

    if(retval < 0)
    {
        ERROR_RETURN_NA(retval);
    }
#ifdef C11_ANNEX_K
    /* IPOK return 1; else return 0 */
    return sprintf_s(buf, PAGE_SIZE, "%d\n", value ? 1 : 0);
#else
    return sprintf(buf, "%d\n", value ? 1 : 0);
#endif

}

static ssize_t s3ip_psu_get_out_curr(struct kobject *kobj, struct psu_attribute *attr, char *buf)
{
    int retval=0;
    char str_buff[PSU_MAX_STR_BUFF_LENGTH] = {0};
    int psu_index = 0;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    psu_index = get_psu_index(kobj);
    if(psu_index < 0)
    {
        PSU_ERR("Get psu index failed.\n");
        ERROR_RETURN_NA(-EINVAL);
    }

    retval = cb_func->get_mfr_info(psu_index, PMBUS_READ_IOUT, str_buff);

    if(retval < 0)
    {
        ERROR_RETURN_NA(retval);
    }
    else
    {
#ifdef C11_ANNEX_K
        return sprintf_s(buf, PSU_MAX_STR_BUFF_LENGTH, "%s\n", str_buff);
#else
	    return sprintf(buf, "%s\n", str_buff);
#endif

    }
}

static ssize_t s3ip_psu_get_out_vol(struct kobject *kobj, struct psu_attribute *attr, char *buf)
{
    int retval=0;
    char str_buff[PSU_MAX_STR_BUFF_LENGTH] = {0};
    int psu_index = 0;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    psu_index = get_psu_index(kobj);
    if(psu_index < 0)
    {
        PSU_ERR("Get psu index failed.\n");
        ERROR_RETURN_NA(-EINVAL);
    }

    retval = cb_func->get_mfr_info(psu_index,  PMBUS_READ_VOUT, str_buff);

    if(retval < 0)
    {
        ERROR_RETURN_NA(retval);
    }
    else
    {
#ifdef C11_ANNEX_K
        return sprintf_s(buf, PSU_MAX_STR_BUFF_LENGTH, "%s\n", str_buff);
#else
	    return sprintf(buf, "%s\n", str_buff);
#endif

    }
}

static ssize_t s3ip_psu_get_out_power(struct kobject *kobj, struct psu_attribute *attr, char *buf)
{
    int retval=0;
    char str_buff[PSU_MAX_STR_BUFF_LENGTH] = {0};
    int psu_index = 0;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    psu_index = get_psu_index(kobj);
    if(psu_index < 0)
    {
        PSU_ERR("Get psu index failed.\n");
        ERROR_RETURN_NA(-EINVAL);
    }

    retval = cb_func->get_mfr_info(psu_index, PMBUS_READ_POUT, str_buff);

    if(retval < 0)
    {
        ERROR_RETURN_NA(retval);
    }
    else
    {
#ifdef C11_ANNEX_K
        return sprintf_s(buf, PSU_MAX_STR_BUFF_LENGTH, "%s\n", str_buff);
#else
	    return sprintf(buf, "%s\n", str_buff);
#endif

    }
}

static ssize_t s3ip_psu_get_out_status(struct kobject *kobj, struct psu_attribute *attr, char *buf)
{
    int retval=0;
    int psu_index = 0;
    unsigned int value = 0;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    psu_index = get_psu_index(kobj);
    if(psu_index < 0)
    {
        PSU_ERR("Get psu index failed.\n");
        ERROR_RETURN_NA(-EINVAL);
    }

    retval = cb_func->get_psu_out_pok(psu_index-1, &value);

    if(retval < 0)
    {
        ERROR_RETURN_NA(retval);
    }
#ifdef C11_ANNEX_K
    /* OPOK return 1; else return 0 */
    return sprintf_s(buf, PAGE_SIZE, "%d\n", value ? 1 : 0);
#else
    return sprintf(buf, "%d\n", value ? 1 : 0);
#endif

}

static ssize_t s3ip_psu_get_present(struct kobject *kobj, struct psu_attribute *attr, char *buf)
{
    int retval=0;
    int psu_index = 0;
    unsigned int value = 0;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    psu_index = get_psu_index(kobj);
    if(psu_index < 0)
    {
        PSU_ERR("Get psu index failed.\n");
        ERROR_RETURN_NA(-EINVAL);
    }

    retval = cb_func->get_present(psu_index-1, &value);

    if(retval < 0)
    {
        ERROR_RETURN_NA(retval);
    }
#ifdef C11_ANNEX_K
    /* Present return 1; else return 0 */
    return sprintf_s(buf, PAGE_SIZE, "%d\n", value ? 1 : 0);
#else
    return sprintf(buf, "%d\n", value ? 1 : 0);
#endif

}

static ssize_t s3ip_psu_get_led_status(struct kobject *kobj, struct psu_attribute *attr, char *buf)
{
    ERROR_RETURN_NA(-EINVAL);
}

static ssize_t s3ip_psu_get_type(struct kobject *kobj, struct psu_attribute *attr, char *buf)
{
#ifdef C11_ANNEX_K
    // PSU only support AC, alway return 0: AC.
    return sprintf_s(buf, PAGE_SIZE, "0\n");
#else
    return sprintf(buf, "0\n");
#endif

}

static ssize_t s3ip_psu_get_num_temp_sensors(struct kobject *kobj, struct psu_attribute *attr, char *buf)
{
    int psu_index = 0;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    psu_index = get_psu_index(kobj);
    if(psu_index < 0)
    {
        PSU_ERR("Get psu index failed.\n");
        ERROR_RETURN_NA(-EINVAL);
    }

    return cb_func->get_num_temp_sensors(psu_index, buf);
}

static ssize_t s3ip_psu_get_fan_speed(struct kobject *kobj, struct psu_attribute *attr, char *buf)
{
    int retval=0;
    char str_buff[PSU_MAX_STR_BUFF_LENGTH] = {0};
    int psu_index = 0;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    psu_index = get_psu_index(kobj);
    if(psu_index < 0)
    {
        PSU_ERR("Get psu index failed.\n");
        ERROR_RETURN_NA(-EINVAL);
    }

    retval = cb_func->get_mfr_info(psu_index, PMBUS_READ_FAN_SPEED_1, str_buff);

    if(retval < 0)
    {
        ERROR_RETURN_NA(retval);
    }
    else
    {
#ifdef C11_ANNEX_K
        return sprintf_s(buf, PSU_MAX_STR_BUFF_LENGTH, "%s\n", str_buff);
#else
	    return sprintf(buf, "%s\n", str_buff);
#endif

    }
}

static ssize_t s3ip_psu_get_fan_ratio(struct kobject *kobj, struct psu_attribute *attr, char *buf)
{
    int retval=0;
    char str_buff[PSU_MAX_STR_BUFF_LENGTH] = {0};
    int psu_index = 0;
    unsigned int fan_speed;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    psu_index = get_psu_index(kobj);
    if(psu_index < 0)
    {
        PSU_ERR("Get psu index failed.\n");
        ERROR_RETURN_NA(-EINVAL);
    }

    retval = cb_func->get_mfr_info(psu_index, PMBUS_READ_FAN_SPEED_1, str_buff);

    if(retval < 0)
    {
        ERROR_RETURN_NA(retval);
    }
    else
    {
#ifdef C11_ANNEX_K
        if (sscanf_s(str_buff, "%d", &fan_speed) < 0)
#else
        if (sscanf(str_buff, "%d", &fan_speed) < 0)
#endif
        {
            ERROR_RETURN_NA(-EINVAL);
        }
#ifdef C11_ANNEX_K
        return sprintf_s(buf, PAGE_SIZE, "%d\n", (fan_speed*100)/PSU_FAN_SPEED_MAX_RPM);
#else
	    return sprintf(buf, "%d\n", (fan_speed*100)/PSU_FAN_SPEED_MAX_RPM);
#endif

    }
}

static ssize_t s3ip_psu_set_fan_ratio(struct kobject *kobj, struct psu_attribute *attr, const char *buf, size_t count)
{
    int psu_index = 0;

    if(cb_func == NULL)
    {
        return -EIO;
    }

    psu_index = get_psu_index(kobj);
    if(psu_index < 0)
    {
        PSU_ERR("Get psu index failed.\n");
        return -EINVAL;
    }

    /* PSU FAN speed manually control isn't supported. */
    return -EPERM;
}

static ssize_t s3ip_get_psu_temp_alias(struct kobject *kobj, struct psu_attribute *attr, char *buf)
{   
    int psu_index = 0;
    int psu_temp_index = 0;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    psu_index = get_psu_index(kobj->parent);
    if(psu_index < 0)
    {
        PSU_ERR("Get psu index failed.\n");
        ERROR_RETURN_NA(-EINVAL);
    }

    psu_temp_index = get_psu_temp_index(kobj);
    if(psu_temp_index < 0)
    {
        PSU_ERR("Get psu temp index failed.\n");
        ERROR_RETURN_NA(-EINVAL);
    }

    return cb_func->get_psu_temp_alias(psu_index, psu_temp_index, buf);
}

static ssize_t s3ip_get_psu_temp_type(struct kobject *kobj, struct psu_attribute *attr, char *buf)
{
    int psu_index = 0;
    int psu_temp_index = 0;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    psu_index = get_psu_index(kobj->parent);
    if(psu_index < 0)
    {
        PSU_ERR("Get psu index failed.\n");
        ERROR_RETURN_NA(-EINVAL);
    }

    psu_temp_index = get_psu_temp_index(kobj);
    if(psu_temp_index < 0)
    {
        PSU_ERR("Get psu temp index failed.\n");
        ERROR_RETURN_NA(-EINVAL);
    }

    return cb_func->get_psu_temp_type(psu_index, psu_temp_index, buf);
}

static ssize_t s3ip_get_psu_temp_max(struct kobject *kobj, struct psu_attribute *attr, char *buf)
{
    int psu_index = 0;
    int psu_temp_index = 0;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    psu_index = get_psu_index(kobj->parent);
    if(psu_index < 0)
    {
        PSU_ERR("Get psu index failed.\n");
        ERROR_RETURN_NA(-EINVAL);
    }

    psu_temp_index = get_psu_temp_index(kobj);
    if(psu_temp_index < 0)
    {
        PSU_ERR("Get psu temp index failed.\n");
        ERROR_RETURN_NA(-EINVAL);
    }

    return cb_func->get_psu_temp_max(psu_index, psu_temp_index, buf);
}

static ssize_t s3ip_set_psu_temp_max(struct kobject *kobj, struct psu_attribute *attr, const char *buf, size_t count)
{
    int retval=0;
    int psu_index;
    int psu_temp_index = 0;
    int temp_max;

    if(cb_func == NULL)
    {
        return -EIO;
    }

    psu_index = get_psu_index(kobj->parent);
    if(psu_index < 0)
    {
        PSU_ERR("Get psu index failed.\n");
        return -EINVAL;
    }

    psu_temp_index = get_psu_temp_index(kobj);
    if(psu_temp_index < 0)
    {
        PSU_ERR("Get psu temp index failed.\n");
        return -EINVAL;
    }

    retval = kstrtoint(buf, 10, &temp_max);
    if(retval == 0)
    {
        PSU_DEBUG("temp_max:%d \n", temp_max);
    }
    else
    {
        PSU_ERR("Error:%d, temp_max:%d \n", retval, temp_max);
        return -EIO;
    }

    retval = cb_func->set_psu_temp_max(psu_index, psu_temp_index, temp_max);
    if(retval < 0)
    {
        PSU_ERR("Set psu %d temp %d temp_max failed.\n", psu_index, psu_temp_index);
        return -EIO;
    }

    return count;
}

static ssize_t s3ip_get_psu_temp_max_hyst(struct kobject *kobj, struct psu_attribute *attr, char *buf)
{
    int psu_index = 0;
    int psu_temp_index = 0;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    psu_index = get_psu_index(kobj->parent);
    if(psu_index < 0)
    {
        PSU_ERR("Get psu index failed.\n");
        ERROR_RETURN_NA(-EINVAL);
    }

    psu_temp_index = get_psu_temp_index(kobj);
    if(psu_temp_index < 0)
    {
        PSU_ERR("Get psu temp index failed.\n");
        ERROR_RETURN_NA(-EINVAL);
    }

    return cb_func->get_psu_temp_max_hyst(psu_index, psu_temp_index, buf);
}

static ssize_t s3ip_get_psu_temp_min(struct kobject *kobj, struct psu_attribute *attr, char *buf)
{
    int psu_index = 0;
    int psu_temp_index = 0;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    psu_index = get_psu_index(kobj->parent);
    if(psu_index < 0)
    {
        PSU_ERR("Get psu index failed.\n");
        ERROR_RETURN_NA(-EINVAL);
    }

    psu_temp_index = get_psu_temp_index(kobj);
    if(psu_temp_index < 0)
    {
        PSU_ERR("Get psu temp index failed.\n");
        ERROR_RETURN_NA(-EINVAL);
    }

    return cb_func->get_psu_temp_min(psu_index, psu_temp_index, buf);
}

static ssize_t s3ip_set_psu_temp_min(struct kobject *kobj, struct psu_attribute *attr, const char *buf, size_t count)
{
    int retval=0;
    int psu_index;
    int psu_temp_index = 0;
    int temp_min;

    if(cb_func == NULL)
    {
        return -EIO;
    }

    psu_index = get_psu_index(kobj->parent);
    if(psu_index < 0)
    {
        PSU_ERR("Get psu index failed.\n");
        return -EINVAL;
    }

    psu_temp_index = get_psu_temp_index(kobj);
    if(psu_temp_index < 0)
    {
        PSU_ERR("Get psu temp index failed.\n");
        return -EINVAL;
    }

    retval = kstrtoint(buf, 10, &temp_min);
    if(retval == 0)
    {
        PSU_DEBUG("temp_min:%d \n", temp_min);
    }
    else
    {
        PSU_ERR("Error:%d, temp_min:%d \n", retval, temp_min);
        return -EIO;
    }

    retval = cb_func->set_psu_temp_min(psu_index, psu_temp_index, temp_min);
    if(retval < 0)
    {
        PSU_ERR("Set psu %d temp %d temp_min failed.\n", psu_index, psu_temp_index);
        return -EIO;
    }

    return count;
}

static ssize_t s3ip_get_psu_temp_input(struct kobject *kobj, struct psu_attribute *attr, char *buf)
{
    int retval=0;
    int psu_index = 0;
    int psu_temp_index = 0;
    long temp_input = 0;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    psu_index = get_psu_index(kobj->parent);
    if(psu_index < 0)
    {
        PSU_ERR("Get psu index failed.\n");
        ERROR_RETURN_NA(-EINVAL);
    }

    psu_temp_index = get_psu_temp_index(kobj);
    if(psu_temp_index < 0)
    {
        PSU_ERR("Get psu temp index failed.\n");
        ERROR_RETURN_NA(-EINVAL);
    }

    retval = cb_func->get_temp_input(psu_index, psu_temp_index, &temp_input);
    if(retval < 0)
    {
        PSU_ERR("Get temp input failed.\n");
        ERROR_RETURN_NA(-1);
    }
#ifdef C11_ANNEX_K
    return sprintf_s(buf, PSU_MAX_STR_BUFF_LENGTH, "%s%ld\n", ((temp_input < 0) ? "-":""), abs(temp_input));
#else
    return sprintf(buf, "%s%ld\n", ((temp_input < 0) ? "-":""), abs(temp_input));
#endif

}

static struct psu_attribute psu_attr[NUM_PSU_ATTR] = {
    [DEBUG]                     = {{.name = "debug",                .mode = S_IRUGO | S_IWUSR},     s3ip_debug_help,                     s3ip_debug},
    [LOGLEVEL]                  = {{.name = "loglevel",             .mode = S_IRUGO | S_IWUSR},     s3ip_get_loglevel,                   s3ip_set_loglevel},
    [PSU_NUM]                   = {{.name = "num",                  .mode = S_IRUGO},               s3ip_psu_get_num,                    NULL},
    [PSU_MODEL_NAME]            = {{.name = "model_name",           .mode = S_IRUGO},               s3ip_psu_get_psu_model_name,         NULL},
    [PSU_SERIAL_NUMBER]         = {{.name = "serial_number",        .mode = S_IRUGO},               s3ip_psu_get_serial_number,          NULL},
    [PSU_DATE]                  = {{.name = "date",                 .mode = S_IRUGO},               s3ip_psu_get_date,                   NULL},
    [PSU_VENDOR]                = {{.name = "vendor",               .mode = S_IRUGO},               s3ip_psu_get_vendor,                 NULL},
    [PSU_HARDWARE_VERSION]      = {{.name = "hardware_version",     .mode = S_IRUGO},               s3ip_psu_get_hardware_version,       NULL},
    [PSU_ALARM]                 = {{.name = "alarm",                .mode = S_IRUGO},               s3ip_psu_get_alarm,                  NULL},
    [PSU_ALARM_THRESHOLD_CURR]  = {{.name = "alarm_threshold_curr", .mode = S_IRUGO},               s3ip_psu_get_alarm_threshold_curr,   NULL},
    [PSU_ALARM_THRESHOLD_VOL]   = {{.name = "alarm_threshold_vol",  .mode = S_IRUGO},               s3ip_psu_get_alarm_threshold_vol,    NULL},
    [PSU_PART_NUMBER]           = {{.name = "part_number",          .mode = S_IRUGO},               s3ip_psu_get_part_number,            NULL},
    [PSU_MAX_OUTPUT_POWER]      = {{.name = "max_output_power",     .mode = S_IRUGO},               s3ip_psu_get_max_output_power,       NULL},
    [PSU_IN_CURR]               = {{.name = "in_curr",              .mode = S_IRUGO},               s3ip_psu_get_in_curr,                NULL},
    [PSU_IN_VOL]                = {{.name = "in_vol",               .mode = S_IRUGO},               s3ip_psu_get_in_vol,                 NULL},
    [PSU_IN_POWER]              = {{.name = "in_power",             .mode = S_IRUGO},               s3ip_psu_get_in_power,               NULL},
    [PSU_IN_STATUS]             = {{.name = "in_status",            .mode = S_IRUGO},               s3ip_psu_get_in_status,              NULL},
    [PSU_OUT_CURR]              = {{.name = "out_curr",             .mode = S_IRUGO},               s3ip_psu_get_out_curr,               NULL},
    [PSU_OUT_VOL]               = {{.name = "out_vol",              .mode = S_IRUGO},               s3ip_psu_get_out_vol,                NULL},
    [PSU_OUT_POWER]             = {{.name = "out_power",            .mode = S_IRUGO},               s3ip_psu_get_out_power,              NULL},
    [PSU_OUT_STATUS]            = {{.name = "out_status",           .mode = S_IRUGO},               s3ip_psu_get_out_status,             NULL},
    [PSU_PRESENT]               = {{.name = "present",              .mode = S_IRUGO},               s3ip_psu_get_present,                NULL},
    [PSU_LED_STATUS]            = {{.name = "led_status",           .mode = S_IRUGO},               s3ip_psu_get_led_status,             NULL},
    [PSU_TYPE]                  = {{.name = "type",                 .mode = S_IRUGO},               s3ip_psu_get_type,                   NULL},
    [PSU_NUM_TEMP_SENSORS]      = {{.name = "num_temp_sensors",     .mode = S_IRUGO},               s3ip_psu_get_num_temp_sensors,       NULL},
    [PSU_FAN_SPEED]             = {{.name = "fan_speed",            .mode = S_IRUGO},               s3ip_psu_get_fan_speed,              NULL},
    [PSU_FAN_RATIO]             = {{.name = "fan_ratio",            .mode = S_IRUGO | S_IWUSR},     s3ip_psu_get_fan_ratio,              s3ip_psu_set_fan_ratio},
    [DIRECTION]                 = {{.name = "direction",            .mode = S_IRUGO},               s3ip_psu_get_direction,              NULL},
};

static struct psu_attribute temp_attr[NUM_TEMP_ATTR] = {
    [TEMP_ALIAS]            = {{.name = "temp_alias",               .mode = S_IRUGO},           s3ip_get_psu_temp_alias,         NULL},
    [TEMP_TYPE]             = {{.name = "temp_type",                .mode = S_IRUGO},           s3ip_get_psu_temp_type,          NULL},
    [TEMP_MAX]              = {{.name = "temp_max",                 .mode = S_IRUGO | S_IWUSR}, s3ip_get_psu_temp_max,           s3ip_set_psu_temp_max},
    [TEMP_MAX_HYST]         = {{.name = "temp_max_hyst",            .mode = S_IRUGO},           s3ip_get_psu_temp_max_hyst,      NULL},
    [TEMP_MIN]              = {{.name = "temp_min",                 .mode = S_IRUGO | S_IWUSR}, s3ip_get_psu_temp_min,           s3ip_set_psu_temp_min},
    [TEMP_INPUT]            = {{.name = "temp_input",               .mode = S_IRUGO},           s3ip_get_psu_temp_input,         NULL},
};

void s3ip_psu_drivers_register(struct psu_drivers_t *pfunc)
{
    cb_func = pfunc;

    return;
}
EXPORT_SYMBOL_GPL(s3ip_psu_drivers_register);

void s3ip_psu_drivers_unregister(void)
{
    cb_func = NULL;

    return;
}
EXPORT_SYMBOL_GPL(s3ip_psu_drivers_unregister);

static int __init switch_psu_init(void)
{
    int err=0;
    int retval=0;
    int i;
    int psu_index, temp_index;    
    char *psu_index_str, *temp_index_str;

    psu_index_str = (char *)kzalloc(sizeof(PSU_NAME_STRING)+3, GFP_KERNEL);
    if (!psu_index_str)
    {
        PSU_ERR("Fail to alloc psu_index_str memory\n");
        return -ENOMEM;
    }

    temp_index_str = (char *)kzalloc(7*sizeof(char), GFP_KERNEL);
    if (!temp_index_str)
    {
        PSU_ERR("Fail to alloc temp_index_str memory\n");
        err =  -ENOMEM;
        goto drv_allocate_temp_index_str_failed;
    }

    /* For s3ip */
    psu_kobj = kobject_create_and_add(PSU_NAME_STRING, switch_kobj);
    if(!psu_kobj)
    {
        PSU_ERR("Failed to create 'psu'\n");
        err = -ENOMEM;
        goto sysfs_create_kobject_psu_failed; 
    }

    for(i=0; i <= PSU_NUM; i++)
    {
        PSU_DEBUG("sysfs_create_file /psu/%s\n", psu_attr[i].attr.name);
        retval = sysfs_create_file(psu_kobj, &psu_attr[i].attr);
        if(retval)
        {
            PSU_ERR("Failed to create file '%s'\n", psu_attr[i].attr.name);
            err = -retval;
            goto sysfs_create_s3ip_attr_failed;
        }
    }

    for(psu_index=0; psu_index<PSU_TOTAL_NUM; psu_index++)
    {
#ifdef C11_ANNEX_K
        if(sprintf_s(psu_index_str, sizeof(PSU_NAME_STRING)+3, "%s%d", PSU_NAME_STRING, psu_index+1) < 0)
#else
        if(sprintf(psu_index_str, "%s%d", PSU_NAME_STRING, psu_index+1) < 0)
#endif
        {
            err = -ENOMEM;
            goto sysfs_create_kobject_switch_psu_index_failed;
        }
        psu_index_kobj[psu_index] = kobject_create_and_add(psu_index_str, psu_kobj);
        if(!psu_index_kobj[psu_index])
        {
            PSU_ERR("Failed to create 'psu%d'\n", psu_index+1);
            err = -ENOMEM;
            goto sysfs_create_kobject_switch_psu_index_failed;
        }

        for(i=PSU_MODEL_NAME; i < NUM_PSU_ATTR; i++)
        {
            PSU_DEBUG("sysfs_create_file /psu/psu%d/%s\n", psu_index, psu_attr[i].attr.name);
            retval = sysfs_create_file(psu_index_kobj[psu_index], &psu_attr[i].attr);
            if(retval)
            {
                PSU_ERR("Failed to create file '%s'\n", psu_attr[i].attr.name);
                err = -retval;
                goto sysfs_create_s3ip_psu_attr_failed;
            }
        }

        for(temp_index=0; temp_index<PSU_TOTAL_TEMP_NUM; temp_index++)
        {
#ifdef C11_ANNEX_K
            if(sprintf_s(temp_index_str, 7, "%s%d", TEMP_NAME_STRING, temp_index+1) < 0)
#else
            if(sprintf(temp_index_str, "%s%d", TEMP_NAME_STRING, temp_index+1) < 0)
#endif
            {
                err = -ENOMEM;
                goto sysfs_create_kobject_switch_temp_index_failed;
            }
            temp_index_kobj[psu_index][temp_index] = kobject_create_and_add(temp_index_str, psu_index_kobj[psu_index]);
            if(!temp_index_kobj[psu_index][temp_index])
            {
                PSU_DEBUG("Failed to create 'temp%d'\n", temp_index);
                err = -ENOMEM;
                goto sysfs_create_kobject_switch_temp_index_failed;
            }

            for(i=TEMP_ALIAS; i<NUM_TEMP_ATTR; i++)
            {
                PSU_DEBUG("sysfs_create_file /switch/psu/psu%d/%s/%s\n", psu_index+1, temp_index_str, temp_attr[i].attr.name);

                retval = sysfs_create_file(temp_index_kobj[psu_index][temp_index], &temp_attr[i].attr);
                if(retval)
                {
                    PSU_ERR("Failed to create file '%s'\n", temp_attr[i].attr.name);
                    err = -retval;
                    goto sysfs_create_s3ip_temp_attrs_failed;
                }
            }
        }
    }

    kfree(psu_index_str);
    kfree(temp_index_str);

    return 0;

sysfs_create_s3ip_temp_attrs_failed:
sysfs_create_kobject_switch_temp_index_failed:
sysfs_create_s3ip_psu_attr_failed:
sysfs_create_kobject_switch_psu_index_failed:
    for(i=0; i <= PSU_NUM; i++)
        sysfs_remove_file(psu_kobj, &psu_attr[i].attr);

    for(psu_index=0; psu_index<PSU_TOTAL_NUM; psu_index++)
    {
        if(psu_index_kobj[psu_index])
        {
            for(i=PSU_MODEL_NAME; i < NUM_PSU_ATTR; i++)
                sysfs_remove_file(psu_index_kobj[psu_index], &psu_attr[i].attr);

            for(temp_index=0; temp_index<PSU_TOTAL_TEMP_NUM; temp_index++)
            {
                if(temp_index_kobj[psu_index][temp_index])
                {
                    for(i=TEMP_ALIAS; i<NUM_TEMP_ATTR; i++)
                        sysfs_remove_file(temp_index_kobj[psu_index][temp_index], &temp_attr[i].attr);
                }
                
                kobject_put(temp_index_kobj[psu_index][temp_index]);
                temp_index_kobj[psu_index][temp_index] = NULL;
            }

            kobject_put(psu_index_kobj[psu_index]);
            psu_index_kobj[psu_index] = NULL;
        }
    }

sysfs_create_s3ip_attr_failed:
    if(psu_kobj)
    {
        kobject_put(psu_kobj);
        psu_kobj = NULL;
    }

sysfs_create_kobject_psu_failed:
    kfree(temp_index_str);

drv_allocate_temp_index_str_failed:
    kfree(psu_index_str);

    return err;
}

static void __exit switch_psu_exit(void)
{
    int i;
    int psu_index, temp_index;

    /* For s3ip */
    for(i=0; i <= PSU_NUM; i++)
        sysfs_remove_file(psu_kobj, &psu_attr[i].attr);

    for(psu_index=0; psu_index<PSU_TOTAL_NUM; psu_index++)
    {
        if(psu_index_kobj[psu_index])
        {
            for(i=PSU_MODEL_NAME; i < NUM_PSU_ATTR; i++)
                sysfs_remove_file(psu_index_kobj[psu_index], &psu_attr[i].attr);

            for(temp_index=0; temp_index<PSU_TOTAL_TEMP_NUM; temp_index++)
            {
                if(temp_index_kobj[psu_index][temp_index])
                {
                    for(i=TEMP_ALIAS; i<NUM_TEMP_ATTR; i++)
                        sysfs_remove_file(temp_index_kobj[psu_index][temp_index], &temp_attr[i].attr);
                }
                
                kobject_put(temp_index_kobj[psu_index][temp_index]);
                temp_index_kobj[psu_index][temp_index] = NULL;
            }

            kobject_put(psu_index_kobj[psu_index]);
            psu_index_kobj[psu_index] = NULL;
        }
    }

    if(psu_kobj)
    {
        kobject_put(psu_kobj);
        psu_kobj = NULL;
    }

    cb_func = NULL;

    return;
}

MODULE_DESCRIPTION("Switch S3IP PSU Driver");
MODULE_VERSION(SWITCH_S3IP_PSU_VERSION);
MODULE_LICENSE("GPL");

module_init(switch_psu_init);
module_exit(switch_psu_exit);
