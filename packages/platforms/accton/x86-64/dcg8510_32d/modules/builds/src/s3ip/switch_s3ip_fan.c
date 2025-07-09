#include <linux/module.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/slab.h>

#include "switch_fan_driver.h"
#include "switch_led_driver.h"

#define SWITCH_S3IP_CPLD_VERSION "0.0.0.1"

unsigned int loglevel = 0;

struct fan_attribute{
    struct attribute attr;
    ssize_t (*show)(struct kobject *obj, struct fan_attribute *attr, char *buf);
    ssize_t (*store)(struct kobject *obj, struct fan_attribute *attr, const char *buf, size_t count);
};

struct fan_drivers_t *cb_func = NULL;

/* For s3ip */
extern struct kobject *switch_kobj;
static struct kobject *fan_kobj;
static struct kobject *fan_index_kobj[MAX_FAN_NUM];
static struct kobject *motor_index_kobj[MAX_FAN_NUM][MAX_MOTOR_NUM];

enum fan_attrs {
    DEBUG,
    LOGLEVEL,
    NUM,
    MODEL_NAME,
    SERIAL_NUMBER,
    VENDOR,
    PART_NUMBER,
    HARDWARE_VERSION,
    NUM_MOTORS,
    STATUS,
    LED_STATUS,
    HI_DIRECTION,
    HI_RATIO,
    NUM_FAN_ATTR,
};

enum motor_attrs {
    SPEED,
    SPEED_TOLERANCE,
    SPEED_MAX,
    SPEED_MIN,
    SPEED_TARGET,
    RATIO,
    DIRECTION,
    NUM_MOTOR_ATTR,
};

int get_fan_index(struct kobject *kobj)
{
    int retval=0;
    unsigned int fan_index;
    char fan_index_str[2] = {0};
#ifdef C11_ANNEX_K
    if(memcpy_s(fan_index_str, 2, (kobject_name(kobj)+3), 1) != 0)
    {
        return -ENOMEM;
    }
#else
    memcpy(fan_index_str, (kobject_name(kobj)+3), 1);
#endif

    retval = kstrtoint(fan_index_str, 10, &fan_index);
    if(retval == 0)
    {
        FAN_DEBUG("fan_index:%d \n", fan_index);
    }
    else
    {
        FAN_DEBUG("Error:%d, fan_index:%s \n", retval, fan_index_str);
        return -1;
    }

    return fan_index;
}

int get_fan_index_from_motor_kobj(struct kobject *kobj)
{
    int retval=0;
    unsigned int fan_index;
    char fan_index_str[2] = {0};

#ifdef C11_ANNEX_K
    if(memcpy_s(fan_index_str, 2, (kobject_name(kobj->parent)+3), 1) != 0)
    {
        return -ENOMEM;
    }
#else
    memcpy(fan_index_str, (kobject_name(kobj->parent)+3), 1);
#endif

    retval = kstrtoint(fan_index_str, 10, &fan_index);
    if(retval == 0)
    {
        FAN_DEBUG("fan_index:%d \n", fan_index);
    }
    else
    {
        FAN_DEBUG("Error:%d, fan_index:%s \n", retval, fan_index_str);
        return -1;
    }

    return fan_index;
}

int get_motor_index(struct kobject *kobj)
{
    int retval=0;
    unsigned int motor_index;
    char motor_index_str[2] = {0};

#ifdef C11_ANNEX_K
    if(memcpy_s(motor_index_str, 2, (kobject_name(kobj)+5), 1) != 0)
    {
        return -ENOMEM;
    }
#else
    memcpy(motor_index_str, (kobject_name(kobj)+5), 1);
#endif

    retval = kstrtoint(motor_index_str, 10, &motor_index);
    if(retval == 0)
    {
        FAN_DEBUG("motor_index:%d \n", motor_index);
    }
    else
    {
        FAN_DEBUG("Error:%d, motor_index:%s \n", retval, motor_index_str);
        return -1;
    }

    return motor_index;
}

