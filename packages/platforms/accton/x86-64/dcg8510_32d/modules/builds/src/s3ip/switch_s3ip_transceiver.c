#include <linux/module.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/slab.h>

#include "switch_transceiver_driver.h"
#include "switch_optoe.h"

#define SWITCH_S3IP_TRANSCEIVER_VERSION "0.0.0.1"

unsigned int loglevel = 0;
static int multiplier = 100000000;

struct transceiver_attribute{
    struct attribute attr;
    ssize_t (*show)(struct kobject *obj, struct transceiver_attribute *attr, char *buf);
    ssize_t (*store)(struct kobject *obj, struct transceiver_attribute *attr, const char *buf, size_t count);
};

struct transceiver_drivers_t *cb_func = NULL;

/* For s3ip */
extern struct kobject *switch_kobj;
static struct kobject *transceiver_kobj;
static struct kobject *transceiver_index_kobj[TRANSCEIVER_TOTAL_NUM];

enum transceiver_attrs {
    DEBUG,
    LOGLEVEL,
    POWER_ON,
    PRESENT,
    NUM,
    ETH_POWER_ON,
    ETH_RESET,
    ETH_LPMODE,
    ETH_TEMP,
    ETH_PRESENT,
    ETH_INTERRUPT,
    ETH_DIAGNOSTIC,
    ETH_RX_LOS,
    ETH_TX_LOS,
    ETH_TX_DISABLE,
    ETH_RX_DISABLE,
    ETH_TX_CDR_LOL,
    ETH_RX_CDR_LOL,
    ETH_TX_FAULT,
    ETH_MODULE_STATUS,
    ETH_DATAPATH_STATUS,
    ETH_TEMP_TYPE,
    ETH_LED_STATUS,
#if 0 /* use bin_attribute instead of kobj create add */
    ETH_EEPROM,
#endif
    NUM_TRANSCEIVER_ATTR,
};

int get_transceiver_index(struct kobject *kobj)
{
    int retval=0;
    unsigned int transceiver_index;
    char transceiver_index_str[5] = {0};

#ifdef C11_ANNEX_K
    if(memcpy_s(transceiver_index_str, 5, (kobject_name(kobj)+sizeof(ETH_NAME_STRING)-1), 3) != 0)
    {
        return -ENOMEM;
    }
#else
    memcpy(transceiver_index_str, (kobject_name(kobj)+sizeof(ETH_NAME_STRING)-1), 3);
#endif

    retval = kstrtoint(transceiver_index_str, 10, &transceiver_index);
    if(retval == 0)
    {
        TRANSCEIVER_DEBUG("[%s] transceiver_index:%d \n", __func__, transceiver_index);
    }
    else
    {
        TRANSCEIVER_DEBUG("[%s] Error:%d, transceiver_index:%s \n", __func__, retval, transceiver_index_str);
        return -EINVAL;
    }

    return transceiver_index;
}

static ssize_t s3ip_debug_help(struct kobject *kobj, struct transceiver_attribute *attr, char *buf)
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

static ssize_t s3ip_debug(struct kobject *kobj, struct transceiver_attribute *attr, const char *buf, size_t count)
{
    return count;
}

static ssize_t s3ip_get_loglevel(struct kobject *kobj, struct transceiver_attribute *attr, char *buf)
{
#ifdef C11_ANNEX_K
    return sprintf_s(buf, PAGE_SIZE, "%d\n", loglevel);
#else
    return sprintf(buf, "%d\n", loglevel);
#endif
}

static ssize_t s3ip_set_loglevel(struct kobject *kobj, struct transceiver_attribute *attr, const char *buf, size_t count)
{
    int retval=0;
    long lev;

    if(cb_func == NULL)
        return -1;

    retval = kstrtol(buf, 0, &lev);
    if(retval == 0)
    {
        TRANSCEIVER_DEBUG("lev:%ld \n", lev);
    }
    else
    {
        TRANSCEIVER_DEBUG("Error:%d, lev:%s \n", retval, buf);
        return -1;
    }

    loglevel = (unsigned int)lev;

    cb_func->set_loglevel(lev);

    return count;
}

static ssize_t s3ip_get_power_on(struct kobject *kobj, struct transceiver_attribute *attr, char *buf)
{
    unsigned int pwr_value = 0;
    unsigned int transceiver_index = 0;
    char rst_str[TRANSCEIVER_TOTAL_NUM + 1];

    if(cb_func == NULL)
        return -1;

    rst_str[TRANSCEIVER_TOTAL_NUM] = '\0';
    for(transceiver_index = 1; transceiver_index <= TRANSCEIVER_TOTAL_NUM; transceiver_index++)
    {
        pwr_value = cb_func->get_eth_power_on(transceiver_index);
        rst_str[transceiver_index - 1] = pwr_value ? '1':'0';

    }
#ifdef C11_ANNEX_K
    return sprintf_s(buf, PAGE_SIZE, "%s\n", rst_str);
#else
    return sprintf(buf, "%s\n", rst_str);
#endif

}


