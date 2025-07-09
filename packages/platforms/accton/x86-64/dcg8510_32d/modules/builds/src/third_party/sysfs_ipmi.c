#include <linux/module.h>
#include <linux/fs.h>
#include <uapi/linux/ipmi.h>
#include <linux/poll.h>
#include <linux/ipmi.h>
#include <linux/ipmi_smi.h>
#include <linux/semaphore.h>

#include "switch_psu_driver.h"
#include "switch_sensor_driver.h"
#include "switch_cpld_driver.h"

/* ott_ipmi_comm.h */
#define OTT_IPMI_XFER_DEFAULT_TIMEOUT 5
unsigned int loglevel_bmc = 0;
EXPORT_SYMBOL_GPL(loglevel_bmc);

#define IPMI_INTF_NUM (0)
static struct ipmi_user *g_ipmi_mh_user = NULL;
static struct semaphore g_ipmi_msg_sem;
static struct ipmi_recv_msg g_bmc_recv_msg;
typedef struct {
    long timeout; /* 单位是秒，ott_ipmi_xfer对小于或等于0的值，会用OTT_IPMI_XFER_DEFAULT_TIMEOUT，所以可以初始化成0 */
    struct kernel_ipmi_msg req;
    struct kernel_ipmi_msg rsp;
} ott_ipmi_xfer_info_s;


typedef struct {
    struct wait_queue_head * wait_address;
    poll_table pt;
} ott_ipmi_poll_info_s;

typedef struct {
    unsigned char ccode; /* IPMI completion code */
    unsigned int value;
} bmc_fan_speed_get_s;

enum ipmi_mfr_info_type
{
    ACK_DATA_RESP_CHAR      = 1,
    ACK_DATA_RESP_INT       = 2,
    ACK_DATA_RESP_FLOAT     = 3,
    ACK_DATA_RESP_FLOAT_DIV = 4,
    ACK_DATA_RESP_PRESENT   = 5,
    ACK_DATA_RESP_BITMAP    = 6,
};

enum ipmi_app_power_attribute_type
{
    POWER_GOOD           = 0x02,
    POWER_STATUS         = 0x03,
    POWER_CURRENT_LIMIT  = 0x04,
    POWER_TEMP_LIMIT     = 0x05,
    POWER_INPUT_VOLTAGE  = 0x0E,
    POWER_OUTPUT_VOLTAGE = 0x0F,
    POWER_OUTPUT_CURRENT = 0x10,
    POWER_TEMPERATURE    = 0x11,
    POWER_FAN_SPEED      = 0x12,
    POWER_OUTPUT_POWER   = 0x13,
    POWER_INPUT_POWER    = 0x14,
    POWER_HARDWARE_VER   = 0x21,
    POWER_INPUT_CURRENT  = 0x23,
    POWER_TEMPERATURE_1  = 0x24,
    POWER_TEMPERATURE_2  = 0x25,
    POWER_TEMPERATURE_3  = 0x26,
    POWER_POUT_MAX       = 0x27,

};

enum ipmi_app_power_alarm_type
{
    POWER_ALARM_CMD            = 0x1,
    POWER_ALARM_TEMPERATURE    = 0x2,
    POWER_ALARM_VIN_UV_FAULT   = 0x3,
    POWER_ALARM_IOUT_OC_FAULT  = 0x4,
    POWER_ALARM_VOUT_OV_FAULT  = 0x5,
    POWER_ALARM_VIN_OV_DETECT  = 0x6,
    POWER_ALARM_VIN_UNIT_OFF   = 0x7,
    POWER_ALARM_OTHER          = 0x9,
    POWER_ALARM_FAN            = 0xA,
    POWER_ALARM_INPUT          = 0xD,
    POWER_ALARM_IOUT_POUT      = 0xE,
    POWER_ALARM_VOUT           = 0xF,
};

enum cpld_access_active
{
    CPLD_READ  = 0x00,
    CPLD_WRITE = 0x01,
};

enum fan_eeprom_field
{
    fan_model_name       = 0x00,
    fan_serial_number    = 0x01,
    fan_vendor           = 0x02,
    fan_part_number      = 0x03,
    fan_hardware_version = 0x04,
    fan_direction        = 0x05,
};

enum eeprom_read_type
{
    eeprom_part_number      = 0x01,
    eeprom_serial_number    = 0x02,
    eeprom_vendor           = 0x03,
    eeprom_model_name       = 0x04,
    eeprom_hardware_version = 0x05,
    eeprom_date             = 0x06,
};

enum Bits
{
    BIT0    = 0x0001,
    BIT1    = 0x0002,
    BIT2    = 0x0004,
    BIT3    = 0x0008,
    BIT4    = 0x0010,
    BIT5    = 0x0020,
    BIT6    = 0x0040,
    BIT7    = 0x0080,
    BIT8    = 0x0100,
    BIT9    = 0x0200,
    BIT10   = 0x0400,
    BIT11   = 0x0800,
    BIT12   = 0x1000,
    BIT13   = 0x2000,
    BIT14   = 0x4000,
    BIT15   = 0x8000,
};

static DEFINE_MUTEX(ott_ipmi_mutex);

#define IPMI_NETFN                   0x36
#define IPMI_GET_THERMAL_DATA        0x10
#define IPMI_GET_FAN_SPEED           0x20
#define IPMI_GET_FAN_PWM             0x21
#define IPMI_SET_FAN_PWM             0x22
#define IPMI_FAN_GET_EEPROM          0x23
#define IPMI_FAN_GET_LED_STATUS      0x25
#define IPMI_FAN_SET_LED_STATUS      0x26
#define IPMI_GET_PSU_ALARM_DATA      0x18
#define IPMI_GET_PSU_ATTRIBUTE_DATA  0x30
#define IPMI_GET_PSU_E_LABEL_DATA    0x1C
#define IPMI_PSU_GET_EEPROM          0x31
#define IPMI_GET_SENSOR_DATA         0x40
#define IPMI_CPLD_REG_ACCESS         0x60
#define IPMI_CMD_GET_FMEA_STATUS     0xA6

#define IPMI_PSU_PRESENT             1
#define IPMI_PSU_ABSENT              2

#define FAN_MODEL_NAME_LEN      2+1
#define FAN_SERIAL_NUM_LEN      10+1
#define FAN_VENDOR_LEN          6+1
#define FAN_PART_NUM_LEN        13+1
#define FAN_HARDWARE_VER_LEN    3+1
#define FAN_MAX_LEN       32+1

#define PMBUS_MFR_MODEL_LEN     21+1
#define PMBUS_MFR_SERIAL_LEN    18+1
#define PMBUS_MFR_DATE_LEN      10+1
#define PMBUS_MFR_ID_LEN        3+1
#define PMBUS_MFR_REVISION_LEN  3+1
#define PMBUS_PART_NUM_LEN      8+1
#define PMBUS_MFR_MAX_LEN       32+1

#define POWER_ALARM_CMD         1

#define IPMI_REQ_RETRY_TIME_MS 0
#define IPMI_REQ_MAX_RETRIES -1
#define IPMI_REQ_PRIORITY 1

volatile static unsigned char g_req_netfn = 0;
volatile static unsigned char g_req_cmd = 0;

static void msg_handler(struct ipmi_recv_msg *msg, void *handler_data)
{
    if (((g_req_netfn + 1) != msg->msg.netfn) || (g_req_cmd != msg->msg.cmd))
    {
        ipmi_free_recv_msg(msg);
        return;
    }
    g_bmc_recv_msg = *msg;
    g_bmc_recv_msg.msg.data = g_bmc_recv_msg.msg_data;
    ipmi_free_recv_msg(msg);
    up(&g_ipmi_msg_sem);
    return;
}

static struct ipmi_user_hndl ipmi_hndlrs = {
    .ipmi_recv_hndl = msg_handler,
};