void set_fan_led(int index, unsigned short val)
{
    return;
}

static ssize_t s3ip_debug_help(struct kobject *kobj, struct fan_attribute *attr, char *buf)
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

static ssize_t s3ip_debug(struct kobject *kobj, struct fan_attribute *attr, const char *buf, size_t count)
{
    return count;
}

static ssize_t s3ip_get_loglevel(struct kobject *kobj, struct fan_attribute *attr, char *buf)
{

#ifdef C11_ANNEX_K
    return sprintf_s(buf, PAGE_SIZE, "%d\n", loglevel);
#else
    return sprintf(buf, "%d\n", loglevel);
#endif
}

static ssize_t s3ip_set_loglevel(struct kobject *kobj, struct fan_attribute *attr, const char *buf, size_t count)
{
    int retval=0;
    long lev;

    if(cb_func == NULL)
        return -1;

    retval = kstrtol(buf, 0, &lev);
    if(retval == 0)
    {
        FAN_DEBUG("lev:%ld \n", lev);
    }
    else
    {
        FAN_DEBUG("Error:%d, lev:%s \n", retval, buf);
        return -1;
    }

    loglevel = (unsigned int)lev;

    cb_func->set_loglevel(lev);

    return count;
}

static ssize_t s3ip_get_num(struct kobject *kobj, struct fan_attribute *attr, char *buf)
{
#ifdef C11_ANNEX_K
    return sprintf_s(buf, PAGE_SIZE, "%d\n", MAX_FAN_NUM);
#else
    return sprintf(buf, "%d\n", MAX_FAN_NUM);
#endif

}

static ssize_t s3ip_get_model_name(struct kobject *kobj, struct fan_attribute *attr, char *buf)
{
    int retval=0;
    int fan_index;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    fan_index = get_fan_index(kobj);
    if(fan_index < 0)
    {
        FAN_DEBUG("Get fan index failed.\n");
        ERROR_RETURN_NA(-1);
    }

    retval = cb_func->get_model_name(fan_index, buf);
    if(retval < 0)
    {
        ERROR_RETURN_NA(retval);
    }
    else
    {
        return retval;
    }
}

static ssize_t s3ip_get_serial_number(struct kobject *kobj, struct fan_attribute *attr, char *buf)
{
    int retval=0;
    int fan_index;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    fan_index = get_fan_index(kobj);
    if(fan_index < 0)
    {
        FAN_DEBUG("Get fan index failed.\n");
        ERROR_RETURN_NA(-1);
    }

    retval = cb_func->get_sn(fan_index, buf);
    if(retval < 0)
    {
        ERROR_RETURN_NA(retval);
    }
    else
    {
        return retval;
    }
}

static ssize_t s3ip_get_vendor(struct kobject *kobj, struct fan_attribute *attr, char *buf)
{
    int retval=0;
    int fan_index;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    fan_index = get_fan_index(kobj);
    if(fan_index < 0)
    {
        FAN_DEBUG("Get fan index failed.\n");
        ERROR_RETURN_NA(-1);
    }

    retval = cb_func->get_vendor(fan_index, buf);
    if(retval < 0)
    {
        ERROR_RETURN_NA(retval);
    }
    else
    {
        return retval;
    }
}

static ssize_t s3ip_get_part_number(struct kobject *kobj, struct fan_attribute *attr, char *buf)
{
    int retval=0;
    int fan_index;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    fan_index = get_fan_index(kobj);
    if(fan_index < 0)
    {
        FAN_DEBUG("Get fan index failed.\n");
        ERROR_RETURN_NA(-1);
    }

    retval = cb_func->get_part_number(fan_index, buf);
    if(retval < 0)
    {
        ERROR_RETURN_NA(retval);
    }
    else
    {
        return retval;
    }
}

