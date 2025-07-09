#include <linux/module.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/slab.h>

#include "switch_sensor_driver.h"

#define SWITCH_S3IP_SENSOR_VERSION "0.0.0.1"

unsigned int loglevel = 0;
struct sensor_drivers_t *cb_func = NULL;

struct sensor_attribute{
    struct attribute attr;
    ssize_t (*show)(struct kobject *obj, struct sensor_attribute *attr, char *buf);
    ssize_t (*store)(struct kobject *obj, struct sensor_attribute *attr, const char *buf, size_t count);
};

/* For s3ip */
extern struct kobject *switch_kobj;
static struct kobject *sensor_kobj;
static struct kobject *temp_index_kobj[TEMP_TOTAL_NUM];
static struct kobject *vol_index_kobj[VOL_TOTAL_NUM];
static struct kobject *curr_index_kobj[CURR_TOTAL_NUM];

enum sensor_attrs {
    DEBUG,
    LOGLEVEL,
    NUM_TEMP_SENSORS,
    NUM_VOL_SENSORS,
    NUM_CURR_SENSORS,
    TEMP_ALIAS,
    TEMP_TYPE,
    TEMP_MAX,
    TEMP_HYST,
    TEMP_MIN,
    TEMP_INPUT,
    VOL_ALIAS,
    VOL_TYPE,
    VOL_MAX,
    VOL_MIN,
    VOL_INPUT,
    RANGE,
    VOL_NOMINAL_VALUE,
    CURR_ALIAS,
    CURR_TYPE,
    CURR_MAX,
    CURR_MIN,
    CURR_INPUT,
    NUM_SENSOR_ATTR,
};

int get_temp_index(struct kobject *kobj)
{
    int retval=0;
    unsigned int temp_index;
    char temp_index_str[2] = {0};

#ifdef C11_ANNEX_K
    if(memcpy_s(temp_index_str, 2, (kobject_name(kobj)+4), 2) != 0)
    {
        return -ENOMEM;
    }
#else
    memcpy(temp_index_str, (kobject_name(kobj)+4), 2);
#endif

    retval = kstrtoint(temp_index_str, 10, &temp_index);
    if(retval == 0)
    {
        SENSOR_DEBUG("temp_index:%d \n", temp_index);
    }
    else
    {
        SENSOR_DEBUG("Error:%d, temp_index:%s \n", retval, temp_index_str);
        return -1;
    }

    return temp_index;
}

int get_vol_index(struct kobject *kobj)
{
    int retval=0;
    unsigned int vol_index;
    char vol_index_str[3] = {0};

#ifdef C11_ANNEX_K
    if(memcpy_s(vol_index_str, 2, (kobject_name(kobj)+3), 2) != 0)
    {
        return -ENOMEM;
    }
#else
    memcpy(vol_index_str, (kobject_name(kobj)+3), 2);
#endif
    retval = kstrtoint(vol_index_str, 10, &vol_index);
    if(retval == 0)
    {
        SENSOR_DEBUG("vol_index:%d \n", vol_index);
    }
    else
    {
        SENSOR_DEBUG("Error:%d, vol_index:%s \n", retval, vol_index_str);
        return -1;
    }

    return vol_index;
}

int get_curr_index(struct kobject *kobj)
{
    int retval=0;
    unsigned int curr_index;
    char curr_index_str[2] = {0};

#ifdef C11_ANNEX_K
    if(memcpy_s(curr_index_str, 2, (kobject_name(kobj)+4), 2) != 0)
    {
        return -ENOMEM;
    }
#else
    memcpy(curr_index_str, (kobject_name(kobj)+4), 2);
#endif

    retval = kstrtoint(curr_index_str, 10, &curr_index);
    if(retval == 0)
    {
        SENSOR_DEBUG("curr_index:%d \n", curr_index);
    }
    else
    {
        SENSOR_DEBUG("Error:%d, curr_index:%s \n", retval, curr_index_str);
        return -1;
    }

    return curr_index;
}

static ssize_t s3ip_debug_help(struct kobject *kobj, struct sensor_attribute *attr, char *buf)
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

static ssize_t s3ip_debug(struct kobject *kobj, struct sensor_attribute *attr, const char *buf, size_t count)
{
    return count;
}

static ssize_t s3ip_get_loglevel(struct kobject *kobj, struct sensor_attribute *attr, char *buf)
{
#ifdef C11_ANNEX_K
    return sprintf_s(buf, PAGE_SIZE, "%d\n", loglevel);
#else
    return sprintf(buf, "%d\n", loglevel);
#endif
}

static ssize_t s3ip_set_loglevel(struct kobject *kobj, struct sensor_attribute *attr, const char *buf, size_t count)
{
    int retval=0;
    long lev;

    if(cb_func == NULL)
        return -1;

    retval = kstrtol(buf, 0, &lev);
    if(retval == 0)
    {
        SENSOR_DEBUG("lev:%ld \n", lev);
    }
    else
    {
        SENSOR_DEBUG("Error:%d, lev:%s \n", retval, buf);
        return -1;
    }

    loglevel = (unsigned int)lev;

    cb_func->set_loglevel(lev);

    return count;
}