int ott_ipmi_xfer(ott_ipmi_xfer_info_s * para)
{
    int ret;
    int rsp_len = 0;
    int timeout = 0;
    struct ipmi_system_interface_addr addr;
    struct kernel_ipmi_msg msg =
    {
        .netfn= para->req.netfn,
        .cmd = para->req.cmd,
        .data_len = para->req.data_len,
        .data = para->req.data
    };

    addr.addr_type = IPMI_SYSTEM_INTERFACE_ADDR_TYPE;
    addr.channel = IPMI_BMC_CHANNEL;
    addr.lun = 0;
    mutex_lock(&ott_ipmi_mutex);
    ret = ipmi_request_settime(g_ipmi_mh_user, (struct ipmi_addr *)&addr, 0, &msg, NULL, IPMI_REQ_PRIORITY, IPMI_REQ_MAX_RETRIES, IPMI_REQ_RETRY_TIME_MS);
    if (ret)
    {
        printk("warning: send request fail, led broken\n");
        mutex_unlock(&ott_ipmi_mutex);
        return ret;
    }
    rsp_len = para->rsp.data_len;// Set length at call ex:ipmi_info.rsp.data_len = 2;
    timeout = (timeout <= 0) ? OTT_IPMI_XFER_DEFAULT_TIMEOUT : timeout;
    g_req_netfn = msg.netfn;
    g_req_cmd = msg.cmd;
    ret = down_timeout(&g_ipmi_msg_sem, timeout*HZ);//wait for recv_msg
    if(ret)
    {
        printk("down_interruptible error timeout ret = %d\n", ret);
        mutex_unlock(&ott_ipmi_mutex);
        return ret;
    }
    para->rsp.netfn = g_bmc_recv_msg.msg.netfn;
    para->rsp.cmd = g_bmc_recv_msg.msg.cmd;
    para->rsp.data_len = g_bmc_recv_msg.msg.data_len;
    if (para->rsp.data_len > rsp_len )
    {
        printk("%s %d,response date error,para->rsp.data_len = %d, rsp_len:%d.\n", __func__,__LINE__,para->rsp.data_len, rsp_len);
        mutex_unlock(&ott_ipmi_mutex);
        return -1;
    }

#ifdef C11_ANNEX_K
    ret = memcpy_s(para->rsp.data, rsp_len, g_bmc_recv_msg.msg.data, para->rsp.data_len);
#else
    memcpy(para->rsp.data, g_bmc_recv_msg.msg.data, para->rsp.data_len);
#endif
    if(ret)
    {
        printk("memcpy g_bmc_recv_msg.msg.data to para->rsp.data error\n");
        printk("req:para->req.netfn = %d, para->req.cmd = %d, para->req.data_len = %d, para->req.data = %d\n", para->req.netfn, para->req.cmd, para->req.data_len, para->req.data[0]);
        printk("rsp:para->rsp.netfn = %d, para->rsp.cmd = %d, para->rsp.data_len = %d, para->rsp.data = %d\n", para->rsp.netfn, para->rsp.cmd, para->rsp.data_len, para->rsp.data[0]);
        mutex_unlock(&ott_ipmi_mutex);
        return ret;
    }
    mutex_unlock(&ott_ipmi_mutex);
    return ret;
}

int drv_get_sensor_temp_input_from_bmc(unsigned int index, long *value)
{
    int ipmi_ret;
    char value_get = 0;
    ott_ipmi_xfer_info_s ipmi_info = {0};
    unsigned char data[1] = {1};
    unsigned char ack_data[2];
    unsigned int loglevel = loglevel_bmc;

    data[0] = (unsigned char)(index);
#ifdef C11_ANNEX_K
    memset_s(ack_data, 2, 0, 2);
#else
    memset(ack_data, 0, 2);
#endif

    ipmi_info.req.netfn = IPMI_NETFN;
    ipmi_info.req.cmd = IPMI_GET_THERMAL_DATA;
    ipmi_info.req.data = data;
    ipmi_info.req.data_len = sizeof(data);

    ipmi_info.rsp.data = ack_data;
    ipmi_info.rsp.data_len = 2;
    ipmi_ret = ott_ipmi_xfer(&ipmi_info);
    if(ipmi_ret != 0)
    {
        SENSOR_DEBUG("%d, get temp info fail! ipmi_ret:0x%x,index:%d.\n",
            __LINE__,ipmi_ret,index);
        return (unsigned int)ipmi_ret;
    }

    if(ack_data[0] != 0)
    {
        SENSOR_DEBUG("%d, get temp info fail! ipmi_ret:0x%x,ack_data[0]:0x%x,index:%d.\n",
            __LINE__,ipmi_ret,ack_data[0],index);
        return -EIO;
    }

    if((ipmi_info.rsp.netfn != (ipmi_info.req.netfn + 1)) || (ipmi_info.rsp.cmd != ipmi_info.req.cmd))
    {
        SENSOR_DEBUG("%d, get temp info fail! ipmi_ret:0x%x,ack_data[0]:0x%x,index:%d.\n",
            __LINE__,ipmi_ret,ack_data[0],index);
        return -EIO;
    }
    value_get = (int)(ack_data[1]);
    *value = value_get * 1000;
    return 0;
}
EXPORT_SYMBOL_GPL(drv_get_sensor_temp_input_from_bmc);

int drv_get_sensor_vol_input_from_bmc(vr_sensor_node_t node, long *value)
{
    int ipmi_ret;
    int value_get = 0;
    ott_ipmi_xfer_info_s ipmi_info = {0};
    unsigned char data[3] = {1};
    unsigned char ack_data[5];
    unsigned int loglevel = loglevel_bmc;

    data[0] = node.chip_id;
    data[1] = node.channel;
    data[2] = node.attr_type;
#ifdef C11_ANNEX_K
    memset_s(ack_data, 5, 0, 5);
#else
    memset(ack_data, 0, 5);
#endif
    ipmi_info.req.netfn = IPMI_NETFN;
    ipmi_info.req.cmd = IPMI_GET_SENSOR_DATA;
    ipmi_info.req.data = data;
    ipmi_info.req.data_len = sizeof(data);

    ipmi_info.rsp.data = ack_data;
    ipmi_info.rsp.data_len = 5;
    ipmi_ret = ott_ipmi_xfer(&ipmi_info);
    if(ipmi_ret != 0)
    {
        SENSOR_DEBUG("%d, get vol info fail! ipmi_ret:0x%x,chip_id:%d,channel:%d,attr_type:%d.\n",
            __LINE__,ipmi_ret,node.chip_id,node.channel,node.attr_type);
        return (unsigned int)ipmi_ret;
    }

    if(ack_data[0] != 0)
    {
        SENSOR_DEBUG("%d, get vol info fail! ipmi_ret:0x%x,ack_data[0]:0x%x,chip_id:%d,channel:%d,attr_type:%d.\n",
            __LINE__,ipmi_ret,ack_data[0],node.chip_id,node.channel,node.attr_type);
        return -EIO;
    }

    if((ipmi_info.rsp.netfn != (ipmi_info.req.netfn + 1)) || (ipmi_info.rsp.cmd != ipmi_info.req.cmd))
    {
        SENSOR_DEBUG("%d, get vol info fail! ipmi_ret:0x%x,ack_data[0]:0x%x,chip_id:%d,channel:%d,attr_type:%d.\n",
            __LINE__,ipmi_ret,ack_data[0],node.chip_id,node.channel,node.attr_type);
        return -EIO;
    }
    value_get = (((int)(ack_data[1])) << 24) + (((int)(ack_data[2])) << 16) + (((int)(ack_data[3])) << 8) + (int)(ack_data[4]);
    *value = value_get;
    return 0;
}
EXPORT_SYMBOL_GPL(drv_get_sensor_vol_input_from_bmc);

int drv_get_sensor_curr_input_from_bmc(vr_sensor_node_t node, long *value)
{
    int ipmi_ret;
    int value_get = 0;
    ott_ipmi_xfer_info_s ipmi_info = {0};
    unsigned char data[3] = {1};
    unsigned char ack_data[5];
    unsigned int loglevel = loglevel_bmc;

    data[0] = node.chip_id;
    data[1] = node.channel;
    data[2] = node.attr_type;
#ifdef C11_ANNEX_K
    memset_s(ack_data, 5, 0, 5);
#else
    memset(ack_data, 0, 5);
#endif
    ipmi_info.req.netfn = IPMI_NETFN;
    ipmi_info.req.cmd = IPMI_GET_SENSOR_DATA;
    ipmi_info.req.data = data;
    ipmi_info.req.data_len = sizeof(data);

    ipmi_info.rsp.data = ack_data;
    ipmi_info.rsp.data_len = 5;
    ipmi_ret = ott_ipmi_xfer(&ipmi_info);
    if(ipmi_ret != 0)
    {
        SENSOR_DEBUG("%d, get curr info fail! ipmi_ret:0x%x,chip_id:%d,channel:%d,attr_type:%d.\n",
            __LINE__,ipmi_ret,node.chip_id,node.channel,node.attr_type);
        return (unsigned int)ipmi_ret;
    }

    if(ack_data[0] != 0)
    {
        SENSOR_DEBUG("%d, get curr info fail! ipmi_ret:0x%x,ack_data[0]:0x%x,chip_id:%d,channel:%d,attr_type:%d.\n",
            __LINE__,ipmi_ret,ack_data[0],node.chip_id,node.channel,node.attr_type);
        return -EIO;
    }

    if((ipmi_info.rsp.netfn != (ipmi_info.req.netfn + 1)) || (ipmi_info.rsp.cmd != ipmi_info.req.cmd))
    {
        SENSOR_DEBUG("%d, get temp info fail! ipmi_ret:0x%x,ack_data[0]:0x%x,chip_id:%d,channel:%d,attr_type:%d.\n",
            __LINE__,ipmi_ret,ack_data[0],node.chip_id,node.channel,node.attr_type);
        return -EIO;
    }
    value_get = (((int)(ack_data[1])) << 24) + (((int)(ack_data[2])) << 16) + (((int)(ack_data[3])) << 8) + (int)(ack_data[4]);
    *value = value_get;
    return 0;
}
EXPORT_SYMBOL_GPL(drv_get_sensor_curr_input_from_bmc);