static ssize_t s3ip_get_hardware_version(struct kobject *kobj, struct fan_attribute *attr, char *buf)
{
    int retval=0;
    int fan_index;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    fan_index = get_fan_index(kobj);
    if(fan_index < 0)
    {
        FAN_DEBUG("Get fan index failed.\n");
        ERROR_RETURN_NA(-1);
    }

    retval = cb_func->get_hw_version(fan_index, buf);
    if(retval < 0)
    {
        ERROR_RETURN_NA(retval);
    }
    else
    {
        return retval;
    }
}

static ssize_t s3ip_get_num_motors(struct kobject *kobj, struct fan_attribute *attr, char *buf)
{
#ifdef C11_ANNEX_K
    return sprintf_s(buf, PAGE_SIZE, "%d\n", MAX_MOTOR_NUM);
#else
    return sprintf(buf, "%d\n", MAX_MOTOR_NUM);
#endif

}

static ssize_t s3ip_get_status(struct kobject *kobj, struct fan_attribute *attr, char *buf)
{
    int fan_index;
    unsigned int status=0;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    fan_index = get_fan_index(kobj);
    if(fan_index < 0)
    {
        FAN_DEBUG("Get fan index failed.\n");
        ERROR_RETURN_NA(-1);
    }

    status = cb_func->get_status(fan_index);
    if(status < 0)
    {
        ERROR_RETURN_NA(status);
    }
#ifdef C11_ANNEX_K
    return sprintf_s(buf, PAGE_SIZE, "%d\n", status);
#else
    return sprintf(buf, "%d\n", status);
#endif

}

ssize_t s3ip_get_led_status(struct kobject *kobj, struct fan_attribute *attr, char *buf)
{
    int retval=0;
    unsigned int val;
    int fan_index;


    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    fan_index = get_fan_index(kobj);
    if(fan_index < 0)
    {
        FAN_DEBUG("Get fan index failed.\n");
        ERROR_RETURN_NA(-1);
    }

    retval = cb_func->get_led_status(fan_index, &val);
    if(retval < 0)
    {
        FAN_DEBUG("Get fan%d led status failed.\n", fan_index);
        ERROR_RETURN_NA(-1);
    }
#ifdef C11_ANNEX_K
    return sprintf_s(buf, PAGE_SIZE, "%d\n", val);
#else
    return sprintf(buf, "%d\n", val);
#endif

}
EXPORT_SYMBOL_GPL(s3ip_get_led_status);

static ssize_t s3ip_set_led_status(struct kobject *kobj, struct fan_attribute *attr, const char *buf, size_t count)
{
    int retval=0;
    int fan_index;
    int led_status;

    if(cb_func == NULL)
    {
        return -EIO;
    }

    fan_index = get_fan_index(kobj);
    if(fan_index < 0)
    {
        FAN_DEBUG("Get fan index failed.\n");
        return -ENXIO;
    }

    retval = kstrtoint(buf, 10, &led_status);

    if(retval == 0)
    {
        FAN_DEBUG("led_status:%d \n", led_status);
    }
    else
    {
        FAN_DEBUG("Error:%d, led_status:%s \n", retval, buf);
        return -EIO;
    }

    retval = cb_func->set_led_status(fan_index, led_status);
    if(retval != 0)
    {
        FAN_DEBUG("Set fan %d led_status failed.\n", fan_index);
        return -EIO;
    }

    return count;
}

static ssize_t s3ip_get_speed(struct kobject *kobj, struct fan_attribute *attr, char *buf)
{
    int retval=0;
    int fan_index, motor_index;
    unsigned int speed;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    fan_index = get_fan_index_from_motor_kobj(kobj);
    if(fan_index < 0)
    {
        FAN_DEBUG("Get fan index failed.\n");
        ERROR_RETURN_NA(-EINVAL);
    }
    
    motor_index = get_motor_index(kobj);
    if(motor_index < 0)
    {
        FAN_DEBUG("Get motor index failed.\n");
        ERROR_RETURN_NA(-1);
    }

    retval = cb_func->get_speed(fan_index, motor_index-1, &speed);
    if(retval < 0)
    {
        FAN_DEBUG("Get fan%d motor%d speed failed.\n", fan_index, motor_index);
        ERROR_RETURN_NA(-1);
    }
#ifdef C11_ANNEX_K
    return sprintf_s(buf, PAGE_SIZE, "%d\n", speed);
#else
    return sprintf(buf, "%d\n", speed);
#endif

}