static ssize_t s3ip_get_temp_num(struct kobject *kobj, struct sensor_attribute *attr, char *buf)
{
#ifdef C11_ANNEX_K
    return sprintf_s(buf, PAGE_SIZE, "%d\n", TEMP_TOTAL_NUM);
#else
    return sprintf(buf, "%d\n", TEMP_TOTAL_NUM);
#endif

}

static ssize_t s3ip_get_vol_num(struct kobject *kobj, struct sensor_attribute *attr, char *buf)
{
#ifdef C11_ANNEX_K
    return sprintf_s(buf, PAGE_SIZE, "%d\n", VOL_TOTAL_NUM);
#else
    return sprintf(buf, "%d\n", VOL_TOTAL_NUM);
#endif

}

static ssize_t s3ip_get_curr_num(struct kobject *kobj, struct sensor_attribute *attr, char *buf)
{
#ifdef C11_ANNEX_K
    return sprintf_s(buf, PAGE_SIZE, "%d\n", CURR_TOTAL_NUM);
#else
	return sprintf(buf, "%d\n", CURR_TOTAL_NUM);
#endif

}

static ssize_t s3ip_get_temp_alias(struct kobject *kobj, struct sensor_attribute *attr, char *buf)
{
    int retval=0;
    int temp_index;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    temp_index = get_temp_index(kobj);
    if(temp_index < 0)
    {
        SENSOR_DEBUG("Get temp index failed.\n");
        return -1;
    }

    retval = cb_func->get_temp_alias(temp_index-1, buf);
    if(retval < 0)
    {
        ERROR_RETURN_NA(retval);
    }
    else
    {
        return retval;
    }
}

static ssize_t s3ip_get_temp_type(struct kobject *kobj, struct sensor_attribute *attr, char *buf)
{
    int retval=0;
    int temp_index;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    temp_index = get_temp_index(kobj);
    if(temp_index < 0)
    {
        SENSOR_DEBUG("Get temp index failed.\n");
        ERROR_RETURN_NA(-1);
    }

    retval = cb_func->get_temp_type(temp_index-1, buf);
    if(retval < 0)
    {
        ERROR_RETURN_NA(retval);
    }
    else
    {
        return retval;
    }
}

static ssize_t s3ip_get_temp_max(struct kobject *kobj, struct sensor_attribute *attr, char *buf)
{
    int retval=0;
    int temp_index;
    long temp_max;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    temp_index = get_temp_index(kobj);
    if(temp_index < 0)
    {
        SENSOR_DEBUG("Get temp index failed.\n");
        ERROR_RETURN_NA(-1);
    }

    retval = cb_func->get_temp_max(temp_index-1, &temp_max);
    if(retval == false)
    {
        SENSOR_DEBUG("Get temp max failed.\n");
        ERROR_RETURN_NA(-1);
    }
#ifdef C11_ANNEX_K
    return sprintf_s(buf, PAGE_SIZE, "%s%ld\n", ((temp_max < 0) ? "-":""), abs(temp_max));
#else
    return sprintf(buf, "%s%ld\n", ((temp_max < 0) ? "-":""), abs(temp_max));
#endif

}

static ssize_t s3ip_set_temp_max(struct kobject *kobj, struct sensor_attribute *attr, const char *buf, size_t count)
{
    int retval=0;
    int temp_index;
    int temp_value;

    if(cb_func == NULL)
    {
        return -EIO;
    }

    temp_index = get_temp_index(kobj);
    if(temp_index < 0)
    {
        SENSOR_DEBUG("Get temp index failed.\n");
        return -EINVAL;
    }
    retval = kstrtoint(buf, 0, &temp_value);
    if(retval == 0)
    {
        SENSOR_DEBUG("temp%d temp_max data:%d \n", temp_index, temp_value);
    }
    else
    {
        SENSOR_DEBUG("Error:%d, temp%d temp_max data:%s \n", retval, temp_index, buf);
        return -EIO;
    }

    retval = cb_func->set_temp_max(temp_index-1, temp_value);
    if(retval < 0)
    {
        SENSOR_DEBUG("Set temp%d max failed.\n",temp_index);
        return -EIO;
    }

    return count;
}

static ssize_t s3ip_get_temp_max_hyst(struct kobject *kobj, struct sensor_attribute *attr, char *buf)
{
    int retval=0;
    int temp_index;
    long temp_max_hyst;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    temp_index = get_temp_index(kobj);
    if(temp_index < 0)
    {
        SENSOR_DEBUG("Get temp index failed.\n");
        ERROR_RETURN_NA(-1);
    }

    retval = cb_func->get_temp_max_hyst(temp_index-1, &temp_max_hyst);
    if(retval == false)
    {
        SENSOR_DEBUG("Get temp max hyst failed.\n");
        ERROR_RETURN_NA(-1);
    }
#ifdef C11_ANNEX_K
    return sprintf_s(buf, PAGE_SIZE, "%s%ld\n", ((temp_max_hyst < 0) ? "-":""), abs(temp_max_hyst));
#else
    return sprintf(buf, "%s%ld\n", ((temp_max_hyst < 0) ? "-":""), abs(temp_max_hyst));
#endif

}