ssize_t drv_get_wind_from_bmc(unsigned int fan_index, unsigned int *wind)
{
    int ipmi_ret;
    ott_ipmi_xfer_info_s ipmi_info = {0};
    unsigned char data[2];
    unsigned char ack_data[2];
    unsigned int loglevel = loglevel_bmc;
    data[0]=(unsigned char)(fan_index);
    data[1]= fan_direction;
#ifdef C11_ANNEX_K
    memset_s(ack_data,2,0,2);
#else
    memset(ack_data,0,2);
#endif
    ipmi_info.req.netfn = IPMI_NETFN;
    ipmi_info.req.cmd = IPMI_FAN_GET_EEPROM;
    ipmi_info.req.data = data;
    ipmi_info.req.data_len = sizeof(data);

    ipmi_info.rsp.data = ack_data;
    ipmi_info.rsp.data_len = 2;
    ipmi_ret = ott_ipmi_xfer(&ipmi_info);

    if(ipmi_ret != 0)
    {
        SENSOR_DEBUG("%d, get fan speed fail! ipmi_ret:0x%x,fan_index:%d.\n", __LINE__,ipmi_ret,fan_index);
        return -1;
    }

    if(ack_data[0] != 0)
    {
        SENSOR_DEBUG("%d, get fan speed fail! ipmi_ret:0x%x,ack_data[0]:0x%x,fan_index:%d.\n", __LINE__,ipmi_ret,ack_data[0],fan_index);
        return -1;
    }

    if((ipmi_info.rsp.netfn != (ipmi_info.req.netfn + 1)) || (ipmi_info.rsp.cmd != ipmi_info.req.cmd))
    {
        SENSOR_DEBUG("%d, get fan speed fail! ipmi_ret:0x%x,ack_data[0]:0x%x,fan_index:%d.\n", __LINE__,ipmi_ret,ack_data[0],fan_index);
        return -1;
    }

    *wind = (unsigned int)(ack_data[1]);
    if(*wind > 1)
    {
        return -1;
    }

    return 0;
}
EXPORT_SYMBOL_GPL(drv_get_wind_from_bmc);

ssize_t drv_get_led_status_from_bmc(unsigned int fan_index, unsigned int *led)
{
    int ipmi_ret;
    ott_ipmi_xfer_info_s ipmi_info = {0};
    unsigned char data;
    unsigned char ack_data[2];
    unsigned int loglevel = loglevel_bmc;

    data = (unsigned char)fan_index;
#ifdef C11_ANNEX_K
    memset_s(ack_data, 2, 0, 2);
#else
    memset(ack_data, 0, 2);
#endif
    ipmi_info.req.netfn = IPMI_NETFN;
    ipmi_info.req.cmd = IPMI_FAN_GET_LED_STATUS;
    ipmi_info.req.data = &data;
    ipmi_info.req.data_len = sizeof(data);

    ipmi_info.rsp.data = ack_data;
    ipmi_info.rsp.data_len = sizeof(ack_data);
    ipmi_ret = ott_ipmi_xfer(&ipmi_info);

    if(ipmi_ret != 0)
    {
        SENSOR_DEBUG("%d, get fan part_number fail! ipmi_ret:0x%x,index:%d.\n",
            __LINE__,ipmi_ret,fan_index);
        return (unsigned int)ipmi_ret;
    }

    if(ack_data[0] != 0)
    {
        SENSOR_DEBUG("%d, get fan part_number fail! ipmi_ret:0x%x,ack_data[0]:0x%x,index:%d.\n",
            __LINE__,ipmi_ret,ack_data[0],fan_index);
        return -EIO;
    }

    if((ipmi_info.rsp.netfn != (ipmi_info.req.netfn + 1)) || (ipmi_info.rsp.cmd != ipmi_info.req.cmd))
    {
        SENSOR_DEBUG("%d, get fan part_number fail! ipmi_ret:0x%x,ack_data[0]:0x%x,index:%d,netfn:0x%x,cmd:0x%x.\n",
            __LINE__,ipmi_ret,ack_data[0],fan_index,ipmi_info.rsp.netfn,ipmi_info.req.cmd);
        return -EIO;
    }

    *led = (unsigned int)(ack_data[1]);

    return 0;
}
EXPORT_SYMBOL_GPL(drv_get_led_status_from_bmc);

ssize_t drv_set_led_status_from_bmc(unsigned int fan_id, unsigned int led)
{
    int ipmi_ret;
    ott_ipmi_xfer_info_s ipmi_info = {0};
    unsigned char data[2] = {1};
    unsigned char ack_data[2];
    unsigned int loglevel = loglevel_bmc;
    data[0]=(unsigned char)(fan_id);
    data[1]=(unsigned char)(led);
#ifdef C11_ANNEX_K
    memset_s(ack_data,2,0,2);
#else
    memset(ack_data,0,2);
#endif
    ipmi_info.req.netfn = IPMI_NETFN;
    ipmi_info.req.cmd = IPMI_FAN_SET_LED_STATUS;
    ipmi_info.req.data = data;
    ipmi_info.req.data_len = sizeof(data);

    ipmi_info.rsp.data = ack_data;
    ipmi_info.rsp.data_len = 2;
    ipmi_ret = ott_ipmi_xfer(&ipmi_info);

    if(ipmi_ret != 0)
    {
        SENSOR_DEBUG("%d, set fan led fail! ipmi_ret:0x%x,fan_id:%d.\n",
            __LINE__,ipmi_ret,fan_id);
        return (unsigned int)ipmi_ret;
    }

    if(ack_data[0] != 0)
    {
        SENSOR_DEBUG("%d, set fan led fail! ipmi_ret:0x%x,ack_data[0]:0x%x,fan_id:%d.\n",
            __LINE__,ipmi_ret,ack_data[0],fan_id);
        return -EIO;
    }

    if((ipmi_info.rsp.netfn != (ipmi_info.req.netfn + 1)) || (ipmi_info.rsp.cmd != ipmi_info.req.cmd))
    {
        SENSOR_DEBUG("%d, set fan led fail! ipmi_ret:0x%x,ack_data[0]:0x%x,fan_id:%d.\n",
            __LINE__,ipmi_ret,ack_data[0],fan_id);
        return -EIO;
    }

    return 0;
}
EXPORT_SYMBOL_GPL(drv_set_led_status_from_bmc);

ssize_t drv_get_hw_version_from_bmc(unsigned int fan_index, char *buf)
{
    int ipmi_ret;
    ott_ipmi_xfer_info_s ipmi_info = {0};

    unsigned char data[2];
    unsigned char ack_data[FAN_MAX_LEN];
    unsigned char hardware_version[FAN_HARDWARE_VER_LEN];
    unsigned int loglevel = loglevel_bmc;

    data[0] = (unsigned char)fan_index;
    data[1] = fan_hardware_version;
#ifdef C11_ANNEX_K
    memset_s(ack_data, FAN_MAX_LEN, 0, FAN_MAX_LEN);
#else
    memset(ack_data, 0, FAN_MAX_LEN);
#endif
    ipmi_info.req.netfn = IPMI_NETFN;
    ipmi_info.req.cmd = IPMI_FAN_GET_EEPROM;
    ipmi_info.req.data = data;
    ipmi_info.req.data_len = sizeof(data);

    ipmi_info.rsp.data = ack_data;
    ipmi_info.rsp.data_len = FAN_MAX_LEN;
    ipmi_ret = ott_ipmi_xfer(&ipmi_info);


    if(ipmi_ret != 0)
    {
        SENSOR_DEBUG("%d, get fan part_number fail! ipmi_ret:0x%x,index:%d.\n",
            __LINE__,ipmi_ret,fan_index);
        return (unsigned int)ipmi_ret;
    }

    if(ack_data[0] != 0)
    {
        SENSOR_DEBUG("%d, get fan part_number fail! ipmi_ret:0x%x,ack_data[0]:0x%x,index:%d.\n",
            __LINE__,ipmi_ret,ack_data[0],fan_index);
        return -EIO;
    }

    if((ipmi_info.rsp.netfn != (ipmi_info.req.netfn + 1)) || (ipmi_info.rsp.cmd != ipmi_info.req.cmd))
    {
        SENSOR_DEBUG("%d, get fan part_number fail! ipmi_ret:0x%x,ack_data[0]:0x%x,index:%d,netfn:0x%x,cmd:0x%x.\n",
            __LINE__,ipmi_ret,ack_data[0],fan_index,ipmi_info.rsp.netfn,ipmi_info.req.cmd);
        return -EIO;
    }
#ifdef C11_ANNEX_K
    if(memcpy_s(hardware_version, FAN_HARDWARE_VER_LEN-1, (unsigned char*)&ack_data[1], FAN_HARDWARE_VER_LEN-1) != 0)
    {
        return -ENOMEM;
    }
#else
    memcpy(hardware_version, (unsigned char*)&ack_data[1], FAN_HARDWARE_VER_LEN-1);
#endif

    hardware_version[FAN_HARDWARE_VER_LEN-1]='\0';
#ifdef C11_ANNEX_K
    return sprintf_s(buf, PAGE_SIZE, "%s\n", hardware_version);
#else
    return sprintf(buf, "%s\n", hardware_version);
#endif
}
EXPORT_SYMBOL_GPL(drv_get_hw_version_from_bmc);