static ssize_t s3ip_get_present(struct kobject *kobj, struct transceiver_attribute *attr, char *buf)
{
    unsigned int present_value = 0;
    unsigned int transceiver_index = 0;
    char rst_str[TRANSCEIVER_TOTAL_NUM + 1];

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    rst_str[TRANSCEIVER_TOTAL_NUM] = '\0';
    for(transceiver_index = 1; transceiver_index <= TRANSCEIVER_TOTAL_NUM; transceiver_index++)
    {
        present_value = cb_func->get_eth_present(transceiver_index);
        rst_str[transceiver_index - 1] = present_value ? '1':'0';
    }
#ifdef C11_ANNEX_K
    return sprintf_s(buf, PAGE_SIZE, "%s\n", rst_str);
#else
    return sprintf(buf, "%s\n", rst_str);
#endif

}

static ssize_t s3ip_get_num(struct kobject *kobj, struct transceiver_attribute *attr, char *buf)
{
#ifdef C11_ANNEX_K
    return sprintf_s(buf, PAGE_SIZE, "%d\n", TRANSCEIVER_TOTAL_NUM);
#else
    return sprintf(buf, "%d\n", TRANSCEIVER_TOTAL_NUM);
#endif

}

static ssize_t s3ip_get_eth_power_on(struct kobject *kobj, struct transceiver_attribute *attr, char *buf)
{
    unsigned int pwr_value = 0;
    int transceiver_index;
    int retval=0;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    transceiver_index = get_transceiver_index(kobj);
    if(transceiver_index < 0)
    {
        TRANSCEIVER_DEBUG("[%s] Get transceiver index failed.\n", __func__);
        ERROR_RETURN_NA(-EINVAL);
    }

    pwr_value = cb_func->get_eth_power_on(transceiver_index);
#ifdef C11_ANNEX_K
    retval = sprintf_s(buf, PAGE_SIZE, "%d\n", pwr_value);
#else
    retval = sprintf(buf, "%d\n", pwr_value);
#endif


    return retval;
}

static ssize_t s3ip_set_eth_power_on(struct kobject *kobj, struct transceiver_attribute *attr, const char *buf, size_t count)
{
    unsigned int pwr_value = 0;
    int transceiver_index = 0;
    int retval=0;

    if(cb_func == NULL)
        return -EIO;

    transceiver_index = get_transceiver_index(kobj);
    if(transceiver_index < 0)
    {
        TRANSCEIVER_DEBUG("[%s] Get transceiver index failed.\n", __func__);
        return -ENXIO;
    }

    retval = kstrtoint(buf, 0, &pwr_value);
    if(retval != 0)
    {
        return -EINVAL;
    }

    cb_func->set_eth_power_on(transceiver_index, pwr_value);

    return count;
}

static ssize_t s3ip_get_eth_reset(struct kobject *kobj, struct transceiver_attribute *attr, char *buf)
{
    unsigned int rst_value = 0;
    int transceiver_index = 0;
    int retval=0;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    transceiver_index = get_transceiver_index(kobj);
    if(transceiver_index < 0)
    {
        TRANSCEIVER_DEBUG("[%s] Get transceiver index failed.\n", __func__);
        ERROR_RETURN_NA(-EINVAL);
    }

	rst_value = cb_func->get_eth_reset(transceiver_index);
#ifdef C11_ANNEX_K
	retval = sprintf_s(buf, PAGE_SIZE, "%d\n", rst_value);
#else
	retval = sprintf(buf, "%d\n", rst_value);
#endif


    return retval;
}

static ssize_t s3ip_set_eth_reset(struct kobject *kobj, struct transceiver_attribute *attr, const char *buf, size_t count)
{
    unsigned int rst_value = 0;
    int transceiver_index = 0;
    int retval=0;

    if(cb_func == NULL)
        return -EIO;

    transceiver_index = get_transceiver_index(kobj);
    if(transceiver_index < 0)
    {
        TRANSCEIVER_DEBUG("[%s] Get transceiver index failed.\n", __func__);
        return -ENXIO;
    }

    retval = kstrtoint(buf, 0, &rst_value);
    if(retval != 0)
    {
        return -EINVAL;
    }

    cb_func->set_eth_reset(transceiver_index, rst_value);

    return count;
}

static ssize_t s3ip_get_eth_lpmode(struct kobject *kobj, struct transceiver_attribute *attr, char *buf)
{
    unsigned int lpmode_value = 0;
    int transceiver_index = 0;
    int retval=0;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    transceiver_index = get_transceiver_index(kobj);
    if(transceiver_index < 0)
    {
        TRANSCEIVER_DEBUG("[%s] Get transceiver index failed.\n", __func__);
        ERROR_RETURN_NA(-EINVAL);
    }

    retval = cb_func->get_eth_lpmode(transceiver_index, &lpmode_value);
    if(retval < 0)
    {
        ERROR_RETURN_NA(retval);
    }
#ifdef C11_ANNEX_K
    return sprintf_s(buf, PAGE_SIZE, "%d\n", lpmode_value);
#else
    return sprintf(buf, "%d\n", lpmode_value);
#endif

}