static ssize_t s3ip_get_temp_min(struct kobject *kobj, struct sensor_attribute *attr, char *buf)
{
    int retval=0;
    int temp_index;
    long temp_min;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    temp_index = get_temp_index(kobj);
    if(temp_index < 0)
    {
        SENSOR_DEBUG("Get temp index failed.\n");
        ERROR_RETURN_NA(-1);
    }

    retval = cb_func->get_temp_min(temp_index-1, &temp_min);
    if(retval == false)
    {
        SENSOR_DEBUG("Get temp min failed.\n");
        ERROR_RETURN_NA(-1);
    }
#ifdef C11_ANNEX_K
    return sprintf_s(buf, PAGE_SIZE, "%s%ld\n", ((temp_min < 0) ? "-":""), abs(temp_min));
#else
    return sprintf(buf, "%s%ld\n", ((temp_min < 0) ? "-":""), abs(temp_min));
#endif

}

static ssize_t s3ip_set_temp_min(struct kobject *kobj, struct sensor_attribute *attr, const char *buf, size_t count)
{
    int retval=0;
    int temp_index;
    int temp_value;

    if(cb_func == NULL)
    {
        return -EIO;
    }

    temp_index = get_temp_index(kobj);
    if(temp_index < 0)
    {
        SENSOR_DEBUG("Get temp index failed.\n");
        return -EINVAL;
    }
    retval = kstrtoint(buf, 0, &temp_value);
    if(retval == 0)
    {
        SENSOR_DEBUG("temp%d temp_min data:%d \n", temp_index, temp_value);
    }
    else
    {
        SENSOR_DEBUG("Error:%d, temp%d temp_min data:%s \n", retval, temp_index, buf);
        return -EIO;
    }

    retval = cb_func->set_temp_min(temp_index-1, temp_value);
    if(retval < 0)
    {
        SENSOR_DEBUG("Set temp%d min failed.\n",temp_index);
        return -EIO;
    }

    return count;
}

static ssize_t s3ip_get_temp_input(struct kobject *kobj, struct sensor_attribute *attr, char *buf)
{
    int retval=0;
    int temp_index;
    long temp_input = 0;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    temp_index = get_temp_index(kobj);
    if(temp_index < 0)
    {
        SENSOR_DEBUG("Get temp index failed.\n");
        ERROR_RETURN_NA(-1);
    }

    retval = cb_func->get_temp_input(temp_index-1, &temp_input);
    if(retval == false)
    {
        SENSOR_DEBUG("Get temp input failed.\n");
        ERROR_RETURN_NA(-1);
    }
#ifdef C11_ANNEX_K
    return sprintf_s(buf, PAGE_SIZE, "%s%ld\n", ((temp_input < 0) ? "-":""), abs(temp_input));
#else
    return sprintf(buf, "%s%ld\n", ((temp_input < 0) ? "-":""), abs(temp_input));
#endif

}

static ssize_t s3ip_get_vol_alias(struct kobject *kobj, struct sensor_attribute *attr, char *buf)
{
    int retval=0;
    int vol_index;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    vol_index = get_vol_index(kobj);
    if(vol_index < 0)
    {
        SENSOR_DEBUG("Get vol index failed.\n");
        ERROR_RETURN_NA(-1);
    }

    retval = cb_func->get_vol_alias(vol_index-1, buf);
    if(retval < 0)
    {
        ERROR_RETURN_NA(retval);
    }
    else
    {
        return retval;
    }
}

static ssize_t s3ip_get_vol_type(struct kobject *kobj, struct sensor_attribute *attr, char *buf)
{
    int retval=0;
    int vol_index;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    vol_index = get_vol_index(kobj);
    if(vol_index < 0)
    {
        SENSOR_DEBUG("Get vol index failed.\n");
        ERROR_RETURN_NA(-1);
    }

    retval = cb_func->get_vol_type(vol_index-1, buf);
    if(retval < 0)
    {
        ERROR_RETURN_NA(retval);
    }
    else
    {
        return retval;
    }
}

static ssize_t s3ip_get_vol_max(struct kobject *kobj, struct sensor_attribute *attr, char *buf)
{
    int retval=0;
    int vol_index;
    long vol_max;
    
    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    vol_index = get_vol_index(kobj);
    if(vol_index < 0)
    {
        SENSOR_DEBUG("Get vol index failed.\n");
        ERROR_RETURN_NA(-1);
    }

    retval = cb_func->get_vol_max(vol_index-1, &vol_max);
    if(retval == false)
    {
        SENSOR_DEBUG("Get vol max failed.\n");
        ERROR_RETURN_NA(-1);
    }
#ifdef C11_ANNEX_K
    return sprintf_s(buf, PAGE_SIZE, "%s%ld\n", ((vol_max < 0) ? "-":""), abs(vol_max));
#else
    return sprintf(buf, "%s%ld\n", ((vol_max < 0) ? "-":""), abs(vol_max));
#endif

}