ssize_t drv_get_model_name_from_bmc(unsigned int index, char *buf)

{
    int ipmi_ret;
    ott_ipmi_xfer_info_s ipmi_info = {0};
    unsigned char data[2];
    unsigned char ack_data[FAN_MAX_LEN];
    unsigned char model_name[FAN_MODEL_NAME_LEN];
    unsigned int loglevel = loglevel_bmc;

    data[0] = (unsigned char)index;
    data[1] = fan_model_name;
#ifdef C11_ANNEX_K
    memset_s(ack_data,FAN_MAX_LEN,0,FAN_MAX_LEN);
#else
    memset(ack_data,0,FAN_MAX_LEN);
#endif
    ipmi_info.req.netfn = IPMI_NETFN;
    ipmi_info.req.cmd = IPMI_FAN_GET_EEPROM;
    ipmi_info.req.data = data;
    ipmi_info.req.data_len = sizeof(data);

    ipmi_info.rsp.data = ack_data;
    ipmi_info.rsp.data_len = FAN_MAX_LEN;
    ipmi_ret = ott_ipmi_xfer(&ipmi_info);

    if(ipmi_ret != 0)
    {
        SENSOR_DEBUG("%d, get fan model_name fail! ipmi_ret:0x%x,index:%d.\n",__LINE__,ipmi_ret,index);
        return (unsigned int)ipmi_ret;
    }

    if(ack_data[0] != 0)
    {
        SENSOR_DEBUG("%d, get fan model_name fail! ipmi_ret:0x%x,ack_data[0]:0x%x,index:%d.\n",__LINE__,ipmi_ret,ack_data[0],index);
        return -EIO;
    }
 
    if((ipmi_info.rsp.netfn != (ipmi_info.req.netfn + 1)) || (ipmi_info.rsp.cmd != ipmi_info.req.cmd))
    {
        SENSOR_DEBUG("%d, get fan model_name fail! ipmi_ret:0x%x,ack_data[0]:0x%x,index:%d,,ipmi_info.rsp.netfn:0x%x,ipmi_info.req.cmd:0x%x.\n",__LINE__,ipmi_ret,ack_data[0],index,ipmi_info.rsp.netfn,ipmi_info.req.cmd);
        return -EIO;
    }
#ifdef C11_ANNEX_K
    if(memcpy_s(model_name,FAN_MODEL_NAME_LEN-1,(unsigned char*)&ack_data[1],FAN_MODEL_NAME_LEN-1) != 0)
    {
        return -ENOMEM;
    }
#else
    memcpy(model_name,(unsigned char*)&ack_data[1],FAN_MODEL_NAME_LEN-1);
#endif

    model_name[FAN_MODEL_NAME_LEN-1]='\0';
#ifdef C11_ANNEX_K
    return sprintf_s(buf, PAGE_SIZE, "%s\n", model_name);
#else
    return sprintf(buf, "%s\n", model_name);
#endif


}
EXPORT_SYMBOL_GPL(drv_get_model_name_from_bmc);

ssize_t drv_get_sn_from_bmc(unsigned int index, char *buf)
{
    int ipmi_ret;
    ott_ipmi_xfer_info_s ipmi_info = {0};

    unsigned char data[2];
    unsigned char ack_data[FAN_MAX_LEN];
    unsigned char serial_number[FAN_SERIAL_NUM_LEN];
    unsigned int loglevel = loglevel_bmc;

    data[0] = (unsigned char)index;
    data[1] = fan_serial_number;
#ifdef C11_ANNEX_K
    memset_s(ack_data, FAN_MAX_LEN, 0, FAN_MAX_LEN);
#else
    memset(ack_data, 0, FAN_MAX_LEN);
#endif
    ipmi_info.req.netfn = IPMI_NETFN;
    ipmi_info.req.cmd = IPMI_FAN_GET_EEPROM;
    ipmi_info.req.data = data;
    ipmi_info.req.data_len = sizeof(data);

    ipmi_info.rsp.data = ack_data;
    ipmi_info.rsp.data_len = FAN_MAX_LEN;
    ipmi_ret = ott_ipmi_xfer(&ipmi_info);

    if(ipmi_ret != 0)
    {
        SENSOR_DEBUG("%d, get fan serial_number fail! ipmi_ret:0x%x,index:%d.\n",__LINE__,ipmi_ret,index);
        return (unsigned int)ipmi_ret;
    }

    if(ack_data[0] != 0)
    {
        SENSOR_DEBUG("%d, get fan serial_number fail! ipmi_ret:0x%x,ack_data[0]:0x%x,index:%d.\n",__LINE__,ipmi_ret,ack_data[0],index);
        return -EIO;
    }

    if((ipmi_info.rsp.netfn != (ipmi_info.req.netfn + 1)) || (ipmi_info.rsp.cmd != ipmi_info.req.cmd))
    {
        SENSOR_DEBUG("%d, get fan serial_number fail! ipmi_ret:0x%x,ack_data[0]:0x%x,index:%d,,ipmi_info.rsp.netfn:0x%x,ipmi_info.req.cmd:0x%x.\n",__LINE__,ipmi_ret,ack_data[0],index,ipmi_info.rsp.netfn,ipmi_info.req.cmd);
        return -EIO;
    }
#ifdef C11_ANNEX_K
    if(memcpy_s(serial_number, FAN_SERIAL_NUM_LEN-1, (unsigned char*)&ack_data[1], FAN_SERIAL_NUM_LEN-1) != 0)
    {
        return -ENOMEM;
    }
#else
    memcpy(serial_number, (unsigned char*)&ack_data[1], FAN_SERIAL_NUM_LEN-1);
#endif

    serial_number[FAN_SERIAL_NUM_LEN-1]='\0';
#ifdef C11_ANNEX_K
    return sprintf_s(buf, PAGE_SIZE, "%s\n", serial_number);
#else
    return sprintf(buf, "%s\n", serial_number);
#endif

}
EXPORT_SYMBOL_GPL(drv_get_sn_from_bmc);

ssize_t drv_get_vendor_from_bmc(unsigned int index, char *buf)
{
    int ipmi_ret;
    ott_ipmi_xfer_info_s ipmi_info = {0};

    unsigned char data[2];
    unsigned char ack_data[FAN_MAX_LEN];
    unsigned char vendor[FAN_VENDOR_LEN];
    unsigned int loglevel = loglevel_bmc;

    data[0] = (unsigned char)index;
    data[1] = fan_vendor;
#ifdef C11_ANNEX_K
    memset_s(ack_data,FAN_MAX_LEN,0,FAN_MAX_LEN);
#else
    memset(ack_data,0,FAN_MAX_LEN);
#endif
    ipmi_info.req.netfn = IPMI_NETFN;
    ipmi_info.req.cmd = IPMI_FAN_GET_EEPROM;
    ipmi_info.req.data = data;
    ipmi_info.req.data_len = sizeof(data);

    ipmi_info.rsp.data = ack_data;
    ipmi_info.rsp.data_len = FAN_MAX_LEN;
    ipmi_ret = ott_ipmi_xfer(&ipmi_info);

    if(ipmi_ret != 0)
    {
        SENSOR_DEBUG("%d, get fan vendor fail! ipmi_ret:0x%x,index:%d.\n",__LINE__,ipmi_ret,index);
        return (unsigned int)ipmi_ret;
    }

    if(ack_data[0] != 0)
    {
        SENSOR_DEBUG("%d, get fan vendor fail! ipmi_ret:0x%x,ack_data[0]:0x%x,index:%d.\n",__LINE__,ipmi_ret,ack_data[0],index);
        return -EIO;
    }
 
    if((ipmi_info.rsp.netfn != (ipmi_info.req.netfn + 1)) || (ipmi_info.rsp.cmd != ipmi_info.req.cmd))
    {
        SENSOR_DEBUG("%d, get fan vendor fail! ipmi_ret:0x%x,ack_data[0]:0x%x,index:%d,,ipmi_info.rsp.netfn:0x%x,ipmi_info.req.cmd:0x%x.\n",__LINE__,ipmi_ret,ack_data[0],index,ipmi_info.rsp.netfn,ipmi_info.req.cmd);
        return -EIO;
    }

#ifdef C11_ANNEX_K
    if(memcpy_s(vendor,FAN_VENDOR_LEN-1,(unsigned char*)&ack_data[1],FAN_VENDOR_LEN-1) != 0)
    {
        return -ENOMEM;
    }
#else
    memcpy(vendor,(unsigned char*)&ack_data[1],FAN_VENDOR_LEN-1);
#endif

    vendor[FAN_VENDOR_LEN-1]='\0';
#ifdef C11_ANNEX_K
    return sprintf_s(buf, PAGE_SIZE, "%s\n", vendor);
#else
    return sprintf(buf, "%s\n", vendor);
#endif

}
EXPORT_SYMBOL_GPL(drv_get_vendor_from_bmc);