static ssize_t s3ip_set_eth_lpmode(struct kobject *kobj, struct transceiver_attribute *attr, const char *buf, size_t count)
{
    unsigned int lpmode_value = 0;
    int transceiver_index = 0;
    int retval=0;

    if(cb_func == NULL)
        return -EIO;

    transceiver_index = get_transceiver_index(kobj);
    if(transceiver_index < 0)
    {
        TRANSCEIVER_DEBUG("[%s] Get transceiver index failed.\n", __func__);
        return -ENXIO;
    }

    retval = kstrtoint(buf, 0, &lpmode_value);
    if(retval != 0)
    {
        return -EINVAL;
    }

    retval = cb_func->set_eth_lpmode(transceiver_index, lpmode_value);
    if(retval < 0)
    {
        return retval;
    }

    return count;
}

static ssize_t s3ip_get_temp(struct kobject *kobj, struct transceiver_attribute *attr, char *buf)
{
    int transceiver_index = 0;
    long long temp = 0;
    int retval=0;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    transceiver_index = get_transceiver_index(kobj);
    if(transceiver_index < 0)
    {
        TRANSCEIVER_DEBUG("[%s] Get transceiver index failed.\n", __func__);
        ERROR_RETURN_NA(-EINVAL);
    }

    retval = cb_func->get_eth_temp(transceiver_index, &temp);
    if(retval < 0)
    {
        ERROR_RETURN_NA(retval);
    }
#ifdef C11_ANNEX_K
    return sprintf_s(buf, PAGE_SIZE, "%s%lld.%03lld\n", ((temp < 0) ? "-":""), abs(temp/multiplier), abs(temp%multiplier)/100000); // return value foramt: 33.123
#else
    return sprintf(buf, "%s%lld.%03lld\n", ((temp < 0) ? "-":""), abs(temp/multiplier), abs(temp%multiplier)/100000); // return value foramt: 33.123
#endif

}

static ssize_t s3ip_get_eth_present(struct kobject *kobj, struct transceiver_attribute *attr, char *buf)
{
    unsigned int present_value = 0;
    int transceiver_index = 0;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    transceiver_index = get_transceiver_index(kobj);
    if(transceiver_index < 0)
    {
        TRANSCEIVER_DEBUG("[%s] Get transceiver index failed.\n", __func__);
        ERROR_RETURN_NA(-EINVAL);
    }

    present_value = cb_func->get_eth_present(transceiver_index);
#ifdef C11_ANNEX_K
    return sprintf_s(buf, PAGE_SIZE, "%d\n", present_value);
#else
    return sprintf(buf, "%d\n", present_value);
#endif

}

static ssize_t s3ip_get_eth_interrupt(struct kobject *kobj, struct transceiver_attribute *attr, char *buf)
{
    unsigned int interrupt_value = 0;
    int transceiver_index = 0;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    transceiver_index = get_transceiver_index(kobj);
    if(transceiver_index < 0)
    {
        TRANSCEIVER_DEBUG("[%s] Get transceiver index failed.\n", __func__);
        ERROR_RETURN_NA(-EINVAL);
    }

	interrupt_value = cb_func->get_eth_interrupt(transceiver_index);
#ifdef C11_ANNEX_K
	return sprintf_s(buf, PAGE_SIZE, "%d\n", interrupt_value);
#else
	return sprintf(buf, "%d\n", interrupt_value);
#endif

}

static ssize_t s3ip_get_diagnostic(struct kobject *kobj, struct transceiver_attribute *attr, char *buf)
{
    int retval=0;
    int transceiver_index = 0;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    transceiver_index = get_transceiver_index(kobj);
    if(transceiver_index < 0)
    {
        TRANSCEIVER_DEBUG("[%s] Get transceiver index failed.\n", __func__);
        ERROR_RETURN_NA(-EINVAL);
    }

    retval = cb_func->get_eth_diagnostic(transceiver_index, buf);

    if(retval < 0)
    {
        ERROR_RETURN_NA(retval);
    }
    else
    {
        return retval;
    }
}

static ssize_t s3ip_get_rx_los(struct kobject *kobj, struct transceiver_attribute *attr, char *buf)
{
    int transceiver_index = 0;
    unsigned char rx_los_buf[2] = {0};
    unsigned char lane_num = 0;
    unsigned char rx_los = 0;
    int retval=0;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    transceiver_index = get_transceiver_index(kobj);
    if(transceiver_index < 0)
    {
        TRANSCEIVER_DEBUG("[%s] Get transceiver index failed.\n", __func__);
        ERROR_RETURN_NA(-EINVAL);
    }

    retval = cb_func->get_eth_rx_los(transceiver_index, rx_los_buf);
    if(retval < 0)
    {
        ERROR_RETURN_NA(retval);
    }

    lane_num = rx_los_buf[0];
    rx_los = rx_los_buf[1];
#ifdef C11_ANNEX_K
    return sprintf_s(buf, PAGE_SIZE, "0x%02X\n", rx_los);
#else
    return sprintf(buf, "0x%02X\n", rx_los);
#endif

}