static ssize_t s3ip_set_vol_max(struct kobject *kobj, struct sensor_attribute *attr, const char *buf, size_t count)
{
    int retval=0;
    int vol_index;
    int vol_value;

    if(cb_func == NULL)
    {
        return -EIO;
    }

    vol_index = get_vol_index(kobj);
    if(vol_index < 0)
    {
        SENSOR_DEBUG("Get vol index failed.\n");
        return -EINVAL;
    }
    retval = kstrtoint(buf, 0, &vol_value);
    if(retval == 0)
    {
        SENSOR_DEBUG("vol%d vol_max data:%d \n", vol_index, vol_value);
    }
    else
    {
        SENSOR_DEBUG("Error:%d, vol%d vol_max data:%s \n", retval, vol_index, buf);
        return -EIO;
    }

    retval = cb_func->set_vol_max(vol_index-1, vol_value);
    if(retval < 0)
    {
        SENSOR_DEBUG("Set vol%d max failed.\n",vol_index);
        return -EIO;
    }

    return count;
}

static ssize_t s3ip_get_vol_min(struct kobject *kobj, struct sensor_attribute *attr, char *buf)
{
    int retval=0;
    int vol_index;
    long vol_min;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    vol_index = get_vol_index(kobj);
    if(vol_index < 0)
    {
        SENSOR_DEBUG("Get vol index failed.\n");
        ERROR_RETURN_NA(-1);
    }

    retval = cb_func->get_vol_min(vol_index-1, &vol_min);
    if(retval == false)
    {
        SENSOR_DEBUG("Get vol min failed.\n");
        ERROR_RETURN_NA(-1);
    }
#ifdef C11_ANNEX_K
    return sprintf_s(buf, PAGE_SIZE, "%s%ld\n", ((vol_min < 0) ? "-":""), abs(vol_min));
#else
    return sprintf(buf, "%s%ld\n", ((vol_min < 0) ? "-":""), abs(vol_min));
#endif

}

static ssize_t s3ip_set_vol_min(struct kobject *kobj, struct sensor_attribute *attr, const char *buf, size_t count)
{
    int retval=0;
    int vol_index;
    int vol_value;

    if(cb_func == NULL)
    {
        return -EIO;
    }

    vol_index = get_vol_index(kobj);
    if(vol_index < 0)
    {
        SENSOR_DEBUG("Get vol index failed.\n");
        return -EINVAL;
    }
    retval = kstrtoint(buf, 0, &vol_value);
    if(retval == 0)
    {
        SENSOR_DEBUG("vol%d vol_min data:%d \n", vol_index, vol_value);
    }
    else
    {
        SENSOR_DEBUG("Error:%d, vol%d vol_min data:%s \n", retval, vol_index, buf);
        return -EIO;
    }

    retval = cb_func->set_vol_min(vol_index-1, vol_value);
    if(retval < 0)
    {
        SENSOR_DEBUG("Set vol%d min failed.\n",vol_index);
        return -EIO;
    }

    return count;
}

static ssize_t s3ip_get_vol_nominal_value(struct kobject *kobj, struct sensor_attribute *attr, char *buf)
{
    int retval=0;
    int vol_index;
    long vol_nominal;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    vol_index = get_vol_index(kobj);
    if(vol_index < 0)
    {
        SENSOR_DEBUG("Get vol index failed.\n");
        ERROR_RETURN_NA(-1);
    }

    retval = cb_func->get_vol_nominal(vol_index-1, &vol_nominal);
    if(retval == false)
    {
        SENSOR_DEBUG("Get vol nominal failed.\n");
        ERROR_RETURN_NA(-1);
    }
#ifdef C11_ANNEX_K
    return sprintf_s(buf, PAGE_SIZE, "%ld\n", vol_nominal);
#else
    return sprintf(buf, "%ld\n", vol_nominal);
#endif

}

static ssize_t s3ip_get_vol_input(struct kobject *kobj, struct sensor_attribute *attr, char *buf)
{
    int retval=0;
    int vol_index;
    long vol_input;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    vol_index = get_vol_index(kobj);
    if(vol_index < 0)
    {
        SENSOR_DEBUG("Get vol index failed.\n");
        ERROR_RETURN_NA(-1);
    }

    retval = cb_func->get_vol_input(vol_index-1, &vol_input);
    if(retval == false)
    {
        SENSOR_DEBUG("Get vol input failed.\n");
        ERROR_RETURN_NA(-1);
    }
#ifdef C11_ANNEX_K
    return sprintf_s(buf, PAGE_SIZE, "%s%ld\n", ((vol_input < 0) ? "-":""), abs(vol_input));
#else
    return sprintf(buf, "%s%ld\n", ((vol_input < 0) ? "-":""), abs(vol_input));
#endif

}

static ssize_t s3ip_get_vol_range(struct kobject *kobj, struct sensor_attribute *attr, char *buf)
{
    int retval=0;
    int vol_index;
    long vol_range;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    vol_index = get_vol_index(kobj);
    if(vol_index < 0)
    {
        SENSOR_DEBUG("Get vol index failed.\n");
        ERROR_RETURN_NA(-1);
    }

    retval = cb_func->get_vol_range(vol_index-1, &vol_range);
    if(retval == false)
    {
        SENSOR_DEBUG("Get vol range failed.\n");
        ERROR_RETURN_NA(-1);
    }
#ifdef C11_ANNEX_K
    return sprintf_s(buf, PAGE_SIZE, "%ld\n", vol_range);
#else
    return sprintf(buf, "%ld\n", vol_range);
#endif

}