ssize_t drv_get_part_number_from_bmc(unsigned int index, char *buf)
{
    int ipmi_ret;
    ott_ipmi_xfer_info_s ipmi_info = {0};

    unsigned char data[2];
    unsigned char ack_data[FAN_MAX_LEN];
    unsigned char part_number[FAN_PART_NUM_LEN];
    unsigned int loglevel = loglevel_bmc;

    data[0] = (unsigned char)index;
    data[1] = fan_part_number;
#ifdef C11_ANNEX_K
    memset_s(ack_data, FAN_MAX_LEN, 0, FAN_MAX_LEN);
#else
    memset(ack_data, 0, FAN_MAX_LEN);
#endif
    ipmi_info.req.netfn = IPMI_NETFN;
    ipmi_info.req.cmd = IPMI_FAN_GET_EEPROM;
    ipmi_info.req.data = data;
    ipmi_info.req.data_len = sizeof(data);

    ipmi_info.rsp.data = ack_data;
    ipmi_info.rsp.data_len = FAN_MAX_LEN;
    ipmi_ret = ott_ipmi_xfer(&ipmi_info);


    if(ipmi_ret != 0)
    {
        SENSOR_DEBUG("%d, get fan part_number fail! ipmi_ret:0x%x,index:%d.\n",__LINE__,ipmi_ret,index);
        return (unsigned int)ipmi_ret;
    }

    if(ack_data[0] != 0)
    {
        SENSOR_DEBUG("%d, get fan part_number fail! ipmi_ret:0x%x,ack_data[0]:0x%x,index:%d.\n",__LINE__,ipmi_ret,ack_data[0],index);
        return -EIO;
    }

    if((ipmi_info.rsp.netfn != (ipmi_info.req.netfn + 1)) || (ipmi_info.rsp.cmd != ipmi_info.req.cmd))
    {
        SENSOR_DEBUG("%d, get fan part_number fail! ipmi_ret:0x%x,ack_data[0]:0x%x,index:%d,,ipmi_info.rsp.netfn:0x%x,ipmi_info.req.cmd:0x%x.\n",__LINE__,ipmi_ret,ack_data[0],index,ipmi_info.rsp.netfn,ipmi_info.req.cmd);
        return -EIO;
    }
#ifdef C11_ANNEX_K
    if(memcpy_s(part_number, FAN_PART_NUM_LEN-1, (unsigned char*)&ack_data[1], FAN_PART_NUM_LEN-1) != 0)
    {
        return -ENOMEM;
    }
#else
    memcpy(part_number, (unsigned char*)&ack_data[1], FAN_PART_NUM_LEN-1);
#endif

    part_number[FAN_PART_NUM_LEN-1]='\0';
#ifdef C11_ANNEX_K
    return sprintf_s(buf, PAGE_SIZE, "%s\n", part_number);
#else
    return sprintf(buf, "%s\n", part_number);
#endif

}
EXPORT_SYMBOL_GPL(drv_get_part_number_from_bmc);

ssize_t drv_get_speed_from_bmc(unsigned int slot_id, unsigned int fan_id, unsigned int *speed)
{
    int ipmi_ret;
    ott_ipmi_xfer_info_s ipmi_info = {0};
    unsigned char data[2] = {1};
    unsigned char ack_data[5];
    unsigned int loglevel = loglevel_bmc;
    data[0]=(unsigned char)(slot_id);
    data[1]=(unsigned char)(fan_id);
#ifdef C11_ANNEX_K
    memset_s(ack_data,5,0,5);
#else
    memset(ack_data,0,5);
#endif
    ipmi_info.req.netfn = IPMI_NETFN;
    ipmi_info.req.cmd = IPMI_GET_FAN_SPEED;
    ipmi_info.req.data = data;
    ipmi_info.req.data_len = sizeof(data);

    ipmi_info.rsp.data = ack_data;
    ipmi_info.rsp.data_len = 5;
    ipmi_ret = ott_ipmi_xfer(&ipmi_info);

    if(ipmi_ret != 0)
    {
        SENSOR_DEBUG("%d, get fan speed fail! ipmi_ret:0x%x,slot_id:%d,fan_id:%d.\n",
            __LINE__,ipmi_ret,slot_id,fan_id);
        return (unsigned int)ipmi_ret;
    }

    if(ack_data[0] != 0)
    {
        SENSOR_DEBUG("%d, get fan speed fail! ipmi_ret:0x%x,ack_data[0]:0x%x,slot_id:%d,fan_id:%d.\n",
            __LINE__,ipmi_ret,ack_data[0],slot_id,fan_id);
        return -EIO;
    }

    if((ipmi_info.rsp.netfn != (ipmi_info.req.netfn + 1)) || (ipmi_info.rsp.cmd != ipmi_info.req.cmd))
    {
        SENSOR_DEBUG("%d, get fan speed fail! ipmi_ret:0x%x,ack_data[0]:0x%x,slot_id:%d,fan_id:%d.\n",
            __LINE__,ipmi_ret,ack_data[0],slot_id,fan_id);
        return -EIO;
    }
    *speed = (((int)(ack_data[1])) << 24) + (((int)(ack_data[2])) << 16) + (((int)(ack_data[3])) << 8) + (int)(ack_data[4]);

    return 0;
}
EXPORT_SYMBOL_GPL(drv_get_speed_from_bmc);

ssize_t drv_get_pwm_from_bmc(unsigned int fan_id, int *pwm)
{
    int ipmi_ret;
    ott_ipmi_xfer_info_s ipmi_info = {0};
    unsigned char data[1] = {1};
    unsigned char ack_data[2];
    unsigned int loglevel = loglevel_bmc;
    data[0]=(unsigned char)(fan_id);
#ifdef C11_ANNEX_K
    memset_s(ack_data,2,0,2);
#else
    memset(ack_data,0,2);
#endif
    ipmi_info.req.netfn = IPMI_NETFN;
    ipmi_info.req.cmd = IPMI_GET_FAN_PWM;
    ipmi_info.req.data = data;
    ipmi_info.req.data_len = sizeof(data);

    ipmi_info.rsp.data = ack_data;
    ipmi_info.rsp.data_len = 2;
    ipmi_ret = ott_ipmi_xfer(&ipmi_info);

    if(ipmi_ret != 0)
    {
        SENSOR_DEBUG("%d, get fan speed fail! ipmi_ret:0x%x,fan_id:%d.\n",
            __LINE__,ipmi_ret,fan_id);
        return (unsigned int)ipmi_ret;
    }

    if(ack_data[0] != 0)
    {
        SENSOR_DEBUG("%d, get fan speed fail! ipmi_ret:0x%x,ack_data[0]:0x%x,fan_id:%d.\n",
            __LINE__,ipmi_ret,ack_data[0],fan_id);
        return -EIO;
    }

    if((ipmi_info.rsp.netfn != (ipmi_info.req.netfn + 1)) || (ipmi_info.rsp.cmd != ipmi_info.req.cmd))
    {
        SENSOR_DEBUG("%d, get fan speed fail! ipmi_ret:0x%x,ack_data[0]:0x%x,fan_id:%d.\n",
            __LINE__,ipmi_ret,ack_data[0],fan_id);
        return -EIO;
    }
    *pwm = (int)(ack_data[1]);

    return 0;
}
EXPORT_SYMBOL_GPL(drv_get_pwm_from_bmc);