static ssize_t s3ip_get_speed_tolerance(struct kobject *kobj, struct fan_attribute *attr, char *buf)
{
    int retval=0;
    int fan_index, motor_index;
    unsigned int speed_tolerance=0;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    fan_index = get_fan_index_from_motor_kobj(kobj);
    if(fan_index < 0)
    {
        FAN_DEBUG("Get fan index failed.\n");
        ERROR_RETURN_NA(-1);
    }

    motor_index = get_motor_index(kobj);
    if(motor_index < 0)
    {
        FAN_DEBUG("Get motor index failed.\n");
        ERROR_RETURN_NA(-1);
    }

    retval = cb_func->get_speed_tolerance(fan_index, motor_index, &speed_tolerance);
    if(retval < 0)
    {
        FAN_DEBUG("Get fan%d motor%d speed tolerance failed.\n", fan_index, motor_index);
        ERROR_RETURN_NA(-1);
    }
#ifdef C11_ANNEX_K
    return sprintf_s(buf, PAGE_SIZE, "%d\n", speed_tolerance);
#else
    return sprintf(buf, "%d\n", speed_tolerance);
#endif

}

static ssize_t s3ip_get_speed_max(struct kobject *kobj, struct fan_attribute *attr, char *buf)
{
    int retval=0;
    int fan_index, motor_index;
    unsigned int speed;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    fan_index = get_fan_index_from_motor_kobj(kobj);
    if(fan_index < 0)
    {
        FAN_DEBUG("Get fan index failed.\n");
        ERROR_RETURN_NA(-EINVAL);
    }
    
    motor_index = get_motor_index(kobj);
    if(motor_index < 0)
    {
        FAN_DEBUG("Get motor index failed.\n");
        ERROR_RETURN_NA(-1);
    }

    retval = cb_func->get_speed_max(fan_index, motor_index, &speed);
    if(retval < 0)
    {
        FAN_DEBUG("Get fan%d motor%d speed failed.\n", fan_index, motor_index);
        ERROR_RETURN_NA(-1);
    }
#ifdef C11_ANNEX_K
    return sprintf_s(buf, PAGE_SIZE, "%d\n", speed);
#else
    return sprintf(buf, "%d\n", speed);
#endif

}

static ssize_t s3ip_get_speed_min(struct kobject *kobj, struct fan_attribute *attr, char *buf)
{
    int retval=0;
    int fan_index, motor_index;
    unsigned int speed;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    fan_index = get_fan_index_from_motor_kobj(kobj);
    if(fan_index < 0)
    {
        FAN_DEBUG("Get fan index failed.\n");
        ERROR_RETURN_NA(-EINVAL);
    }
    
    motor_index = get_motor_index(kobj);
    if(motor_index < 0)
    {
        FAN_DEBUG("Get motor index failed.\n");
        ERROR_RETURN_NA(-1);
    }

    retval = cb_func->get_speed_min(fan_index, motor_index, &speed);
    if(retval < 0)
    {
        FAN_DEBUG("Get fan%d motor%d speed failed.\n", fan_index, motor_index);
        ERROR_RETURN_NA(-1);
    }
#ifdef C11_ANNEX_K
    return sprintf_s(buf, PAGE_SIZE, "%d\n", speed);
#else
    return sprintf(buf, "%d\n", speed);
#endif

}