static ssize_t s3ip_get_tx_los(struct kobject *kobj, struct transceiver_attribute *attr, char *buf)
{
    int transceiver_index = 0;
    unsigned char tx_los_buf[2] = {0};
    unsigned char lane_num = 0;
    unsigned char tx_los = 0;
    int retval=0;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    transceiver_index = get_transceiver_index(kobj);
    if(transceiver_index < 0)
    {
        TRANSCEIVER_DEBUG("[%s] Get transceiver index failed.\n", __func__);
        ERROR_RETURN_NA(-EINVAL);
    }

    retval = cb_func->get_eth_tx_los(transceiver_index, tx_los_buf);
    if(retval < 0)
    {
        ERROR_RETURN_NA(retval);
    }

    lane_num = tx_los_buf[0];
    tx_los = tx_los_buf[1];
#ifdef C11_ANNEX_K
    return sprintf_s(buf, PAGE_SIZE, "0x%02X\n", tx_los);
#else
    return sprintf(buf, "0x%02X\n", tx_los);
#endif

}

static ssize_t s3ip_get_tx_disable(struct kobject *kobj, struct transceiver_attribute *attr, char *buf)
{
    int transceiver_index = 0;
    unsigned char tx_disable_buf[2] = {0};
    unsigned char lane_num = 0;
    unsigned char tx_disable = 0;
    int retval=0;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    transceiver_index = get_transceiver_index(kobj);
    if(transceiver_index < 0)
    {
        TRANSCEIVER_DEBUG("[%s] Get transceiver index failed.\n", __func__);
        ERROR_RETURN_NA(-EINVAL);
    }

    retval = cb_func->get_eth_tx_disable(transceiver_index, tx_disable_buf);
    if(retval < 0)
    {
        ERROR_RETURN_NA(retval);
    }

    lane_num = tx_disable_buf[0];
    tx_disable = tx_disable_buf[1];
#ifdef C11_ANNEX_K
    return sprintf_s(buf, PAGE_SIZE, "0x%02X\n", tx_disable);
#else
    return sprintf(buf, "0x%02X\n", tx_disable);
#endif

}

static ssize_t s3ip_set_tx_disable(struct kobject *kobj, struct transceiver_attribute *attr, const char *buf, size_t count)
{
    int transceiver_index = 0;
    unsigned int tx_disable_state = 0;
    unsigned char tx_disable = 0;
    int retval=0;

    if(cb_func == NULL)
        return -EIO;

    transceiver_index = get_transceiver_index(kobj);
    if(transceiver_index < 0)
    {
        TRANSCEIVER_DEBUG("[%s] Get transceiver index failed.\n", __func__);
        return -ENXIO;
    }

    retval = kstrtoint(buf, 0, &tx_disable_state);
    if(retval != 0)
    {
        return -EINVAL;
    }

    if(tx_disable_state == 0)
    {
        tx_disable = 0x0;
    }
    else
    {
        tx_disable = 0xFF;
    }

    retval = cb_func->set_eth_tx_disable(transceiver_index, tx_disable);
    if(retval < 0)
    {
        return retval;
    }

    return count;
}

static ssize_t s3ip_get_rx_disable(struct kobject *kobj, struct transceiver_attribute *attr, char *buf)
{
    int transceiver_index = 0;
    unsigned char rx_disable_buf[2] = {0};
    unsigned char lane_num = 0;
    unsigned char rx_disable = 0;
    int retval=0;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    transceiver_index = get_transceiver_index(kobj);
    if(transceiver_index < 0)
    {
        TRANSCEIVER_DEBUG("[%s] Get transceiver index failed.\n", __func__);
        ERROR_RETURN_NA(-EINVAL);
    }

    retval = cb_func->get_eth_rx_disable(transceiver_index, rx_disable_buf);
    if(retval < 0)
    {
        ERROR_RETURN_NA(retval);
    }

    lane_num = rx_disable_buf[0];
    rx_disable = rx_disable_buf[1];
#ifdef C11_ANNEX_K
    return sprintf_s(buf, PAGE_SIZE, "0x%02X\n", rx_disable);
#else
    return sprintf(buf, "0x%02X\n", rx_disable);
#endif

}

static ssize_t s3ip_get_tx_cdr_lol(struct kobject *kobj, struct transceiver_attribute *attr, char *buf)
{
    int transceiver_index = 0;
    unsigned char tx_cdr_lol_buf[2] = {0};
    unsigned char lane_num = 0;
    unsigned char tx_cdr_lol = 0;
    int retval=0;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    transceiver_index = get_transceiver_index(kobj);
    if(transceiver_index < 0)
    {
        TRANSCEIVER_DEBUG("[%s] Get transceiver index failed.\n", __func__);
        ERROR_RETURN_NA(-EINVAL);
    }

    retval = cb_func->get_eth_tx_cdr_lol(transceiver_index, tx_cdr_lol_buf);
    if(retval < 0)
    {
        ERROR_RETURN_NA(retval);
    }

    lane_num = tx_cdr_lol_buf[0];
    tx_cdr_lol = tx_cdr_lol_buf[1];
#ifdef C11_ANNEX_K
    return sprintf_s(buf, PAGE_SIZE, "0x%02X\n", tx_cdr_lol);
#else
    return sprintf(buf, "0x%02X\n", tx_cdr_lol);
#endif

}