ssize_t drv_set_pwm_from_bmc(unsigned int fan_id, int pwm)
{
    int ipmi_ret;
    ott_ipmi_xfer_info_s ipmi_info = {0};
    unsigned char data[2] = {1};
    unsigned char ack_data[2];
    unsigned int loglevel = loglevel_bmc;
    data[0]=(unsigned char)(fan_id);
    data[1]=(unsigned char)(pwm);
#ifdef C11_ANNEX_K
    memset_s(ack_data,2,0,2);
#else
    memset(ack_data,0,2);
#endif
    ipmi_info.req.netfn = IPMI_NETFN;
    ipmi_info.req.cmd = IPMI_SET_FAN_PWM;
    ipmi_info.req.data = data;
    ipmi_info.req.data_len = sizeof(data);

    ipmi_info.rsp.data = ack_data;
    ipmi_info.rsp.data_len = 2;
    ipmi_ret = ott_ipmi_xfer(&ipmi_info);

    if(ipmi_ret != 0)
    {
        SENSOR_DEBUG("%d, get fan speed fail! ipmi_ret:0x%x,fan_id:%d.\n",
            __LINE__,ipmi_ret,fan_id);
        return (unsigned int)ipmi_ret;
    }

    if(ack_data[0] != 0)
    {
        SENSOR_DEBUG("%d, get fan speed fail! ipmi_ret:0x%x,ack_data[0]:0x%x,fan_id:%d.\n",
            __LINE__,ipmi_ret,ack_data[0],fan_id);
        return -EIO;
    }

    if((ipmi_info.rsp.netfn != (ipmi_info.req.netfn + 1)) || (ipmi_info.rsp.cmd != ipmi_info.req.cmd))
    {
        SENSOR_DEBUG("%d, get fan speed fail! ipmi_ret:0x%x,ack_data[0]:0x%x,fan_id:%d.\n",
            __LINE__,ipmi_ret,ack_data[0],fan_id);
        return -EIO;
    }

    return 0;
}
EXPORT_SYMBOL_GPL(drv_set_pwm_from_bmc);

int drv_get_mfr_info_from_bmc(unsigned int psu_index, u8 pmbus_command, char *buf)
{
    int ipmi_ret;
    int value = 0;
    int psu_present = 0;
    int psu_alarm = 0;
    int ack_data_len = 5;
    int ack_data_handle = 0;

    ott_ipmi_xfer_info_s ipmi_info = {0};
    unsigned char data[2] = {};
    unsigned char ack_data[PMBUS_MFR_MAX_LEN];
    unsigned int loglevel = loglevel_bmc;
    data[0]=(unsigned char)(psu_index);
    data[1]=(unsigned char)(pmbus_command);
#ifdef C11_ANNEX_K
    memset_s(ack_data,PMBUS_MFR_MAX_LEN,0,PMBUS_MFR_MAX_LEN);
#else
    memset(ack_data,0,PMBUS_MFR_MAX_LEN);
#endif
    ipmi_info.req.netfn = IPMI_NETFN;
    ipmi_info.req.data = data;
    ipmi_info.req.data_len = sizeof(data);

    ipmi_info.rsp.data = ack_data;
    ipmi_info.rsp.data_len = PMBUS_MFR_MAX_LEN;

    switch(pmbus_command)
    {
        case PMBUS_STATUS_WORD:
        case PMBUS_IIN_OC_WARN_LIMIT:
        case PMBUS_VIN_OV_WARN_LIMIT:
            // PMBUS_IIN_OC_WARN_LIMIT and PMBUS_VIN_OV_WARN_LIMIT are hard code since PSU not support
            // But we still need to check PSU status. Return NA if get failed
            ack_data_handle = ACK_DATA_RESP_BITMAP;
            break;
        case PMBUS_STATUS_INPUT:
            ack_data_handle = ACK_DATA_RESP_PRESENT;
            break;
        case PMBUS_POUT_MAX:
            ipmi_info.req.cmd = IPMI_GET_PSU_ATTRIBUTE_DATA;
            break;
        case PMBUS_READ_TEMPERATURE_1:
            ipmi_info.req.cmd = IPMI_GET_PSU_ATTRIBUTE_DATA;
            ack_data_handle = ACK_DATA_RESP_FLOAT;
            break;
        case PMBUS_READ_VIN:
            ipmi_info.req.cmd = IPMI_GET_PSU_ATTRIBUTE_DATA;
            ack_data_handle = ACK_DATA_RESP_FLOAT;
            break;
        case PMBUS_READ_VOUT:
            ipmi_info.req.cmd = IPMI_GET_PSU_ATTRIBUTE_DATA;
            ack_data_handle = ACK_DATA_RESP_FLOAT;
            break;
        case PMBUS_READ_IIN:
            ipmi_info.req.cmd = IPMI_GET_PSU_ATTRIBUTE_DATA;
            ack_data_handle = ACK_DATA_RESP_FLOAT;
            break;
        case PMBUS_READ_IOUT:
            ipmi_info.req.cmd = IPMI_GET_PSU_ATTRIBUTE_DATA;
            ack_data_handle = ACK_DATA_RESP_FLOAT;
            break;
        case PMBUS_READ_PIN:
            ipmi_info.req.cmd = IPMI_GET_PSU_ATTRIBUTE_DATA;
            ack_data_handle = ACK_DATA_RESP_FLOAT_DIV;
            break;
        case PMBUS_READ_POUT:
            ipmi_info.req.cmd = IPMI_GET_PSU_ATTRIBUTE_DATA;
            ack_data_handle = ACK_DATA_RESP_FLOAT_DIV;
            break;
        case PMBUS_READ_FAN_SPEED_1:
            ipmi_info.req.cmd = IPMI_GET_PSU_ATTRIBUTE_DATA;
            break;
        case PMBUS_MFR_SERIAL:
            ack_data_len = PMBUS_MFR_SERIAL_LEN + 1;
            ipmi_info.req.cmd = IPMI_PSU_GET_EEPROM;
            ack_data_handle = ACK_DATA_RESP_CHAR;
            break;
        case PMBUS_MFR_ID:
            ack_data_len = PMBUS_MFR_ID_LEN + 1;
            ipmi_info.req.cmd = IPMI_PSU_GET_EEPROM;
            ack_data_handle = ACK_DATA_RESP_CHAR;
            break;
        case PMBUS_MFR_MODEL:
            ack_data_len = PMBUS_MFR_MODEL_LEN + 1;
            ipmi_info.req.cmd = IPMI_PSU_GET_EEPROM;
            ack_data_handle = ACK_DATA_RESP_CHAR;
            break;
        case PMBUS_MFR_REVISION:
            ack_data_len = PMBUS_MFR_REVISION_LEN + 1;
            ipmi_info.req.cmd = IPMI_PSU_GET_EEPROM;
            ack_data_handle = ACK_DATA_RESP_CHAR;
            break;
        case PMBUS_MFR_DATE:
            ack_data_len = PMBUS_MFR_DATE_LEN + 1;
            ipmi_info.req.cmd = IPMI_PSU_GET_EEPROM;
            ack_data_handle = ACK_DATA_RESP_CHAR;
            break;
        case PMBUS_PART_NUM:
            ack_data_len = PMBUS_PART_NUM_LEN + 1;
            ipmi_info.req.cmd = IPMI_PSU_GET_EEPROM;
            ack_data_handle = ACK_DATA_RESP_CHAR;
            break;

        default:
#ifdef C11_ANNEX_K
            if(sprintf_s(buf, PAGE_SIZE, "%x", pmbus_command) < 0)
#else
            if(sprintf(buf, "%x", pmbus_command) < 0)
#endif
            {
                return -ENOMEM;
            }
            return 0;
            break;
    }

    ipmi_ret = ott_ipmi_xfer(&ipmi_info);


    if(ipmi_ret != 0)
    {
        SENSOR_DEBUG("%d, get fan info fail! ipmi_ret:0x%x,psu_index:%d,pmbus_command:%x.\n",
            __LINE__,ipmi_ret,psu_index,pmbus_command);
        return (unsigned int)ipmi_ret;
    }

    if(ack_data[0] != 0)
    {
        SENSOR_DEBUG("%d, get fan info fail! ipmi_ret:0x%x,ack_data[0]:0x%x,psu_index:%d,pmbus_command:%x.\n",
            __LINE__,ipmi_ret,ack_data[0],psu_index,pmbus_command);
        return -EIO;
    }

    if((ipmi_info.rsp.netfn != (ipmi_info.req.netfn + 1)) || (ipmi_info.rsp.cmd != ipmi_info.req.cmd))
    {
        SENSOR_DEBUG("%d, get fan info fail! ipmi_ret:0x%x,ack_data[0]:0x%x,psu_index:%d,pmbus_command:%x.\n",
            __LINE__,ipmi_ret,ack_data[0],psu_index,pmbus_command);
        return -EIO;
    }

    if(ack_data_handle == ACK_DATA_RESP_CHAR)
    {
#ifdef C11_ANNEX_K
        if(memcpy_s(buf,PAGE_SIZE,ack_data+1,ack_data_len) != 0)
        {
            return -ENOMEM;
        }
#else
        memcpy(buf,ack_data+1,ack_data_len);
#endif
        return 0;
    }
    else if(ack_data_handle == ACK_DATA_RESP_INT)
    {
        char value[ack_data_len];
        int i = 0;
        for(i = 1 ; i < ack_data_len ; i++)
            value[i]= (int)(ack_data[i]);
#ifdef C11_ANNEX_K
        if(memcpy_s(buf,PAGE_SIZE,&value,sizeof(value)) != 0)
        {
            return -ENOMEM;
        }
#else
        memcpy(buf,&value,sizeof(value));
#endif
        return 0;
    }
    value = (((int)(ack_data[1])) << 24) + (((int)(ack_data[2])) << 16) + (((int)(ack_data[3])) << 8) + (int)(ack_data[4]);
    if(ack_data_handle == ACK_DATA_RESP_FLOAT)
    {
#ifdef C11_ANNEX_K
        if(sprintf_s(buf, PAGE_SIZE, "%d", value) < 0)
#else
        if(sprintf(buf, "%d", value) < 0)
#endif
        {
            return -ENOMEM;
        }
        return 0;
    }
    else if(ack_data_handle == ACK_DATA_RESP_FLOAT_DIV)
    {
        value = value;
#ifdef C11_ANNEX_K
        if(sprintf_s(buf, PAGE_SIZE, "%d", value) < 0)
#else
        if(sprintf(buf, "%d", value) < 0)
#endif
        {
            return -ENOMEM;
        }
        return 0;
    }
    else if(ack_data_handle == ACK_DATA_RESP_PRESENT)
    {
        if(value)
        {
            psu_present = IPMI_PSU_ABSENT;
        }
        else
        {
            psu_present = IPMI_PSU_PRESENT;
        }
#ifdef C11_ANNEX_K
        if(sprintf_s(buf, PAGE_SIZE, "%d", psu_present) < 0)
#else
        if(sprintf(buf, "%d", psu_present) < 0)
#endif
        {
            return -ENOMEM;
        }
        return 0;
    }
    else if(ack_data_handle == ACK_DATA_RESP_BITMAP)
    {
        psu_alarm = value & 0x1fff;
#ifdef C11_ANNEX_K
        return sprintf_s(buf, PAGE_SIZE, "%d", psu_alarm);
#else
	    return sprintf(buf, "%d", psu_alarm);
#endif
    }
#ifdef C11_ANNEX_K
    if(sprintf_s(buf, PAGE_SIZE, "%d", value) < 0)
#else
    if(sprintf(buf, "%d", value) < 0)
#endif
    {
        return -ENOMEM;
    }

    return 0;
}
EXPORT_SYMBOL_GPL(drv_get_mfr_info_from_bmc);