static ssize_t s3ip_get_speed_target(struct kobject *kobj, struct fan_attribute *attr, char *buf)
{
    int retval=0;
    int fan_index, motor_index;
    unsigned int speed_target;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    fan_index = get_fan_index_from_motor_kobj(kobj);
    if(fan_index < 0)
    {
        FAN_DEBUG("Get fan index failed.\n");
        ERROR_RETURN_NA(-1);
    }

    motor_index = get_motor_index(kobj);
    if(motor_index < 0)
    {
        FAN_DEBUG("Get motor index failed.\n");
        ERROR_RETURN_NA(-1);
    }

    retval = cb_func->get_speed_target(fan_index, motor_index, &speed_target);
    if(retval < 0)
    {
        FAN_DEBUG("Get fan%d motor%d speed target failed.\n", fan_index, motor_index);
        ERROR_RETURN_NA(-1);
    }
#ifdef C11_ANNEX_K
    return sprintf_s(buf, PAGE_SIZE, "%d\n", speed_target);
#else
    return sprintf(buf, "%d\n", speed_target);
#endif

}

static ssize_t s3ip_get_ratio(struct kobject *kobj, struct fan_attribute *attr, char *buf)
{
    int retval=0;
    int fan_index;
    int pwm;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    if(strstr(kobject_name(kobj), "fan")) // /sys/switch/fan/fan*
    {
        fan_index = get_fan_index(kobj);
        if(fan_index < 0)
        {
            FAN_DEBUG("Get fan index failed.\n");
            ERROR_RETURN_NA(-EINVAL);
        }
    }
    else // /sys/switch/fan/fan*/motor/*
    {
        fan_index = get_fan_index_from_motor_kobj(kobj);
        if(fan_index < 0)
        {
            FAN_DEBUG("Get fan index failed.\n");
            ERROR_RETURN_NA(-EINVAL);
        }
    }

    retval = cb_func->get_pwm(fan_index, &pwm);
    if(retval < 0)
    {
        FAN_DEBUG("Get fan %d pwm failed.\n", fan_index);
        ERROR_RETURN_NA(-1);
    }
#ifdef C11_ANNEX_K
    return sprintf_s(buf, PAGE_SIZE, "%d\n", pwm);
#else
    return sprintf(buf, "%d\n", pwm);
#endif

}

static ssize_t s3ip_set_ratio(struct kobject *kobj, struct fan_attribute *attr, const char *buf, size_t count)
{
    int retval=0;
    int fan_index;
    int pwm;

    if(cb_func == NULL)
    {
        return -EIO;
    }

    // /sys/switch/fan/fan*/motor/*
    fan_index = get_fan_index_from_motor_kobj(kobj);
    if(fan_index < 0)
    {
        FAN_DEBUG("Get fan index failed.\n");
        return -ENXIO;
    }

    retval = kstrtoint(buf, 10, &pwm);
    if(retval == 0)
    {
        FAN_DEBUG("pwm:%d \n", pwm);
    }
    else
    {
        FAN_DEBUG("Error:%d, pwm:%s \n", retval, buf);
        return -EIO;
    }

    retval = cb_func->set_pwm(fan_index, pwm);
    if(retval < 0)
    {
        FAN_DEBUG("Get fan %d pwm failed.\n", fan_index);
        return -EIO;
    }

    return count;
}

static ssize_t s3ip_get_direction(struct kobject *kobj, struct fan_attribute *attr, char *buf)
{
    int retval=0;
    int fan_index;
    int direction;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    if(strstr(kobject_name(kobj), "fan")) // /sys/switch/fan/fan*
    {
        fan_index = get_fan_index(kobj);
        if(fan_index < 0)
        {
            FAN_DEBUG("Get fan index failed.\n");
            ERROR_RETURN_NA(-EINVAL);
        }
    }
    else // /sys/switch/fan/fan*/motor/*
    {
        fan_index = get_fan_index_from_motor_kobj(kobj);
        if(fan_index < 0)
        {
            FAN_DEBUG("Get fan index failed.\n");
            ERROR_RETURN_NA(-EINVAL);
        }
    }

    retval = cb_func->get_wind(fan_index, &direction);
    if(retval < 0)
    {
        FAN_DEBUG("Get fan %d direction failed.\n", fan_index);
        ERROR_RETURN_NA(-EINVAL);
    }
#ifdef C11_ANNEX_K
    return sprintf_s(buf, PAGE_SIZE, "%d\n", direction);
#else
    return sprintf(buf, "%d\n", direction);
#endif

}