static ssize_t s3ip_get_curr_alias(struct kobject *kobj, struct sensor_attribute *attr, char *buf)
{
    int retval=0;
    int curr_index;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    curr_index = get_curr_index(kobj);
    if(curr_index < 0)
    {
        SENSOR_DEBUG("Get curr index failed.\n");
        return -1;
    }

    retval = cb_func->get_curr_alias(curr_index-1, buf);
    if(retval < 0)
    {
        ERROR_RETURN_NA(retval);
    }
    else
    {
        return retval;
    }
}

static ssize_t s3ip_get_curr_type(struct kobject *kobj, struct sensor_attribute *attr, char *buf)
{
    int retval=0;
    int curr_index;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    curr_index = get_curr_index(kobj);
    if(curr_index < 0)
    {
        SENSOR_DEBUG("Get curr index failed.\n");
        ERROR_RETURN_NA(-1);
    }

    retval = cb_func->get_curr_type(curr_index-1, buf);
    if(retval < 0)
    {
        ERROR_RETURN_NA(retval);
    }
    else
    {
        return retval;
    }
}

static ssize_t s3ip_get_curr_max(struct kobject *kobj, struct sensor_attribute *attr, char *buf)
{
    int retval=0;
    int curr_index;
    long curr_max;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    curr_index = get_curr_index(kobj);
    if(curr_index < 0)
    {
        SENSOR_DEBUG("Get curr index failed.\n");
        ERROR_RETURN_NA(-1);
    }

    retval = cb_func->get_curr_max(curr_index-1, &curr_max);
    if(retval == false)
    {
        SENSOR_DEBUG("Get curr max failed.\n");
        ERROR_RETURN_NA(-1);
    }
#ifdef C11_ANNEX_K
    return sprintf_s(buf, PAGE_SIZE, "%s%ld\n", ((curr_max < 0) ? "-":""), abs(curr_max));
#else
    return sprintf(buf, "%s%ld\n", ((curr_max < 0) ? "-":""), abs(curr_max));
#endif

}

static ssize_t s3ip_set_curr_max(struct kobject *kobj, struct sensor_attribute *attr, const char *buf, size_t count)
{
    int retval=0;
    int curr_index;
    int curr_value;

    if(cb_func == NULL)
    {
        return -EIO;
    }

    curr_index = get_curr_index(kobj);
    if(curr_index < 0)
    {
        SENSOR_DEBUG("Get curr index failed.\n");
        return -EINVAL;
    }
    retval = kstrtoint(buf, 0, &curr_value);
    if(retval == 0)
    {
        SENSOR_DEBUG("curr%d curr_max data:%d \n", curr_index, curr_value);
    }
    else
    {
        SENSOR_DEBUG("Error:%d, curr%d curr_max data:%s \n", retval, curr_index, buf);
        return -EIO;
    }

    retval = cb_func->set_curr_max(curr_index-1, curr_value);
    if(retval < 0)
    {
        SENSOR_DEBUG("Set curr%d max failed.\n",curr_index);
        return -EIO;
    }

    return count;
}

static ssize_t s3ip_get_curr_min(struct kobject *kobj, struct sensor_attribute *attr, char *buf)
{
    int retval=0;
    int curr_index;
    long curr_min;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    curr_index = get_curr_index(kobj);
    if(curr_index < 0)
    {
        SENSOR_DEBUG("Get curr index failed.\n");
        ERROR_RETURN_NA(-1);
    }

    retval = cb_func->get_curr_min(curr_index-1, &curr_min);
    if(retval == false)
    {
        SENSOR_DEBUG("Get curr min failed.\n");
        ERROR_RETURN_NA(-1);
    }
#ifdef C11_ANNEX_K
    return sprintf_s(buf, PAGE_SIZE, "%s%ld\n", ((curr_min < 0) ? "-":""), abs(curr_min));
#else
    return sprintf(buf, "%s%ld\n", ((curr_min < 0) ? "-":""), abs(curr_min));
#endif

}

static ssize_t s3ip_set_curr_min(struct kobject *kobj, struct sensor_attribute *attr, const char *buf, size_t count)
{
    int retval=0;
    int curr_index;
    int curr_value;

    if(cb_func == NULL)
    {
        return -EIO;
    }

    curr_index = get_curr_index(kobj);
    if(curr_index < 0)
    {
        SENSOR_DEBUG("Get curr index failed.\n");
        return -EINVAL;
    }
    retval = kstrtoint(buf, 0, &curr_value);
    if(retval == 0)
    {
        SENSOR_DEBUG("curr%d curr_min data:%d \n", curr_index, curr_value);
    }
    else
    {
        SENSOR_DEBUG("Error:%d, curr%d curr_min data:%s \n", retval, curr_index, buf);
        return -EIO;
    }

    retval = cb_func->set_curr_min(curr_index-1, curr_value);
    if(retval < 0)
    {
        SENSOR_DEBUG("Set curr%d min failed.\n",curr_index);
        return -EIO;
    }

    return count;
}