ssize_t drv_fmea_get_work_status_from_bmc(unsigned int index, char *buf, char *plt)
{
    int ipmi_ret;
    ott_ipmi_xfer_info_s ipmi_info = {0};
    unsigned char data[] = {(unsigned char)index, 1};
    unsigned char ack_data[5];
    unsigned char work_status[5];
    unsigned int loglevel = loglevel_bmc;
#ifdef C11_ANNEX_K
    memset_s(ack_data,5,0,5);
#else
    memset(ack_data,0,5);
#endif
    ipmi_info.req.netfn = IPMI_NETFN;
    ipmi_info.req.cmd = IPMI_CMD_GET_FMEA_STATUS;
    ipmi_info.req.data = data;
    ipmi_info.req.data_len = sizeof(data);

    ipmi_info.rsp.data = ack_data;
    ipmi_info.rsp.data_len = 5;
    ipmi_ret = ott_ipmi_xfer(&ipmi_info);

    if(ipmi_ret != 0)
    {
        SENSOR_DEBUG("%s:%d  fail! ipmi_ret:0x%x,index:%d.\n",__func__,__LINE__,ipmi_ret,index);
        return (unsigned int)ipmi_ret;
    }

    if(ack_data[0] != 0)
    {
        SENSOR_DEBUG("%s:%d  fail! ipmi_ret:0x%x,ack_data[0]:0x%x,index:%d.\n",__func__,__LINE__,ipmi_ret,ack_data[0],index);
        return -EIO;
    }

    if((ipmi_info.rsp.netfn != (ipmi_info.req.netfn + 1)) || (ipmi_info.rsp.cmd != ipmi_info.req.cmd))
    {
        SENSOR_DEBUG("%s:%d  fail! ipmi_ret:0x%x,ack_data[0]:0x%x,index:%d,,ipmi_info.rsp.netfn:0x%x,ipmi_info.req.cmd:0x%x.\n",__func__,__LINE__,ipmi_ret,ack_data[0],index,ipmi_info.rsp.netfn,ipmi_info.req.cmd);
        return -EIO;
    }
#ifdef C11_ANNEX_K
    if(memcpy_s(work_status,4,(unsigned char*)&ack_data[1],4) != 0)
    {
        return -ENOMEM;
    }
#else
    memcpy(work_status,(unsigned char*)&ack_data[1],4);
#endif

    work_status[4]='\0';
#ifdef C11_ANNEX_K
    return sprintf_s(buf, PAGE_SIZE, "%s\n", work_status);
#else
    return sprintf(buf, "%s\n", work_status);
#endif

}
EXPORT_SYMBOL_GPL(drv_fmea_get_work_status_from_bmc);

ssize_t drv_fmea_get_current_status_from_bmc(unsigned int index, char *buf, char *plt)
{
    int ipmi_ret;
    ott_ipmi_xfer_info_s ipmi_info = {0};
    unsigned char data[] = {(unsigned char)index, 1};
    unsigned char ack_data[5];
    unsigned char current_status[5];
    unsigned int loglevel = loglevel_bmc;
#ifdef C11_ANNEX_K
    memset_s(ack_data,5,0,5);
#else
    memset(ack_data,0,5);
#endif
    ipmi_info.req.netfn = IPMI_NETFN;
    ipmi_info.req.cmd = IPMI_CMD_GET_FMEA_STATUS;
    ipmi_info.req.data = data;
    ipmi_info.req.data_len = sizeof(data);

    ipmi_info.rsp.data = ack_data;
    ipmi_info.rsp.data_len = 5;
    ipmi_ret = ott_ipmi_xfer(&ipmi_info);

    if(ipmi_ret != 0)
    {
        SENSOR_DEBUG("%s:%d  fail! ipmi_ret:0x%x,index:%d.\n",__func__,__LINE__,ipmi_ret,index);
        return (unsigned int)ipmi_ret;
    }

    if(ack_data[0] != 0)
    {
        SENSOR_DEBUG("%s:%d  fail! ipmi_ret:0x%x,ack_data[0]:0x%x,index:%d.\n",__func__,__LINE__,ipmi_ret,ack_data[0],index);
        return -EIO;
    }

    if((ipmi_info.rsp.netfn != (ipmi_info.req.netfn + 1)) || (ipmi_info.rsp.cmd != ipmi_info.req.cmd))
    {
        SENSOR_DEBUG("%s:%d  fail! ipmi_ret:0x%x,ack_data[0]:0x%x,index:%d,,ipmi_info.rsp.netfn:0x%x,ipmi_info.req.cmd:0x%x.\n",__func__,__LINE__,ipmi_ret,ack_data[0],index,ipmi_info.rsp.netfn,ipmi_info.req.cmd);
        return -EIO;
    }
#ifdef C11_ANNEX_K
    if(memcpy_s(current_status,4,(unsigned char*)&ack_data[1],4) != 0)
    {
        return -ENOMEM;
    }
#else
    memcpy(current_status,(unsigned char*)&ack_data[1],4);
#endif

    current_status[4]='\0';
#ifdef C11_ANNEX_K
    return sprintf_s(buf, PAGE_SIZE, "%s\n", current_status);
#else
    return sprintf(buf, "%s\n", current_status);
#endif

}
EXPORT_SYMBOL_GPL(drv_fmea_get_current_status_from_bmc);