static ssize_t s3ip_get_rx_cdr_lol(struct kobject *kobj, struct transceiver_attribute *attr, char *buf)
{
    int transceiver_index = 0;
    unsigned char rx_cdr_lol_buf[2] = {0};
    unsigned char lane_num = 0;
    unsigned char rx_cdr_lol = 0;
    int retval=0;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    transceiver_index = get_transceiver_index(kobj);
    if(transceiver_index < 0)
    {
        TRANSCEIVER_DEBUG("[%s] Get transceiver index failed.\n", __func__);
        ERROR_RETURN_NA(-EINVAL);
    }

    retval = cb_func->get_eth_rx_cdr_lol(transceiver_index, rx_cdr_lol_buf);
    if(retval < 0)
    {
        ERROR_RETURN_NA(retval);
    }

    lane_num = rx_cdr_lol_buf[0];
    rx_cdr_lol = rx_cdr_lol_buf[1];
#ifdef C11_ANNEX_K
    return sprintf_s(buf, PAGE_SIZE, "0x%02X\n", rx_cdr_lol);
#else
    return sprintf(buf, "0x%02X\n", rx_cdr_lol);
#endif

}

static ssize_t s3ip_get_tx_fault(struct kobject *kobj, struct transceiver_attribute *attr, char *buf)
{
    int transceiver_index = 0;
    unsigned char tx_fault_buf[2] = {0};
    unsigned char lane_num = 0;
    unsigned char tx_fault = 0;
    int retval=0;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    transceiver_index = get_transceiver_index(kobj);
    if(transceiver_index < 0)
    {
        TRANSCEIVER_DEBUG("[%s] Get transceiver index failed.\n", __func__);
        ERROR_RETURN_NA(-EINVAL);
    }

    retval = cb_func->get_eth_tx_fault(transceiver_index, tx_fault_buf);
    if(retval < 0)
    {
        ERROR_RETURN_NA(retval);
    }

    lane_num = tx_fault_buf[0];
    tx_fault = tx_fault_buf[1];
#ifdef C11_ANNEX_K
    return sprintf_s(buf, PAGE_SIZE, "0x%02X\n", tx_fault);
#else
    return sprintf(buf, "0x%02X\n", tx_fault);
#endif

}

static ssize_t s3ip_get_module_status(struct kobject *kobj, struct transceiver_attribute *attr, char *buf)
{
    int transceiver_index = 0;
    unsigned char module_status = 0;
    int retval=0;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    transceiver_index = get_transceiver_index(kobj);
    if(transceiver_index < 0)
    {
        TRANSCEIVER_DEBUG("[%s] Get transceiver index failed.\n", __func__);
        ERROR_RETURN_NA(-EINVAL);
    }

    retval = cb_func->get_eth_module_status(transceiver_index, &module_status);
    if(retval < 0)
    {
        ERROR_RETURN_NA(retval);
    }
#ifdef C11_ANNEX_K
    return sprintf_s(buf, PAGE_SIZE, "0x%X\n", module_status);
#else
    return sprintf(buf, "0x%X\n", module_status);
#endif

}

static ssize_t s3ip_get_datapath_status(struct kobject *kobj, struct transceiver_attribute *attr, char *buf)
{
    int transceiver_index = 0;
    unsigned int datapath_status = 0;
    unsigned int datapath_status_buf[2] = {0};
    unsigned char lane_num = 0;
    int retval=0;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    transceiver_index = get_transceiver_index(kobj);
    if(transceiver_index < 0)
    {
        TRANSCEIVER_DEBUG("[%s] Get transceiver index failed.\n", __func__);
        ERROR_RETURN_NA(-EINVAL);
    }

    retval = cb_func->get_eth_datapath_status(transceiver_index, datapath_status_buf);
    if(retval < 0)
    {
        ERROR_RETURN_NA(retval);
    }

    lane_num = datapath_status_buf[0];
    datapath_status = datapath_status_buf[1];
#ifdef C11_ANNEX_K
    return sprintf_s(buf, PAGE_SIZE, "0x%08X\n", datapath_status);
#else
    return sprintf(buf, "0x%08X\n", datapath_status);
#endif

}

static ssize_t s3ip_get_eth_eeprom(struct file *filp, struct kobject *kobj, struct bin_attribute *attr, char *buf, loff_t off, size_t count)
{
    int retval=0;
    int transceiver_index = 0;

    if(cb_func == NULL)
    {
        return -EIO;
    }

    transceiver_index = get_transceiver_index(kobj);
    if(transceiver_index < 0)
    {
        TRANSCEIVER_DEBUG("[%s] Get transceiver index failed.\n", __func__);
        return -EIO;
    }

    retval = cb_func->get_eth_eeprom(transceiver_index, buf, off, count);
    if(retval < 0)
    {
        return -EIO;
    }
    else
    {
        return retval;
    }
}