static ssize_t s3ip_get_curr_input(struct kobject *kobj, struct sensor_attribute *attr, char *buf)
{
    int retval=0;
    int curr_index;
    long curr_input;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    curr_index = get_curr_index(kobj);
    if(curr_index < 0)
    {
        SENSOR_DEBUG("Get curr index failed.\n");
        ERROR_RETURN_NA(-1);
    }

    retval = cb_func->get_curr_input(curr_index-1, &curr_input);
    if(retval == false)
    {
        SENSOR_DEBUG("Get curr input failed.\n");
        ERROR_RETURN_NA(-1);
    }
#ifdef C11_ANNEX_K
    return sprintf_s(buf, PAGE_SIZE, "%s%ld\n", ((curr_input < 0) ? "-":""), abs(curr_input));
#else
    return sprintf(buf, "%s%ld\n", ((curr_input < 0) ? "-":""), abs(curr_input));
#endif

}

static struct sensor_attribute sensor_attr[NUM_SENSOR_ATTR] = {
    [DEBUG]                 = {{.name = "debug",                    .mode = S_IRUGO | S_IWUSR},     s3ip_debug_help,             s3ip_debug},
    [LOGLEVEL]              = {{.name = "loglevel",                 .mode = S_IRUGO | S_IWUSR},     s3ip_get_loglevel,           s3ip_set_loglevel},
    [NUM_TEMP_SENSORS]      = {{.name = "num_temp_sensors",         .mode = S_IRUGO},               s3ip_get_temp_num,           NULL},
    [NUM_VOL_SENSORS]       = {{.name = "num_vol_sensors",          .mode = S_IRUGO},               s3ip_get_vol_num,            NULL},
    [NUM_CURR_SENSORS]      = {{.name = "num_curr_sensors",         .mode = S_IRUGO},               s3ip_get_curr_num,           NULL},
    [TEMP_ALIAS]            = {{.name = "temp_alias",               .mode = S_IRUGO},               s3ip_get_temp_alias,         NULL},
    [TEMP_TYPE]             = {{.name = "temp_type",                .mode = S_IRUGO},               s3ip_get_temp_type,          NULL},
    [TEMP_MAX]              = {{.name = "temp_max",                 .mode = S_IRUGO | S_IWUSR},     s3ip_get_temp_max,           s3ip_set_temp_max},
    [TEMP_HYST]             = {{.name = "temp_hyst",                .mode = S_IRUGO},               s3ip_get_temp_max_hyst,      NULL},
    [TEMP_MIN]              = {{.name = "temp_min",                 .mode = S_IRUGO | S_IWUSR},     s3ip_get_temp_min,           s3ip_set_temp_min},
    [TEMP_INPUT]            = {{.name = "temp_input",               .mode = S_IRUGO},               s3ip_get_temp_input,         NULL}, 
    [VOL_ALIAS]             = {{.name = "vol_alias",                .mode = S_IRUGO},               s3ip_get_vol_alias,          NULL},
    [VOL_TYPE]              = {{.name = "vol_type",                 .mode = S_IRUGO},               s3ip_get_vol_type,           NULL},
    [VOL_MAX]               = {{.name = "vol_max",                  .mode = S_IRUGO | S_IWUSR},     s3ip_get_vol_max,            s3ip_set_vol_max},
    [VOL_MIN]               = {{.name = "vol_min",                  .mode = S_IRUGO | S_IWUSR},     s3ip_get_vol_min,            s3ip_set_vol_min},
    [VOL_INPUT]             = {{.name = "vol_input",                .mode = S_IRUGO},               s3ip_get_vol_input,          NULL},
    [RANGE]                 = {{.name = "range",                    .mode = S_IRUGO},               s3ip_get_vol_range,          NULL},
    [VOL_NOMINAL_VALUE]     = {{.name = "nominal_value",            .mode = S_IRUGO},               s3ip_get_vol_nominal_value,  NULL},
    [CURR_ALIAS]            = {{.name = "curr_alias",               .mode = S_IRUGO},               s3ip_get_curr_alias,         NULL},
    [CURR_TYPE]             = {{.name = "curr_type",                .mode = S_IRUGO},               s3ip_get_curr_type,          NULL},
    [CURR_MAX]              = {{.name = "curr_max",                 .mode = S_IRUGO | S_IWUSR},     s3ip_get_curr_max,           s3ip_set_curr_max},
    [CURR_MIN]              = {{.name = "curr_min",                 .mode = S_IRUGO | S_IWUSR},     s3ip_get_curr_min,           s3ip_set_curr_min},
    [CURR_INPUT]            = {{.name = "curr_input",               .mode = S_IRUGO},               s3ip_get_curr_input,         NULL},
};

void s3ip_sensor_drivers_register(struct sensor_drivers_t *pfunc)
{
    cb_func = pfunc;

    return;
}
EXPORT_SYMBOL_GPL(s3ip_sensor_drivers_register);

void s3ip_sensor_drivers_unregister(void)
{
    cb_func = NULL;

    return;
}
EXPORT_SYMBOL_GPL(s3ip_sensor_drivers_unregister);