static struct fan_attribute fan_attr[NUM_FAN_ATTR] = {
    [DEBUG]                     = {{.name = "debug",                    .mode = S_IRUGO | S_IWUSR},     s3ip_debug_help,                     s3ip_debug},
    [LOGLEVEL]                  = {{.name = "loglevel",                 .mode = S_IRUGO | S_IWUSR},     s3ip_get_loglevel,                   s3ip_set_loglevel},
    [NUM]                       = {{.name = "num",                      .mode = S_IRUGO},               s3ip_get_num,                        NULL},
    [MODEL_NAME]                = {{.name = "model_name",               .mode = S_IRUGO},               s3ip_get_model_name,                 NULL},
    [SERIAL_NUMBER]             = {{.name = "serial_number",            .mode = S_IRUGO},               s3ip_get_serial_number,              NULL},
    [VENDOR]                    = {{.name = "vendor",                   .mode = S_IRUGO},               s3ip_get_vendor,                     NULL},
    [PART_NUMBER]               = {{.name = "part_number",              .mode = S_IRUGO},               s3ip_get_part_number,                NULL},
    [HARDWARE_VERSION]          = {{.name = "hardware_version",         .mode = S_IRUGO},               s3ip_get_hardware_version,           NULL},
    [NUM_MOTORS]                = {{.name = "num_motors",               .mode = S_IRUGO},               s3ip_get_num_motors,                 NULL},
    [STATUS]                    = {{.name = "status",                   .mode = S_IRUGO},               s3ip_get_status,                     NULL},
    [LED_STATUS]                = {{.name = "led_status",               .mode = S_IRUGO | S_IWUSR},     s3ip_get_led_status,                 s3ip_set_led_status},
    [HI_DIRECTION]              = {{.name = "direction",                .mode = S_IRUGO},               s3ip_get_direction,                  NULL},
    [HI_RATIO]                  = {{.name = "ratio",                    .mode = S_IRUGO | S_IWUSR},     s3ip_get_ratio,                      s3ip_set_ratio},
};  
    
static struct fan_attribute motor_attr[NUM_MOTOR_ATTR] = {  
    [SPEED]                     = {{.name = "speed",                    .mode = S_IRUGO},               s3ip_get_speed,                      NULL},
    [SPEED_TOLERANCE]           = {{.name = "speed_tolerance",          .mode = S_IRUGO},               s3ip_get_speed_tolerance,            NULL},
    [SPEED_MAX]                 = {{.name = "speed_max",                .mode = S_IRUGO},               s3ip_get_speed_max,                  NULL},
    [SPEED_MIN]                 = {{.name = "speed_min",                .mode = S_IRUGO},               s3ip_get_speed_min,                  NULL},
    [SPEED_TARGET]              = {{.name = "speed_target",             .mode = S_IRUGO},               s3ip_get_speed_target,               NULL},
    [RATIO]                     = {{.name = "ratio",                    .mode = S_IRUGO | S_IWUSR},     s3ip_get_ratio,                      s3ip_set_ratio},
    [DIRECTION]                 = {{.name = "direction",                .mode = S_IRUGO},               s3ip_get_direction,                  NULL},
};

void s3ip_fan_drivers_register(struct fan_drivers_t *pfunc)
{
    cb_func = pfunc;

    return;
}
EXPORT_SYMBOL_GPL(s3ip_fan_drivers_register);