static ssize_t s3ip_set_eth_eeprom(struct file *filp, struct kobject *kobj, struct bin_attribute *attr, char *buf, loff_t off, size_t count)
{
    int transceiver_index = 0;

    if(cb_func == NULL)
        return -EIO;

    transceiver_index = get_transceiver_index(kobj);
    if(transceiver_index < 0)
    {
        TRANSCEIVER_DEBUG("[%s] Get transceiver index failed.\n", __func__);
        return -ENXIO;
    }

    if(count > ONE_ADDR_EEPROM_SIZE)
    {
        return -EINVAL;
    }

    return cb_func->set_eth_eeprom(transceiver_index, buf, off, count);
}

static char transceiver_temp_type[TRANSCEIVER_TOTAL_NUM] = {0};

static ssize_t s3ip_get_eth_temp_type(struct kobject *kobj, struct transceiver_attribute *attr, char *buf)
{
    int transceiver_index;
    int retval=0;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    transceiver_index = get_transceiver_index(kobj);
    if(transceiver_index < 0)
    {
        TRANSCEIVER_DEBUG("[%s] Get transceiver index failed.\n", __func__);
        ERROR_RETURN_NA(-EINVAL);
    }

    if((transceiver_index >= 1) && (transceiver_index <= 32))
    {
#ifdef C11_ANNEX_K
        retval = sprintf_s(buf, PAGE_SIZE, "%d\n",transceiver_temp_type[transceiver_index-1]);
#else
	    retval = sprintf(buf, "%d\n",transceiver_temp_type[transceiver_index-1]);
#endif

    }else{
        return -EINVAL;
    }

    return retval;
}

static ssize_t s3ip_set_eth_temp_type(struct kobject *kobj, struct transceiver_attribute *attr, const char *buf, size_t count)
{
    unsigned int tempTpye_value = 0;
    int transceiver_index = 0;
    int retval=0;

    if(cb_func == NULL)
        return -EIO;

    transceiver_index = get_transceiver_index(kobj);
    if(transceiver_index < 0)
    {
        TRANSCEIVER_DEBUG("[%s] Get transceiver index failed.\n", __func__);
        return -ENXIO;
    }

    retval = kstrtoint(buf, 0, &tempTpye_value);
    if(retval != 0)
    {
        return -EINVAL;
    }
    if((transceiver_index >= 1) && (transceiver_index <= 32))
    {
        transceiver_temp_type[transceiver_index-1] = tempTpye_value;
    }else{
        return -EINVAL;
    }

    return count;
}

static ssize_t s3ip_get_eth_led_status(struct kobject *kobj, struct transceiver_attribute *attr, char *buf)
{
    unsigned int led_value = 0;
    int transceiver_index = 0;
    int retval=0;

    if(cb_func == NULL)
    {
        ERROR_RETURN_NA(-1);
    }

    transceiver_index = get_transceiver_index(kobj);
    if(transceiver_index < 0)
    {
        TRANSCEIVER_DEBUG("[%s] Get transceiver index failed.\n", __func__);
        ERROR_RETURN_NA(-EINVAL);
    }

	led_value = cb_func->get_eth_led_status(transceiver_index);
#ifdef C11_ANNEX_K
	retval = sprintf_s(buf, PAGE_SIZE, "0x%x\n", led_value);
#else
	retval = sprintf(buf, "0x%x\n", led_value);
#endif


    return retval;
}


static ssize_t s3ip_set_eth_led_status(struct kobject *kobj, struct transceiver_attribute *attr, const char *buf, size_t count)
{
    unsigned int led_value = 0;
    int transceiver_index = 0;
    int retval=0;

    if(cb_func == NULL)
        return -EIO;

    transceiver_index = get_transceiver_index(kobj);
    if(transceiver_index < 0)
    {
        TRANSCEIVER_DEBUG("[%s] Get transceiver index failed.\n", __func__);
        return -ENXIO;
    }

    retval = kstrtoint(buf, 0, &led_value);
    if(retval != 0)
    {
        return -EINVAL;
    }

    cb_func->set_eth_led_status(transceiver_index, led_value);

    return count;
}