static int __init switch_sensor_init(void)
{
    int err=0;
    int retval=0;
    int i;
    int temp_index, vol_index, curr_index;
    char *temp_index_str, *vol_index_str, *curr_index_str;

    temp_index_str = (char *)kzalloc(7*sizeof(char), GFP_KERNEL);
    if (!temp_index_str)
    {
        SENSOR_DEBUG( "Fail to alloc temp_index_str memory\n");
        return -ENOMEM;
    }

    vol_index_str = (char *)kzalloc(5*sizeof(char), GFP_KERNEL);
    if (!vol_index_str)
    {
        SENSOR_DEBUG( "Fail to alloc vol_index_str memory\n");
        err = -ENOMEM;
        goto drv_allocate_vol_index_str_failed;
    }

    curr_index_str = (char *)kzalloc(7*sizeof(char), GFP_KERNEL);
    if (!curr_index_str)
    {
        SENSOR_DEBUG( "Fail to alloc curr_index_str memory\n");
        err = -ENOMEM;
        goto drv_allocate_curr_index_str_failed;
    }

    /* For s3ip */
    sensor_kobj = kobject_create_and_add("sensor", switch_kobj);
    if(!sensor_kobj)
    {
        SENSOR_DEBUG( "Failed to create 'sensor'\n");
        err = -ENOMEM;
        goto sysfs_create_kobject_sensor_failed; 
    }

    for(i=0; i<=NUM_CURR_SENSORS; i++)
    {
        SENSOR_DEBUG( "sysfs_create_file /sensor/%s\n", sensor_attr[i].attr.name);
        retval = sysfs_create_file(sensor_kobj, &sensor_attr[i].attr);
        if(retval)
        {
            printk(KERN_ERR "Failed to create file '%s'\n", sensor_attr[i].attr.name);
            err = -retval;
            goto sysfs_create_s3ip_attr_num_temp_sensors_failed;
        }
    }

    for(temp_index=0; temp_index<TEMP_TOTAL_NUM; temp_index++)
    {
#ifdef C11_ANNEX_K
        if(sprintf_s(temp_index_str, 7, "temp%d", temp_index+1) < 0)
#else
        if(sprintf(temp_index_str, "temp%d", temp_index+1) < 0)
#endif

        {
            err = -ENOMEM;
            goto sysfs_create_kobject_sensor_temp_index_failed;
        }
        temp_index_kobj[temp_index] = kobject_create_and_add(temp_index_str, sensor_kobj);
        if(!temp_index_kobj[temp_index])
        {
            SENSOR_DEBUG( "Failed to create '%s'\n", temp_index_str);
            err = -ENOMEM;
            goto sysfs_create_kobject_sensor_temp_index_failed;
        }

        for(i=TEMP_ALIAS; i<=TEMP_INPUT; i++)
        {
            SENSOR_DEBUG( "sysfs_create_file /sensor/%s/%s\n", temp_index_str, sensor_attr[i].attr.name);
            retval = sysfs_create_file(temp_index_kobj[temp_index], &sensor_attr[i].attr);
            if(retval)
            {
                printk(KERN_ERR "Failed to create file '%s'\n", sensor_attr[i].attr.name);
                err = -retval;
                goto sysfs_create_s3ip_sensor_temp_attr_failed;
            }
        }
    }

    for(vol_index=0; vol_index<VOL_TOTAL_NUM; vol_index++)
    {
#ifdef C11_ANNEX_K
        if(sprintf_s(vol_index_str, 6, "vol%d", vol_index+1) < 0)
#else
        if(sprintf(vol_index_str, "vol%d", vol_index+1) < 0)
#endif

        {
            err = -ENOMEM;
            goto sysfs_create_kobject_sensor_vol_index_failed;
        }
        vol_index_kobj[vol_index] = kobject_create_and_add(vol_index_str, sensor_kobj);
        if(!vol_index_kobj[vol_index])
        {
            SENSOR_DEBUG( "Failed to create '%s'\n", vol_index_str);
            err = -ENOMEM;
            goto sysfs_create_kobject_sensor_vol_index_failed;
        }

        for(i=VOL_ALIAS; i<=VOL_NOMINAL_VALUE; i++)
        {
            SENSOR_DEBUG( "sysfs_create_file /sensor/%s/%s\n", vol_index_str, sensor_attr[i].attr.name);
            retval = sysfs_create_file(vol_index_kobj[vol_index], &sensor_attr[i].attr);
            if(retval)
            {
                printk(KERN_ERR "Failed to create file '%s'\n", sensor_attr[i].attr.name);
                err = -retval;
                goto sysfs_create_s3ip_sensor_vol_attr_failed;
            }
        }
    }

    for(curr_index=0; curr_index<CURR_TOTAL_NUM; curr_index++)
    {
#ifdef C11_ANNEX_K
        if(sprintf_s(curr_index_str, 7, "curr%d", curr_index+1) < 0)
#else
        if(sprintf(curr_index_str, "curr%d", curr_index+1) < 0)
#endif

        {
            err = -ENOMEM;
            goto sysfs_create_kobject_sensor_curr_index_failed;
        }
        curr_index_kobj[curr_index] = kobject_create_and_add(curr_index_str, sensor_kobj);
        if(!curr_index_kobj[curr_index])
        {
            SENSOR_DEBUG( "Failed to create '%s'\n", curr_index_str);
            err = -ENOMEM;
            goto sysfs_create_kobject_sensor_curr_index_failed;
        }

        for(i=CURR_ALIAS; i<=CURR_INPUT; i++)
        {
            SENSOR_DEBUG( "sysfs_create_file /sensor/%s/%s\n", curr_index_str, sensor_attr[i].attr.name);
            retval = sysfs_create_file(curr_index_kobj[curr_index], &sensor_attr[i].attr);
            if(retval)
            {
                printk(KERN_ERR "Failed to create file '%s'\n", sensor_attr[i].attr.name);
                err = -retval;
                goto sysfs_create_s3ip_sensor_curr_attr_failed;
            }
        }
    }

    kfree(temp_index_str);
    kfree(vol_index_str);
    kfree(curr_index_str);

    return 0;

sysfs_create_s3ip_sensor_curr_attr_failed:
sysfs_create_kobject_sensor_curr_index_failed:
    for(curr_index=0; curr_index<CURR_TOTAL_NUM; curr_index++)
    {
        if(curr_index_kobj[curr_index])
        {
            for(i=CURR_ALIAS; i<=CURR_INPUT; i++)
                sysfs_remove_file(curr_index_kobj[curr_index], &sensor_attr[i].attr);

            kobject_put(curr_index_kobj[curr_index]);
            curr_index_kobj[curr_index] = NULL;
        }
    }

sysfs_create_s3ip_sensor_vol_attr_failed:
sysfs_create_kobject_sensor_vol_index_failed: 
    for(vol_index=0; vol_index<VOL_TOTAL_NUM; vol_index++)
    {
        if(vol_index_kobj[vol_index])
        {
            for(i=VOL_ALIAS; i<=RANGE; i++)
                sysfs_remove_file(vol_index_kobj[vol_index], &sensor_attr[i].attr);

            kobject_put(vol_index_kobj[vol_index]);
            vol_index_kobj[vol_index] = NULL;
        }
    }

sysfs_create_s3ip_sensor_temp_attr_failed:
sysfs_create_kobject_sensor_temp_index_failed:

    for(temp_index=0; temp_index<TEMP_TOTAL_NUM; temp_index++)
    {
        if(temp_index_kobj[temp_index])
        {
            for(i=TEMP_ALIAS; i<=TEMP_INPUT; i++)
                sysfs_remove_file(temp_index_kobj[temp_index], &sensor_attr[i].attr);

            kobject_put(temp_index_kobj[temp_index]);
            temp_index_kobj[temp_index] = NULL;
        }
    }

    for(i=0; i<=NUM_CURR_SENSORS; i++)
        sysfs_remove_file(sensor_kobj, &sensor_attr[i].attr);

sysfs_create_s3ip_attr_num_temp_sensors_failed:
    if(sensor_kobj)
    {
        kobject_put(sensor_kobj);
        sensor_kobj = NULL;
    }

sysfs_create_kobject_sensor_failed:
    kfree(curr_index_str);

drv_allocate_curr_index_str_failed:
    kfree(vol_index_str);

drv_allocate_vol_index_str_failed:
    kfree(temp_index_str);

    return err;
}