void s3ip_fan_drivers_unregister(void)
{
    cb_func = NULL;

    return;
}
EXPORT_SYMBOL_GPL(s3ip_fan_drivers_unregister);

static int __init switch_fan_init(void)
{
    int err=0;
    int retval=0;
    int i;
    int fan_index, motor_index;
    char *fan_index_str;
    char *motor_index_str;

    fan_index_str = (char *)kzalloc(6*sizeof(char), GFP_KERNEL);
    if (!fan_index_str)
    {
        FAN_DEBUG( "Fail to alloc fan_index_str memory\n");
        return -ENOMEM;
    }

    motor_index_str = (char *)kzalloc(8*sizeof(char), GFP_KERNEL);
    if (!motor_index_str)
    {
        FAN_DEBUG( "Fail to alloc motor_index_str memory\n");
        kfree(motor_index_str);
        return -ENOMEM;
    }

    /* For s3ip */
    fan_kobj = kobject_create_and_add("fan", switch_kobj);
    if(!fan_kobj)
    {
        FAN_DEBUG( "Failed to create 'fan'\n");
        err = -ENOMEM;
        goto sysfs_create_kobject_fan_failed;
    }

    for(i=0; i<=NUM; i++)
    {
        FAN_DEBUG( "sysfs_create_file /fan/%s\n", fan_attr[i].attr.name);
        retval = sysfs_create_file(fan_kobj, &fan_attr[i].attr);
        if(retval)
        {
            FAN_DEBUG( "Failed to create file '%s'\n", fan_attr[i].attr.name);
            err = -retval;
            goto sysfs_create_s3ip_fan_attr_failed;
        }
    }

    for(fan_index=0; fan_index<MAX_FAN_NUM; fan_index++)
    {
#ifdef C11_ANNEX_K
        if(sprintf_s(fan_index_str, 6, "fan%d", fan_index+1) < 0)
#else
        if(sprintf(fan_index_str, "fan%d", fan_index+1) < 0)
#endif

        {
             FAN_DEBUG( "Failed to sprintf 'fan%d'\n", fan_index+1);
             err = -ENOMEM;
             goto sysfs_create_kobject_fan_index_failed;
        }
        fan_index_kobj[fan_index] = kobject_create_and_add(fan_index_str, fan_kobj);
        if(!fan_index_kobj[fan_index])
        {
            FAN_DEBUG( "Failed to create 'fan%d'\n", fan_index+1);
            err = -ENOMEM;
            goto sysfs_create_kobject_fan_index_failed;
        }

        for(i=MODEL_NAME; i<NUM_FAN_ATTR; i++)
        {
            FAN_DEBUG( "sysfs_create_file /switch/fan/fan%d/%s\n", fan_index+1, fan_attr[i].attr.name);       
            
            retval = sysfs_create_file(fan_index_kobj[fan_index], &fan_attr[i].attr);
            if(retval)
            {
                printk(KERN_ERR "Failed to create file '%s'\n", fan_attr[i].attr.name);
                err = -retval;
                goto sysfs_create_s3ip_fan_attrs_failed;
            }
        }

        for(motor_index=0; motor_index<MAX_MOTOR_NUM; motor_index++)
        {
#ifdef C11_ANNEX_K
            if(sprintf_s(motor_index_str, 8, "motor%d", motor_index+1) < 0)
#else
            if(sprintf(motor_index_str, "motor%d", motor_index+1) < 0)
#endif

            {
                err = -ENOMEM;
                goto sysfs_create_kobject_switch_motor_index_failed;
            }
            motor_index_kobj[fan_index][motor_index] = kobject_create_and_add(motor_index_str, fan_index_kobj[fan_index]);
            if(!motor_index_kobj[fan_index][motor_index])
            {
                FAN_DEBUG( "Failed to create 'motor%d'\n", motor_index+1);
                err = -ENOMEM;
                goto sysfs_create_kobject_switch_motor_index_failed;
            }
            
            for(i=SPEED; i<NUM_MOTOR_ATTR; i++)
            {
                FAN_DEBUG( "sysfs_create_file /switch/fan/fan%d/%s/%s\n", fan_index+1, motor_index_str, motor_attr[i].attr.name);

                retval = sysfs_create_file(motor_index_kobj[fan_index][motor_index], &motor_attr[i].attr);
                if(retval)
                {
                    printk(KERN_ERR "Failed to create file '%s'\n", motor_attr[i].attr.name);
                    err = -retval;
                    goto sysfs_create_s3ip_motor_attrs_failed;
                }
            }
        }
    }

    kfree(fan_index_str);
    kfree(motor_index_str);

    return 0;

sysfs_create_s3ip_motor_attrs_failed:
sysfs_create_kobject_switch_motor_index_failed:
sysfs_create_s3ip_fan_attrs_failed:
sysfs_create_kobject_fan_index_failed:
    for(i=0; i<=NUM; i++)
        sysfs_remove_file(fan_kobj, &fan_attr[i].attr);

    for(fan_index=0; fan_index<MAX_FAN_NUM; fan_index++)
    {
        if(fan_index_kobj[fan_index])
        {
            for(i=MODEL_NAME; i < NUM_FAN_ATTR; i++)
                sysfs_remove_file(fan_index_kobj[fan_index], &fan_attr[i].attr);
        
            for(motor_index=0; motor_index<MAX_MOTOR_NUM; motor_index++)
            {
                if(motor_index_kobj[fan_index][motor_index])
                {
                    for(i=SPEED; i < NUM_MOTOR_ATTR; i++)
                        sysfs_remove_file(motor_index_kobj[fan_index][motor_index], &motor_attr[i].attr);
                }
                
                kobject_put(motor_index_kobj[fan_index][motor_index]);
                motor_index_kobj[fan_index][motor_index] = NULL;
            }
        }

        kobject_put(fan_index_kobj[fan_index]);
        fan_index_kobj[fan_index] = NULL;
    }

sysfs_create_s3ip_fan_attr_failed:
    if(fan_kobj)
    {
        kobject_put(fan_kobj);
        fan_kobj = NULL;
    }

sysfs_create_kobject_fan_failed:
    kfree(fan_index_str);
    kfree(motor_index_str);

    return err;
}