static struct transceiver_attribute transceiver_attr[NUM_TRANSCEIVER_ATTR] = {
    [DEBUG]               = {{.name = "debug",             .mode = S_IRUGO | S_IWUSR}, s3ip_debug_help,          s3ip_debug},
    [LOGLEVEL]            = {{.name = "loglevel",          .mode = S_IRUGO | S_IWUSR}, s3ip_get_loglevel,        s3ip_set_loglevel},
    [POWER_ON]            = {{.name = "power_on",          .mode = S_IRUGO | S_IWUSR}, s3ip_get_power_on,        NULL},
    [PRESENT]             = {{.name = "present",           .mode = S_IRUGO},           s3ip_get_present,         NULL},
    [NUM]                 = {{.name = "num",               .mode = S_IRUGO},           s3ip_get_num,             NULL},
    [ETH_POWER_ON]        = {{.name = "power_on",          .mode = S_IRUGO | S_IWUSR}, s3ip_get_eth_power_on,    s3ip_set_eth_power_on},
    [ETH_RESET]           = {{.name = "reset",             .mode = S_IRUGO | S_IWUSR}, s3ip_get_eth_reset,       s3ip_set_eth_reset},
    [ETH_LPMODE]          = {{.name = "lpmode",            .mode = S_IRUGO | S_IWUSR}, s3ip_get_eth_lpmode,      s3ip_set_eth_lpmode},
    [ETH_TEMP]            = {{.name = "temp_input",        .mode = S_IRUGO},           s3ip_get_temp,            NULL},
    [ETH_PRESENT]         = {{.name = "present",           .mode = S_IRUGO},           s3ip_get_eth_present,     NULL},
    [ETH_INTERRUPT]       = {{.name = "interrupt",         .mode = S_IRUGO},           s3ip_get_eth_interrupt,   NULL},
    [ETH_DIAGNOSTIC]      = {{.name = "diagnostic",        .mode = S_IRUGO},           s3ip_get_diagnostic,      NULL},
    [ETH_RX_LOS]          = {{.name = "rx_los",            .mode = S_IRUGO},           s3ip_get_rx_los,          NULL},
    [ETH_TX_LOS]          = {{.name = "tx_los",            .mode = S_IRUGO},           s3ip_get_tx_los,          NULL},
    [ETH_TX_DISABLE]      = {{.name = "tx_disable",        .mode = S_IRUGO | S_IWUSR}, s3ip_get_tx_disable,      s3ip_set_tx_disable},
    [ETH_RX_DISABLE]      = {{.name = "rx_disable",        .mode = S_IRUGO},           s3ip_get_rx_disable,      NULL},
    [ETH_TX_CDR_LOL]      = {{.name = "tx_cdr_lol",        .mode = S_IRUGO},           s3ip_get_tx_cdr_lol,      NULL},
    [ETH_RX_CDR_LOL]      = {{.name = "rx_cdr_lol",        .mode = S_IRUGO},           s3ip_get_rx_cdr_lol,      NULL},
    [ETH_TX_FAULT]        = {{.name = "tx_fault",          .mode = S_IRUGO},           s3ip_get_tx_fault,        NULL},
    [ETH_MODULE_STATUS]   = {{.name = "module_status",     .mode = S_IRUGO},           s3ip_get_module_status,   NULL},
    [ETH_DATAPATH_STATUS] = {{.name = "datapath_status",   .mode = S_IRUGO},           s3ip_get_datapath_status, NULL},
    [ETH_TEMP_TYPE]       = {{.name = "temp_type",         .mode = S_IRUGO | S_IWUSR}, s3ip_get_eth_temp_type,   s3ip_set_eth_temp_type},
    [ETH_LED_STATUS]      = {{.name = "led_status",        .mode = S_IRUGO | S_IWUSR}, s3ip_get_eth_led_status,   s3ip_set_eth_led_status},
#if 0 /* use bin_attribute instead of kobj create add */
    [ETH_EEPROM]    = {{.name = "eeprom",       .mode = S_IRUGO | S_IWUSR}, s3ip_get_eth_eeprom,     s3ip_set_eth_eeprom},
#endif
};

static struct bin_attribute bin_attr;

void s3ip_transceiver_drivers_register(struct transceiver_drivers_t *pfunc)
{
    cb_func = pfunc;

    return;
}
EXPORT_SYMBOL_GPL(s3ip_transceiver_drivers_register);

void s3ip_transceiver_drivers_unregister(void)
{
    cb_func = NULL;

    return;
}
EXPORT_SYMBOL_GPL(s3ip_transceiver_drivers_unregister);