static void __exit switch_sensor_exit(void)
{
    int i;
    int temp_index, vol_index, curr_index;

    /* For s3ip */
    for(i=0; i<=NUM_CURR_SENSORS; i++)
        sysfs_remove_file(sensor_kobj, &sensor_attr[i].attr);

    for(temp_index=0; temp_index<TEMP_TOTAL_NUM; temp_index++)
    {
        if(temp_index_kobj[temp_index])
        {
            for(i=TEMP_ALIAS; i<=TEMP_INPUT; i++)
                sysfs_remove_file(temp_index_kobj[temp_index], &sensor_attr[i].attr);
    
            kobject_put(temp_index_kobj[temp_index]);
            temp_index_kobj[temp_index] = NULL;
        }
    }

    for(vol_index=0; vol_index<VOL_TOTAL_NUM; vol_index++)
    {
        if(vol_index_kobj[vol_index])
        {
            for(i=VOL_ALIAS; i<=RANGE; i++)
                sysfs_remove_file(vol_index_kobj[vol_index], &sensor_attr[i].attr);

            kobject_put(vol_index_kobj[vol_index]);
            vol_index_kobj[vol_index] = NULL;
        }
    }

    for(curr_index=0; curr_index<CURR_TOTAL_NUM; curr_index++)
    {
        if(curr_index_kobj[curr_index])
        {
            for(i=CURR_ALIAS; i<=CURR_INPUT; i++)
                sysfs_remove_file(curr_index_kobj[curr_index], &sensor_attr[i].attr);

            kobject_put(curr_index_kobj[curr_index]);
            curr_index_kobj[curr_index] = NULL;
        }
    }

    if(sensor_kobj)
    {
        kobject_put(sensor_kobj);
        sensor_kobj = NULL;
    }

    cb_func = NULL;

    return;
}

MODULE_DESCRIPTION("Switch S3IP Sensor Driver");
MODULE_VERSION(SWITCH_S3IP_SENSOR_VERSION);
MODULE_LICENSE("GPL");

module_init(switch_sensor_init);
module_exit(switch_sensor_exit);