ssize_t drv_fmea_get_pmbus_status_from_bmc(unsigned int index, char *buf, char *plt)
{
    int ipmi_ret;
    ott_ipmi_xfer_info_s ipmi_info = {0};
    unsigned char data[] = {(unsigned char)index, 1};
    unsigned char ack_data[5];
    unsigned char pmbus_status[5];
    unsigned int loglevel = loglevel_bmc;
#ifdef C11_ANNEX_K
    memset_s(ack_data,5,0,5);
#else
    memset(ack_data,0,5);
#endif
    ipmi_info.req.netfn = IPMI_NETFN;
    ipmi_info.req.cmd = IPMI_CMD_GET_FMEA_STATUS;
    ipmi_info.req.data = data;
    ipmi_info.req.data_len = sizeof(data);

    ipmi_info.rsp.data = ack_data;
    ipmi_info.rsp.data_len = 5;
    ipmi_ret = ott_ipmi_xfer(&ipmi_info);

    if(ipmi_ret != 0)
    {
        SENSOR_DEBUG("%s:%d  fail! ipmi_ret:0x%x,index:%d.\n",__func__,__LINE__,ipmi_ret,index);
        return (unsigned int)ipmi_ret;
    }

    if(ack_data[0] != 0)
    {
        SENSOR_DEBUG("%s:%d  fail! ipmi_ret:0x%x,ack_data[0]:0x%x,index:%d.\n",__func__,__LINE__,ipmi_ret,ack_data[0],index);
        return -EIO;
    }

    if((ipmi_info.rsp.netfn != (ipmi_info.req.netfn + 1)) || (ipmi_info.rsp.cmd != ipmi_info.req.cmd))
    {
        SENSOR_DEBUG("%s:%d  fail! ipmi_ret:0x%x,ack_data[0]:0x%x,index:%d,,ipmi_info.rsp.netfn:0x%x,ipmi_info.req.cmd:0x%x.\n",__func__,__LINE__,ipmi_ret,ack_data[0],index,ipmi_info.rsp.netfn,ipmi_info.req.cmd);
        return -EIO;
    }
#ifdef C11_ANNEX_K
    if(memcpy_s(pmbus_status,4,(unsigned char*)&ack_data[1],4) != 0)
    {
        return -ENOMEM;
    }
#else
    memcpy(pmbus_status,(unsigned char*)&ack_data[1],4);
#endif

    pmbus_status[4]='\0';
#ifdef C11_ANNEX_K
    return sprintf_s(buf, PAGE_SIZE, "%s\n", pmbus_status);
#else
    return sprintf(buf, "%s\n", pmbus_status);
#endif

}
EXPORT_SYMBOL_GPL(drv_fmea_get_pmbus_status_from_bmc);

ssize_t drv_fmea_get_mfr_id_from_bmc(unsigned int index, int *value)
{
    int ipmi_ret;
    ott_ipmi_xfer_info_s ipmi_info = {0};
    unsigned char ack_data[3];
    unsigned char data[] = {(unsigned char)index, 4};
    unsigned int loglevel = loglevel_bmc;
#ifdef C11_ANNEX_K
    memset_s(ack_data,3,0,3);
#else
    memset(ack_data,0,3);
#endif
    ipmi_info.req.netfn = IPMI_NETFN;
    ipmi_info.req.cmd = IPMI_CMD_GET_FMEA_STATUS;
    ipmi_info.req.data = data;
    ipmi_info.req.data_len = sizeof(data);

    ipmi_info.rsp.data = ack_data;
    ipmi_info.rsp.data_len = 3;
    ipmi_ret = ott_ipmi_xfer(&ipmi_info);

    if(ipmi_ret != 0)
    {
        SENSOR_DEBUG("Fail! ipmi_ret:0x%x,index:%d.\n",ipmi_ret,index);
        return (unsigned int)ipmi_ret;
    }

    if(ack_data[0] != 0)
    {
        SENSOR_DEBUG("Fail! ipmi_ret:0x%x,ack_data[0]:0x%x,index:%d.\n",ipmi_ret,ack_data[0],index);
        return -EIO;
    }

    if((ipmi_info.rsp.netfn != (ipmi_info.req.netfn + 1)) || (ipmi_info.rsp.cmd != ipmi_info.req.cmd))
    {
        SENSOR_DEBUG("Fail! ipmi_ret:0x%x,ack_data[0]:0x%x,index:%d,,ipmi_info.rsp.netfn:0x%x,ipmi_info.req.cmd:0x%x.\n",ipmi_ret,ack_data[0],index,ipmi_info.rsp.netfn,ipmi_info.req.cmd);
        return -EIO;
    }
#ifdef C11_ANNEX_K
    if(memcpy_s((char *)value,2,(unsigned char*)&ack_data[1],2) != 0)
    {
        return -ENOMEM;
    }
#else
    memcpy((char *)value,(unsigned char*)&ack_data[1],2);
#endif


    return 0;
}
EXPORT_SYMBOL_GPL(drv_fmea_get_mfr_id_from_bmc);

int drv_cpld_write_from_bmc(unsigned char index, unsigned char reg, unsigned char value)
{
    int ipmi_ret;
    ott_ipmi_xfer_info_s ipmi_info = {0};
    unsigned char data[4];
    unsigned char ack_data[2];
    unsigned int loglevel = loglevel_bmc;

    if(index != FAN_CPLD)
    {
        return -1;
    }

    data[0] = index;
    data[1] = CPLD_WRITE;
    data[2] = reg;
    data[3] = value;
#ifdef C11_ANNEX_K
    memset_s(ack_data, 2, 0, 2);
#else
    memset(ack_data, 0, 2);
#endif
    ipmi_info.req.netfn = IPMI_NETFN;
    ipmi_info.req.cmd = IPMI_CPLD_REG_ACCESS;
    ipmi_info.req.data = data;
    ipmi_info.req.data_len = sizeof(data);

    ipmi_info.rsp.data = ack_data;
    ipmi_info.rsp.data_len = 2;
    ipmi_ret = ott_ipmi_xfer(&ipmi_info);

    if(ipmi_ret != 0)
    {
        SENSOR_DEBUG("%d, write cpld fail! ipmi_ret:0x%x,cpld_id:%d.\n",
            __LINE__,ipmi_ret,index);
        return (unsigned int)ipmi_ret;
    }

    if(ack_data[0] != 0)
    {
        SENSOR_DEBUG("%d, write cpld fail! ipmi_ret:0x%x,ack_data[0]:0x%x,cpld_id:%d.\n",
            __LINE__,ipmi_ret,ack_data[0],index);
        return -EIO;
    }

    if((ipmi_info.rsp.netfn != (ipmi_info.req.netfn + 1)) || (ipmi_info.rsp.cmd != ipmi_info.req.cmd))
    {
        SENSOR_DEBUG("%d, write cpld fail! ipmi_ret:0x%x,ack_data[0]:0x%x,cpld_id:%d.\n",
            __LINE__,ipmi_ret,ack_data[0],index);
        return -EIO;
    }

    return 0;
}
EXPORT_SYMBOL_GPL(drv_cpld_write_from_bmc);

int drv_cpld_read_from_bmc(unsigned char index, unsigned char reg, unsigned char *value)
{
    int ipmi_ret;
    ott_ipmi_xfer_info_s ipmi_info = {0};
    unsigned char data[4];
    unsigned char ack_data[2];
    unsigned int loglevel = loglevel_bmc;

    if(index != FAN_CPLD)
    {
        return -1;
    }

    data[0] = index;
    data[1] = CPLD_READ;
    data[2] = reg;
    data[3] = 0xff;
#ifdef C11_ANNEX_K
    memset_s(ack_data, 2, 0, 2);
#else
    memset(ack_data, 0, 2);
#endif
    ipmi_info.req.netfn = IPMI_NETFN;
    ipmi_info.req.cmd = IPMI_CPLD_REG_ACCESS;
    ipmi_info.req.data = data;
    ipmi_info.req.data_len = sizeof(data);

    ipmi_info.rsp.data = ack_data;
    ipmi_info.rsp.data_len = 2;
    ipmi_ret = ott_ipmi_xfer(&ipmi_info);

    if(ipmi_ret != 0)
    {
        SENSOR_DEBUG("%d, read cpld fail! ipmi_ret:0x%x,cpld_id:%d.\n",
            __LINE__,ipmi_ret,index);
        return (unsigned int)ipmi_ret;
    }

    if(ack_data[0] != 0)
    {
        SENSOR_DEBUG("%d, read cpld fail! ipmi_ret:0x%x,ack_data[0]:0x%x,cpld_id:%d.\n",
            __LINE__,ipmi_ret,ack_data[0],index);
        return -EIO;
    }

    if((ipmi_info.rsp.netfn != (ipmi_info.req.netfn + 1)) || (ipmi_info.rsp.cmd != ipmi_info.req.cmd))
    {
        SENSOR_DEBUG("%d, read cpld fail! ipmi_ret:0x%x,ack_data[0]:0x%x,cpld_id:%d.\n",
            __LINE__,ipmi_ret,ack_data[0],index);
        return -EIO;
    }

    *value = ack_data[1];

    return 0;
}
EXPORT_SYMBOL_GPL(drv_cpld_read_from_bmc);

static int __init sys_impi_init(void)
{
    int ret = 0;
    sema_init(&g_ipmi_msg_sem, 0);
    ret = ipmi_create_user(IPMI_INTF_NUM, &ipmi_hndlrs, NULL, &g_ipmi_mh_user);
    if(ret)
    {
        printk("error:ipmi_create_user failed!!!\n");
    }

    return 0;
}
static void __exit sys_impi_exit(void)
{
    if(g_ipmi_mh_user != NULL)
        ipmi_destroy_user(g_ipmi_mh_user);
    return;
}

MODULE_AUTHOR("Jian Wang <jian007_wang@accton.com.cn>");
MODULE_DESCRIPTION("sys ipmi");
MODULE_VERSION("0.0.0.1");
MODULE_LICENSE("GPL");
module_init(sys_impi_init);
module_exit(sys_impi_exit);