static int __init switch_transceiver_init(void)
{
    int err=0;
    int retval=0;
    int i;
    int transceiver_index;
    char *transceiver_index_str;

    transceiver_index_str = (char *)kzalloc(sizeof(ETH_NAME_STRING)+3, GFP_KERNEL);
    if (!transceiver_index_str)
    {
        TRANSCEIVER_DEBUG( "[%s] Fail to alloc transceiver_index_str memory\n", __func__);
        return -ENOMEM;
    }

    /* init bin attribute for the sysfs eeprom file */
    sysfs_bin_attr_init(bin_attr);
    bin_attr.attr.name = "eeprom";
    bin_attr.attr.mode = (S_IRUGO | S_IWUSR);
    bin_attr.read = s3ip_get_eth_eeprom;
    bin_attr.write = s3ip_set_eth_eeprom;
    bin_attr.size = ONE_ADDR_EEPROM_SIZE;

    /* For s3ip */
    transceiver_kobj = kobject_create_and_add(TRANSCEIVER_NAME_STRING, switch_kobj);
    if(!transceiver_kobj)
    {
        TRANSCEIVER_DEBUG( "[%s]Failed to create 'transceiver'\n", __func__);
        err = -ENOMEM;
        goto sysfs_create_kobject_transceiver_failed;
    }

    for(i=0; i <= NUM; i++)
    {
        TRANSCEIVER_DEBUG( "[%s]sysfs_create_file /transceiver/%s\n", __func__, transceiver_attr[i].attr.name);
        retval = sysfs_create_file(transceiver_kobj, &transceiver_attr[i].attr);
        if(retval)
        {
            printk(KERN_ERR "Failed to create file '%s'\n", transceiver_attr[i].attr.name);
            err = -retval;
            goto sysfs_create_s3ip_attr_failed;
        }
    }

    for(transceiver_index=0; transceiver_index<TRANSCEIVER_TOTAL_NUM; transceiver_index++)
    {
#ifdef C11_ANNEX_K
        if(sprintf_s(transceiver_index_str, sizeof(ETH_NAME_STRING)+3, "%s%d", ETH_NAME_STRING, transceiver_index+1) < 0)
#else
        if(sprintf(transceiver_index_str, "%s%d", ETH_NAME_STRING, transceiver_index+1) < 0)
#endif

        {
            err = -ENOMEM;
            goto sysfs_create_kobject_switch_transceiver_index_failed;
        }
        transceiver_index_kobj[transceiver_index] = kobject_create_and_add(transceiver_index_str, transceiver_kobj);
        if(!transceiver_index_kobj[transceiver_index])
        {
            TRANSCEIVER_DEBUG( "[%s]Failed to create 'transceiver%d'\n", __func__, transceiver_index+1);
            err = -ENOMEM;
            goto sysfs_create_kobject_switch_transceiver_index_failed;
        }

        for(i=ETH_POWER_ON; i < NUM_TRANSCEIVER_ATTR; i++)
        {
            TRANSCEIVER_DEBUG( "[%s]sysfs_create_file /transceiver/eth%d/%s\n", __func__, transceiver_index, transceiver_attr[i].attr.name);
            retval = sysfs_create_file(transceiver_index_kobj[transceiver_index], &transceiver_attr[i].attr);
            if(retval)
            {
                printk(KERN_ERR "Failed to create file '%s'\n", transceiver_attr[i].attr.name);
                err = -retval;
                goto sysfs_create_s3ip_transceiver_attr_failed;
            }
        }

        /* create the sysfs eeprom file */
        TRANSCEIVER_DEBUG( "[%s]sysfs_create_bin_file /transceiver/eth%d/%s\n", __func__, transceiver_index, bin_attr.attr.name);
        retval = sysfs_create_bin_file(transceiver_index_kobj[transceiver_index], &bin_attr);
        if (retval)
        {
            printk(KERN_ERR "Failed to create bin file '%s'\n", bin_attr.attr.name);
            err = -retval;
            goto sysfs_create_s3ip_transceiver_attr_failed;
        }
    }

    kfree(transceiver_index_str);

    return 0;

sysfs_create_s3ip_transceiver_attr_failed:
sysfs_create_kobject_switch_transceiver_index_failed:
    for(i=0; i <= NUM; i++)
        sysfs_remove_file(transceiver_kobj, &transceiver_attr[i].attr);

    for(transceiver_index=0; transceiver_index<TRANSCEIVER_TOTAL_NUM; transceiver_index++)
    {
        if(transceiver_index_kobj[transceiver_index])
        {
            for(i=ETH_POWER_ON; i < NUM_TRANSCEIVER_ATTR; i++)
                sysfs_remove_file(transceiver_index_kobj[transceiver_index], &transceiver_attr[i].attr);

            kobject_put(transceiver_index_kobj[transceiver_index]);
            transceiver_index_kobj[transceiver_index] = NULL;
        }
    }

sysfs_create_s3ip_attr_failed:
    if(transceiver_kobj)
    {
        kobject_put(transceiver_kobj);
        transceiver_kobj = NULL;
    }

sysfs_create_kobject_transceiver_failed:
    kfree(transceiver_index_str);

    return err;
}

static void __exit switch_transceiver_exit(void)
{
    int i;
    int transceiver_index;

    /* For s3ip */
    for(i=0; i <= NUM; i++)
        sysfs_remove_file(transceiver_kobj, &transceiver_attr[i].attr);

    for(transceiver_index=0; transceiver_index<TRANSCEIVER_TOTAL_NUM; transceiver_index++)
    {
        if(transceiver_index_kobj[transceiver_index])
        {
            for(i=ETH_POWER_ON; i < NUM_TRANSCEIVER_ATTR; i++)
            {
                sysfs_remove_file(transceiver_index_kobj[transceiver_index], &transceiver_attr[i].attr);
            }
            sysfs_remove_bin_file(transceiver_index_kobj[transceiver_index], &bin_attr);

            kobject_put(transceiver_index_kobj[transceiver_index]);
            transceiver_index_kobj[transceiver_index] = NULL;
        }
    }

    if(transceiver_kobj)
    {
        kobject_put(transceiver_kobj);
        transceiver_kobj = NULL;
    }

    cb_func = NULL;

    return;
}

MODULE_DESCRIPTION("Switch S3IP TRANSCEIVER Driver");
MODULE_VERSION(SWITCH_S3IP_TRANSCEIVER_VERSION);
MODULE_LICENSE("GPL");

module_init(switch_transceiver_init);
module_exit(switch_transceiver_exit);