static void __exit switch_fan_exit(void)
{
    int i;
    int fan_index;
    int motor_index;

    /* For s3ip */
    for(i=0; i<=NUM; i++)
        sysfs_remove_file(fan_kobj, &fan_attr[i].attr);

    for(fan_index=0; fan_index<MAX_FAN_NUM; fan_index++)
    {
        if(fan_index_kobj[fan_index])
        {
            for(i=MODEL_NAME; i < NUM_FAN_ATTR; i++)
                sysfs_remove_file(fan_index_kobj[fan_index], &fan_attr[i].attr);

            for(motor_index=0; motor_index<MAX_MOTOR_NUM; motor_index++)
            {
                if(motor_index_kobj[fan_index][motor_index])
                {
                    for(i=SPEED; i < NUM_MOTOR_ATTR; i++)
                        sysfs_remove_file(motor_index_kobj[fan_index][motor_index], &motor_attr[i].attr);
                    
                    kobject_put(motor_index_kobj[fan_index][motor_index]);
                    motor_index_kobj[fan_index][motor_index] = NULL;
                }                           
            }

            kobject_put(fan_index_kobj[fan_index]);
            fan_index_kobj[fan_index] = NULL;
        }
    }

    if(fan_kobj)
    {
        kobject_put(fan_kobj);
        fan_kobj = NULL;
    }

    cb_func = NULL;

    return;
}

MODULE_DESCRIPTION("Switch S3IP Fan Driver");
MODULE_VERSION(SWITCH_S3IP_CPLD_VERSION);
MODULE_LICENSE("GPL");

module_init(switch_fan_init);
module_exit(switch_fan_exit);