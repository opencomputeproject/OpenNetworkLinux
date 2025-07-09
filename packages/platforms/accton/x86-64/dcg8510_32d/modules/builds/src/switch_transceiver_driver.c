#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/slab.h>

#include "switch_transceiver_driver.h"
#include "switch_optoe.h"
#include "switch_cpld_driver.h"

#define DRVNAME "drv_transceiver_driver"
#define SWITCH_TRANSCEIVER_DRIVER_VERSION "0.0.1"

unsigned int loglevel = 0;
static struct platform_device *drv_transceiver_device;

struct eeprom_parse_value{
    int temperature;
    int voltage;
    int tx_bias[8];
    int tx_power[8];
    int rx_power[8];
};
int port_cpld_addr[2] = {
    0x62,
    0x64
};

unsigned int drv_get_eth_value(unsigned int transceiver_index,int eth_type,unsigned int *bit_mask)
{
    unsigned int cpld_addr = 0, offset = 0,  device_id = 0;
    u8 value = 0;
    if(transceiver_index<=16)
    {
        cpld_addr = port_cpld_addr[0];
        device_id = port_cpld1;
        offset = eth_type+(transceiver_index-1)/8;
        *bit_mask = (transceiver_index-1)%8;
    }
    else if(transceiver_index<=TRANSCEIVER_TOTAL_NUM)
    {
        cpld_addr = port_cpld_addr[1];
        device_id = port_cpld2;
        offset = eth_type+(transceiver_index-17)/8;
        *bit_mask = (transceiver_index-1)%8;
    }
    switch_i2c_cpld_read(device_id, offset, &value);

    return value;
}

unsigned int drv_set_eth_value(unsigned int transceiver_index,int eth_type,u8 value)
{
    unsigned int cpld_addr = 0, offset = 0, bit_mask = 0, device_id = 0;
    unsigned int ret = 0;
    if(transceiver_index<=16)
    {
        cpld_addr = port_cpld_addr[0];
        device_id = port_cpld1;
        offset = eth_type+(transceiver_index-1)/8;
        bit_mask = (transceiver_index-1)%8;
    }
    else if(transceiver_index<=TRANSCEIVER_TOTAL_NUM)
    {
        cpld_addr = port_cpld_addr[1];
        device_id = port_cpld2;
        offset = eth_type+(transceiver_index-17)/8;
        bit_mask = (transceiver_index-1)%8;
    }
    ret = switch_i2c_cpld_write(device_id , offset, value);

    return ret;
}

int calc_temperature(unsigned int transceiver_index, int offset)
{
    int val = 0;
    unsigned char data = 0;

    optoe_bin_read_by_index(transceiver_index, &data, offset, 1);
    val += (data * 100);

    optoe_bin_read_by_index(transceiver_index, &data, offset+1, 1);
    val += ((data * 100)/256);

    return val;
}

int calc_voltage(unsigned int transceiver_index, int offset)
{
    int val = 0;
    unsigned char data = 0;

    optoe_bin_read_by_index(transceiver_index, &data, offset, 1);
    val += (data * 256);

    optoe_bin_read_by_index(transceiver_index, &data, offset+1, 1);
    val += data;

    return val;
}

int calc_bias(unsigned int transceiver_index, int offset)
{
    int val = 0;
    unsigned char data = 0;

    optoe_bin_read_by_index(transceiver_index, &data, offset, 1);
    val += (data * 256);

    optoe_bin_read_by_index(transceiver_index, &data, offset+1, 1);
    val += data;

    val = (val * 2);

    return val;
}

int calc_tx_power(unsigned int transceiver_index, int offset)
{
    int val = 0;
    unsigned char data = 0;

    optoe_bin_read_by_index(transceiver_index, &data, offset, 1);
    val += (data * 256);

    optoe_bin_read_by_index(transceiver_index, &data, offset+1, 1);
    val += data;

    return val;
}

int calc_rx_power(unsigned int transceiver_index, int offset)
{
    int val = 0;
    unsigned char data = 0;

    optoe_bin_read_by_index(transceiver_index, &data, offset, 1);
    val += (data * 256);

    optoe_bin_read_by_index(transceiver_index, &data, offset+1, 1);
    val += data;

    return val;
}

/*parse power temperature voltage tx_bias*/
static ssize_t sfp_eeprom_parse(unsigned int transceiver_index, struct eeprom_parse_value *parse_value)
{
    int i;

    parse_value->temperature = calc_temperature(transceiver_index, SFP_TEMPERATURE_OFFSET);
    parse_value->voltage = calc_voltage(transceiver_index, SFP_VOLTAGE_OFFSET);

    for(i = 0;i < 1;i++){
        parse_value->tx_bias[i] = calc_bias(transceiver_index, SFP_BIAS_OFFSET + (i * 2));
    }
    for(i = 0;i < 1;i++){
        parse_value->tx_power[i] = calc_tx_power(transceiver_index, SFP_TX_POWER_OFFSET + (i * 2));
    }
    for(i = 0;i < 1;i++){
        parse_value->rx_power[i] = calc_rx_power(transceiver_index, SFP_RX_POWER_OFFSET + (i * 2));
    }

    return 0;
}

static ssize_t qsfp_eeprom_parse(unsigned int transceiver_index, struct eeprom_parse_value *parse_value)
{
    int i;

    parse_value->temperature = calc_temperature(transceiver_index, QSFP_TEMPERATURE_OFFSET);
    parse_value->voltage = calc_voltage(transceiver_index, QSFP_VOLTAGE_OFFSET);

    for(i = 0;i < 4;i++){
        parse_value->tx_bias[i] = calc_bias(transceiver_index, QSFP_BIAS_OFFSET + (i * 2));
    }
    for(i = 0;i < 4;i++){
        parse_value->tx_power[i] = calc_tx_power(transceiver_index, QSFP_TX_POWER_OFFSET + (i * 2));
    }
    for(i = 0;i < 4;i++){
        parse_value->rx_power[i] = calc_rx_power(transceiver_index, QSFP_RX_POWER_OFFSET + (i * 2));
    }

    return 0;
}

static ssize_t qsfp_dd_eeprom_parse(unsigned int transceiver_index, struct eeprom_parse_value *parse_value)
{
    int i;

    parse_value->temperature = calc_temperature(transceiver_index, QSFP_DD_TEMPERATURE_OFFSET);
    parse_value->voltage = calc_voltage(transceiver_index, QSFP_DD_VOLTAGE_OFFSET);

    for(i = 0;i < 8;i++){
        parse_value->tx_bias[i] = calc_bias(transceiver_index, QSFP_DD_BIAS_OFFSET + (i * 2));
    }
    for(i = 0;i < 8;i++){
        parse_value->tx_power[i] = calc_tx_power(transceiver_index, QSFP_DD_TX_POWER_OFFSET + (i * 2));
    }
    for(i = 0;i < 8;i++){
        parse_value->rx_power[i] = calc_rx_power(transceiver_index, QSFP_DD_RX_POWER_OFFSET + (i * 2));
    }

    return 0;
}
#if 0
static ssize_t qsfp_detect_power_class(unsigned int transceiver_index, unsigned int *power_class)
{
    int ret = 0;
    unsigned char data = 0;

    ret = optoe_bin_read_by_index(transceiver_index, &data, QSFP_EXT_IDENTIFIER_OFFSET, 1);

    if(ret < 0)
    {
        return ret;
    }

    if(data & QSFP_EXT_IDENTIFIER_POWER_CLASS_8_BITMAP)
    {
        *power_class = 8;
    }
    else if((data & QSFP_EXT_IDENTIFIER_POWER_CLASS_1_4_BITMAP) != QSFP_EXT_IDENTIFIER_POWER_CLASS_1_4_BITMAP)
    {
        *power_class = (((data & QSFP_EXT_IDENTIFIER_POWER_CLASS_1_4_BITMAP) >> 6) + 1);
    }
    else
    {
        *power_class = ((data & QSFP_EXT_IDENTIFIER_POWER_CLASS_5_7_BITMAP) + 4);
    }

    return 0;
}

static ssize_t qsfp_enable_power_class(unsigned int transceiver_index, unsigned int power_class)
{
    int ret = 0;
    unsigned char power_control = 0;

    ret = optoe_bin_read_by_index(transceiver_index, &power_control, QSFP_POWER_CONTROL_OFFSET, 1);
    if(ret < 0)
        return ret;

    switch(power_class)
    {
        case 1:
        case 2:
        case 3:
        case 4:
            power_control &= ~(QSFP_POWER_CONTROL_ENABLE_CLASS_5_7_BIT | QSFP_POWER_CONTROL_ENABLE_CLASS_8_BIT);
            break;
        case 5:
        case 6:
        case 7:
            power_control |= (QSFP_POWER_CONTROL_ENABLE_CLASS_5_7_BIT);
            power_control &= ~(QSFP_POWER_CONTROL_ENABLE_CLASS_8_BIT);
            break;
        case 8:
            power_control |= (QSFP_POWER_CONTROL_ENABLE_CLASS_5_7_BIT | QSFP_POWER_CONTROL_ENABLE_CLASS_8_BIT);
            break;
        default:
            break;
    };

    ret = optoe_bin_write_by_index(transceiver_index, &power_control, QSFP_POWER_CONTROL_OFFSET, 1);
    if(ret < 0)
        return ret;

    return 0;
}
#endif
unsigned int drv_get_eth_diagnostic(unsigned int transceiver_index, char *buf)
{
    int i,num;
    ssize_t ret;
    struct eeprom_parse_value parse_value;
    unsigned char identifier = 0;
    int rv = 0;

    /* Detect the transceiver type */
    rv = optoe_bin_read_by_index(transceiver_index, &identifier, TRANSCEIVER_IDENTIIFIER_OFFSET, 1);
    if(rv < 0)
        return rv;

    if(((identifier == 0x0) || (identifier >= 0x1f)) && (transceiver_index != 56))
    {
        /* If identifier is 0, check the extended identifier */
        rv = optoe_bin_read_by_index(transceiver_index, &identifier, TRANSCEIVER_EXTENDED_IDENTIIFIER_OFFSET, 1);
        if(rv < 0)
            return rv;
    }

    switch(identifier)
    {
        case 0x00:
        case 0x03: /* SFP/SFP+/SFP28 */
            sfp_eeprom_parse(transceiver_index, &parse_value);
            num = 1;
            break;

        case 0x0d: /* QSFP+ or later with SFF8636 or SFF8436 mgmt interface */
        case 0x11: /* QSFP28 or later with SFF8636 mgmt interface */
            qsfp_eeprom_parse(transceiver_index, &parse_value);
            num = 4;
            break;

        case 0x18: /* QSFP-DD */
        case 0x1e: /* QSFP+ or later with CMIS */
            qsfp_dd_eeprom_parse(transceiver_index, &parse_value);
            num = 8;
            break;

        default:
            return -EPERM;
    }

    if(num){
#ifdef C11_ANNEX_K
        ret = sprintf_s(buf, 4096, "temperature :%s%3d.%-2d C \n",((parse_value.temperature < 0) ? "-":""),abs(parse_value.temperature/100),abs(parse_value.temperature%100));
#else
        ret = sprintf(buf, "temperature :%s%3d.%-2d C \n",((parse_value.temperature < 0) ? "-":""),abs(parse_value.temperature/100),abs(parse_value.temperature%100));
#endif
#ifdef C11_ANNEX_K
        //printk(KERN_DEBUG "voltage:%d",parse_value.voltage);
        ret = sprintf_s(buf, 4096, "%svoltage :%3d.%-2d V \n",buf,parse_value.voltage/10000,(parse_value.voltage/10)%1000);
#else
        ret = sprintf(buf, "%svoltage :%3d.%-2d V \n",buf,parse_value.voltage/10000,(parse_value.voltage/10)%1000);
#endif
        for(i = 0;i < num;i++){
#ifdef C11_ANNEX_K
            ret = sprintf_s(buf, 4096, "%stxbias%d :%3d.%-2d mA \n",buf,i+1,parse_value.tx_bias[i]/1000,parse_value.tx_bias[i]%1000);
#else
		    ret = sprintf(buf, "%stxbias%d :%3d.%-2d mA \n",buf,i+1,parse_value.tx_bias[i]/1000,parse_value.tx_bias[i]%1000);
#endif
        }
        for(i = 0;i < num;i++){
#ifdef C11_ANNEX_K
           ret = sprintf_s(buf, 4096, "%stx%d_power :%3d.%-2d mW \n",buf,i+1,parse_value.tx_power[i]/10000,parse_value.tx_power[i]%10000);
#else
		   ret = sprintf(buf, "%stx%d_power :%3d.%-2d mW \n",buf,i+1,parse_value.tx_power[i]/10000,parse_value.tx_power[i]%10000);
#endif
		}
        for(i = 0;i < num;i++){
#ifdef C11_ANNEX_K
           ret = sprintf_s(buf, 4096, "%srx%d_power :%3d.%-2d mW \n",buf,i+1,parse_value.rx_power[i]/10000,parse_value.rx_power[i]%10000);
#else
		   ret = sprintf(buf, "%srx%d_power :%3d.%-2d mW \n",buf,i+1,parse_value.rx_power[i]/10000,parse_value.rx_power[i]%10000);
#endif
		}

        return ret;

    }
#ifdef C11_ANNEX_K
    ret = sprintf_s(buf, 4096, "N/A\n");
#else
    ret = sprintf(buf, "N/A\n");
#endif
    return ret;
}

EXPORT_SYMBOL_GPL(drv_get_eth_diagnostic);

unsigned int drv_get_eth_power_on (unsigned int transceiver_index)
{
    unsigned int power_on_value = 0,bit_mask = 0;

    power_on_value = drv_get_eth_value(transceiver_index,QSFP_POWER_ON1,&bit_mask);
    if(power_on_value < 0){
        return -1;
    }
    power_on_value = (power_on_value >> bit_mask) & 1;
    //reg present bit in reg: 1: enable,   0: disable.
    //return                  1: present,       0: not present
    return (power_on_value == 0) ? 1 : 0;
}
unsigned int drv_set_eth_power_on(unsigned int transceiver_index, unsigned int pwr_value)
{
    int tmp_value = 0,ret = 0;
    unsigned int bit_mask = 0;

    tmp_value = drv_get_eth_value(transceiver_index,QSFP_POWER_ON1,&bit_mask);
    if(tmp_value < 0){
        return -1;
    }
    tmp_value = (tmp_value & ~(1 << bit_mask));
    /*set register*/
    tmp_value = tmp_value | (!pwr_value << bit_mask);
    ret = drv_set_eth_value(transceiver_index,QSFP_POWER_ON1,tmp_value);

    return ret;

}

unsigned int drv_get_eth_reset(unsigned int transceiver_index)
{
    int rst_value = 0;
    unsigned int bit_mask = 0;

    rst_value = drv_get_eth_value(transceiver_index,QSFP_RST1,&bit_mask);
    if(rst_value < 0){
        return -1;
    }
    rst_value = (rst_value >> bit_mask) & 1;
    /*qsfp 0 set reset*/

    return (rst_value == 0) ? 1 : 0;
}

unsigned int drv_set_eth_reset(unsigned int transceiver_index, unsigned int rst_value)
{
    int tmp_value = 0,ret = 0;
    unsigned int bit_mask = 0;

    rst_value = rst_value == 0 ? 1 : 0;
    tmp_value = drv_get_eth_value(transceiver_index,QSFP_RST1,&bit_mask);
    if(tmp_value < 0){
        return -1;
    }
    tmp_value = (tmp_value & ~(1 << bit_mask));
    /*set register*/
    tmp_value = tmp_value | (rst_value << bit_mask);
    ret = drv_set_eth_value(transceiver_index,QSFP_RST1,tmp_value);

    return ret;
}

int drv_get_eth_lpmode(unsigned int transceiver_index, unsigned int *lpmode_value)
{

    unsigned char identifier = 0;
    unsigned char power_control = 0;
    unsigned int lp_mode = 0;
    unsigned int bit_mask = 0;
    int tmp_value = 0;
    int rv = 0;

    tmp_value = drv_get_eth_value(transceiver_index,QSFP_LPWN1,&bit_mask);
    if(tmp_value < 0){
        return tmp_value;
    }
    tmp_value = (tmp_value >> bit_mask) & 1;
    *lpmode_value = tmp_value ? 1:0;
    return rv;

    /* Detect the transceiver type */
    rv = optoe_bin_read_by_index(transceiver_index, &identifier, TRANSCEIVER_IDENTIIFIER_OFFSET, 1);
    if(rv < 0)
        return rv;

    if((identifier == 0x0) || (identifier >= 0x1f))
    {
        /* If identifier is 0, check the extended identifier */
        rv = optoe_bin_read_by_index(transceiver_index, &identifier, TRANSCEIVER_EXTENDED_IDENTIIFIER_OFFSET, 1);
        if(rv < 0)
            return rv;
    }

    switch(identifier)
    {
        case 0x0d: /* QSFP+ or later with SFF8636 or SFF8436 mgmt interface */
        case 0x11: /* QSFP28 or later with SFF8636 mgmt interface */
            rv = optoe_bin_read_by_index(transceiver_index, &power_control, QSFP_POWER_CONTROL_OFFSET, 1);
            if(rv < 0)
                return rv;

            if(power_control & QSFP_POWER_CONTROL_POWER_OVERRIDE_BIT)
            {
                /* The LP mode is depending on the Power Set value when the override is true. */
                lp_mode = (power_control & QSFP_POWER_CONTROL_POWER_SET_BIT) ? 1 : 0;
            }
            else
            {
                /* The LP mode is on by default */
                lp_mode = 1;
            }
            break;
        case 0x18: /* QSFP-DD */
        case 0x1b: /* DSFP+ or later with CMIS */
        case 0x1e: /* QSFP+ or later with CMIS */
            rv = optoe_bin_read_by_index(transceiver_index, &power_control, QSFP_DD_MODULE_GLOBAL_CONTROL_OFFSET, 1);
            if(rv < 0)
                return rv;

            if(power_control & QSFP_DD_MODULE_GLOBAL_CONTROL_FORCE_LOW_POWER_BIT)
            {
                /* Force low power is enabled */
                lp_mode = 1;
            }
            else
            {
                /* The LP mode is depending on the Low Power value when the Force Low Power is disabled. */
                lp_mode = (power_control & QSFP_DD_MODULE_GLOBAL_CONTROL_LOW_POWER_BIT) ? 1 : 0;
            }
            break;
        default:
            /* Fail to recognize the transceiver type, regard it as default low power. */
            lp_mode = 1;
            break;
    }

    *lpmode_value = (lp_mode ? 1 : 0);
    return rv;
}
EXPORT_SYMBOL_GPL(drv_get_eth_lpmode);

int drv_set_eth_lpmode(unsigned int transceiver_index, unsigned int lpmode_value)
{
    unsigned char identifier = 0;
    unsigned char power_control = 0;
    int tmp_value = 0;
    int rv = 0;
    unsigned int bit_mask = 0;


    tmp_value = drv_get_eth_value(transceiver_index,QSFP_LPWN1,&bit_mask);
    tmp_value = (tmp_value & ~(1 << bit_mask));
    /*set register*/
    tmp_value = tmp_value | (lpmode_value << bit_mask);
    rv = drv_set_eth_value(transceiver_index,QSFP_LPWN1,tmp_value);

    return rv;

    /* Detect the transceiver type */
    rv = optoe_bin_read_by_index(transceiver_index, &identifier, TRANSCEIVER_IDENTIIFIER_OFFSET, 1);
    if(rv < 0)
        return rv;

    if((identifier == 0x0) || (identifier >= 0x1f))
    {
        /* If identifier is 0, check the extended identifier */
        rv = optoe_bin_read_by_index(transceiver_index, &identifier, TRANSCEIVER_EXTENDED_IDENTIIFIER_OFFSET, 1);
        if(rv < 0)
            return rv;
    }

    switch(identifier)
    {
        case 0x0d: /* QSFP+ or later with SFF8636 or SFF8436 mgmt interface */
        case 0x11: /* QSFP28 or later with SFF8636 mgmt interface */
            rv = optoe_bin_read_by_index(transceiver_index, &power_control, QSFP_POWER_CONTROL_OFFSET, 1);
            if(rv < 0)
                return rv;

            if(!(power_control & QSFP_POWER_CONTROL_POWER_OVERRIDE_BIT))
            {
                /* Setup the Power Override bit */
                power_control |= QSFP_POWER_CONTROL_POWER_OVERRIDE_BIT;
            }

            if(lpmode_value)
            {
                /* Setup the Power Set bit for the Low power mode */
                if(!(power_control & QSFP_POWER_CONTROL_POWER_SET_BIT))
                {
                    power_control |= QSFP_POWER_CONTROL_POWER_SET_BIT;
                }
            }
            else
            {
                /* Clear the Power Set bit for the High power mode */
                if(power_control & QSFP_POWER_CONTROL_POWER_SET_BIT)
                {
                    power_control &= ~(QSFP_POWER_CONTROL_POWER_SET_BIT);
                }
            }
            rv = optoe_bin_write_by_index(transceiver_index, &power_control, QSFP_POWER_CONTROL_OFFSET, 1);
            if(rv < 0)
                return rv;

            break;

        case 0x18: /* QSFP-DD */
        case 0x1b: /* DSFP+ or later with CMIS */
        case 0x1e: /* QSFP+ or later with CMIS */
            rv = optoe_bin_read_by_index(transceiver_index, &power_control, QSFP_DD_MODULE_GLOBAL_CONTROL_OFFSET, 1);
            printk("port%d value 0x%x\n",transceiver_index,power_control);
            if(rv < 0)
                return rv;

            if(power_control & QSFP_DD_MODULE_GLOBAL_CONTROL_FORCE_LOW_POWER_BIT)
            {
                /* Clear the Force Low Power bit */
                power_control &= ~QSFP_DD_MODULE_GLOBAL_CONTROL_FORCE_LOW_POWER_BIT;
            }

            if(lpmode_value)
            {
                /* Setup the Low Power bit for the Low power mode */
                if(!(power_control & QSFP_DD_MODULE_GLOBAL_CONTROL_LOW_POWER_BIT))
                {
                    power_control |= QSFP_DD_MODULE_GLOBAL_CONTROL_LOW_POWER_BIT;
                }
            }
            else
            {
                /* Clear the Low Power bit for the High power mode */
                if(power_control & QSFP_DD_MODULE_GLOBAL_CONTROL_LOW_POWER_BIT)
                {
                    power_control &= ~(QSFP_DD_MODULE_GLOBAL_CONTROL_LOW_POWER_BIT);
                }
            }
            rv = optoe_bin_write_by_index(transceiver_index, &power_control, QSFP_DD_MODULE_GLOBAL_CONTROL_OFFSET, 1);
            if(rv < 0)
                return rv;

            break;

        default:
            break;
    }

    return rv;
}
EXPORT_SYMBOL_GPL(drv_set_eth_lpmode);

unsigned int drv_get_eth_present(unsigned int transceiver_index)
{
    unsigned int present_value = 0;
    unsigned int bit_mask = 0;
    present_value = drv_get_eth_value(transceiver_index,QSFP_PRESENT1,&bit_mask);
    if(present_value < 0){
        return -1;
    }
    present_value = (present_value >> bit_mask) & 1;
    //reg present bit in reg: 1: not present,   0: present.
    //return                  1: present,       0: not present
    return (present_value == 0) ? 1 : 0;
}

unsigned int drv_get_eth_present_history(unsigned int transceiver_index)
{
    return 0;
}

unsigned int drv_get_eth_led_status(unsigned int transceiver_index)
{
    u8 led_value = 0;
    unsigned int cpld_addr = 0, offset = 0,  device_id = 0;
    if(transceiver_index<=16)
    {
        cpld_addr = port_cpld_addr[0];
        device_id = port_cpld1;
        offset = QSFP_LED+(transceiver_index-1);
    }
    else if(transceiver_index<=TRANSCEIVER_TOTAL_NUM)
    {
        cpld_addr = port_cpld_addr[1];
        device_id = port_cpld2;
        offset = QSFP_LED+(transceiver_index-17);
    }
    switch_i2c_cpld_read(device_id, offset, &led_value);

    if(led_value < 0){
        return -1;
    }

    return led_value;
}

unsigned int drv_set_eth_led_status(unsigned int transceiver_index, unsigned int led_value)
{
    unsigned int cpld_addr = 0, offset = 0,device_id = 0;
    unsigned int ret = 0;
    if(transceiver_index<=16)
    {
        cpld_addr = port_cpld_addr[0];
        device_id = port_cpld1;
        offset = QSFP_LED+(transceiver_index-1);
    }
    else if(transceiver_index<=TRANSCEIVER_TOTAL_NUM)
    {
        cpld_addr = port_cpld_addr[1];
        device_id = port_cpld2;
        offset = QSFP_LED+(transceiver_index-17);
    }
    ret = switch_i2c_cpld_write(device_id , offset, led_value);

    return ret;
}

unsigned int drv_get_eth_interrupt(unsigned int transceiver_index)
{
    unsigned int interrupt_value = 0;
    unsigned int bit_mask = 0;

    interrupt_value = drv_get_eth_value(transceiver_index,QSFP_INTR1,&bit_mask);
    if(interrupt_value < 0){
        return -1;
    }
    interrupt_value = (interrupt_value >> bit_mask) & 1;
    //reg interrupt bit in reg: 1: block,   0: pass.
    //return                  1:pass ,      0: block
    return (interrupt_value == 0) ? 1 : 0;
}

int drv_get_eth_eeprom(unsigned int transceiver_index, char *buf, loff_t off, size_t len)
{
    return optoe_bin_read_by_index(transceiver_index, buf, off, len);
}

int drv_set_eth_eeprom(unsigned int transceiver_index, char *buf, loff_t off, size_t len)
{
    return optoe_bin_write_by_index(transceiver_index, buf, off, len);
}

int drv_get_eth_temp(unsigned int transceiver_index, long long *temp)
{
    unsigned char identifier = 0;
    unsigned char temp_buffer[2];
    unsigned char ddm_type = 0;
    short temperature = 0;
    unsigned int offset = 0;
    int rv = 0;

    /* Detect the transceiver type */
    rv = optoe_bin_read_by_index(transceiver_index, &identifier, TRANSCEIVER_IDENTIIFIER_OFFSET, 1);
    if(rv < 0)
        return rv;

    if(((identifier == 0x0) || (identifier >= 0x1f)) && (transceiver_index != 33))
    {
        /* If identifier is 0, check the extended identifier */
        rv = optoe_bin_read_by_index(transceiver_index, &identifier, TRANSCEIVER_EXTENDED_IDENTIIFIER_OFFSET, 1);
        if(rv < 0)
            return rv;
    }

    switch(identifier)
    {
        case 0x00:
        case 0x03: /* SFP/SFP+/SFP28 */
            offset = SFP_TEMPERATURE_OFFSET;
            break;

        case 0x0d: /* QSFP+ or later with SFF8636 or SFF8436 mgmt interface */
        case 0x11: /* QSFP28 or later with SFF8636 mgmt interface */
            offset = QSFP_TEMPERATURE_OFFSET;
            break;

        case 0x18: /* QSFP-DD */
        case 0x1e: /* QSFP+ or later with CMIS */
            offset = QSFP_DD_TEMPERATURE_OFFSET;
            break;

        default:
            return -EPERM;
    }

    if((transceiver_index == 33) && (offset == SFP_TEMPERATURE_OFFSET))
    {
        rv = optoe_bin_read_by_index(transceiver_index, &ddm_type, SFP_DDM_TYPE_OFFSET, 1);
        if(rv < 0)
            return rv;

        if(!(ddm_type & SFP_DDM_TYPE_MONITOR_IMPLEMENTED_OFFSET))
        {
            /* Return -1 if the SFP does not support DDM */
            return -1;
        }
    }

    rv = optoe_bin_read_by_index(transceiver_index, temp_buffer, offset, 2);
	temperature = (temp_buffer[1] + (temp_buffer[0] << 8));
	/* 16-bit signed twos complement value in increments of 1/256 (0.00390625) degrees Celsius */
	*temp = (((long long)temperature >> 8) * 100000000) + ((long long)(temperature & 0xFF) * 390625);
    return rv;
}

int drv_get_eth_rx_los(unsigned int transceiver_index, unsigned char *rx_los)
{
    unsigned char identifier = 0;
    unsigned char buffer;
    unsigned int offset = 0;
    unsigned char lane_num = 0;
    unsigned int mask = 0;
    unsigned int bit_offset = 0;
    int rv = 0;

    /* Detect the transceiver type */
    rv = optoe_bin_read_by_index(transceiver_index, &identifier, TRANSCEIVER_IDENTIIFIER_OFFSET, 1);
    if(rv < 0)
        return rv;

    if((identifier == 0x0) || (identifier >= 0x1f))
    {
        /* If identifier is 0, check the extended identifier */
        rv = optoe_bin_read_by_index(transceiver_index, &identifier, TRANSCEIVER_EXTENDED_IDENTIIFIER_OFFSET, 1);
        if(rv < 0)
            return rv;
    }

    switch(identifier)
    {
        case 0x00:
        case 0x03: /* SFP/SFP+/SFP28 */
            offset = SFP_RX_LOS_OFFSET;
            mask = SFP_RX_LOS_MASK;
            bit_offset = SFP_RX_LOS_BIT_OFFSET;
            lane_num = SFP_LANE_NUM;
            break;

        case 0x0d: /* QSFP+ or later with SFF8636 or SFF8436 mgmt interface */
        case 0x11: /* QSFP28 or later with SFF8636 mgmt interface */
            offset = QSFP_RX_LOS_OFFSET;
            mask = QSFP_RX_LOS_MASK;
            bit_offset = QSFP_RX_LOS_BIT_OFFSET;
            lane_num = QSFP_LANE_NUM;
            break;

        case 0x18: /* QSFP-DD */
        case 0x1e: /* QSFP+ or later with CMIS */
            offset = QSFP_DD_RX_LOS_OFFSET;
            mask = QSFP_DD_RX_LOS_MASK;
            bit_offset = QSFP_DD_RX_LOS_BIT_OFFSET;
            rv = optoe_bin_read_by_index(transceiver_index, &buffer, QSFP_DD_LANE_NUM_OFFSET, 1);
            if(rv < 0)
                return rv;
            lane_num = (buffer >> QSFP_DD_LANE_NUM_MEDIALANE_BIT_OFFSET) & QSFP_DD_LANE_NUM_MEDIALANE_MASK;
            break;

        default:
            return -EPERM;
    }

    rv = optoe_bin_read_by_index(transceiver_index, &buffer, offset, 1);
    rx_los[0] = lane_num;
    rx_los[1] = (buffer >> bit_offset) & mask;

    return rv;
}

int drv_get_eth_tx_los(unsigned int transceiver_index, unsigned char *tx_los)
{
    unsigned char identifier = 0;
    unsigned char buffer;
    unsigned int offset = 0;
    unsigned char lane_num = 0;
    unsigned int mask = 0;
    unsigned int bit_offset = 0;
    int rv = 0;

    /* Detect the transceiver type */
    rv = optoe_bin_read_by_index(transceiver_index, &identifier, TRANSCEIVER_IDENTIIFIER_OFFSET, 1);
    if(rv < 0)
        return rv;

    if((identifier == 0x0) || (identifier >= 0x1f))
    {
        /* If identifier is 0, check the extended identifier */
        rv = optoe_bin_read_by_index(transceiver_index, &identifier, TRANSCEIVER_EXTENDED_IDENTIIFIER_OFFSET, 1);
        if(rv < 0)
            return rv;
    }

    switch(identifier)
    {
        case 0x00:
        case 0x03: /* SFP/SFP+/SFP28 */
            return -1;

        case 0x0d: /* QSFP+ or later with SFF8636 or SFF8436 mgmt interface */
        case 0x11: /* QSFP28 or later with SFF8636 mgmt interface */
            offset = QSFP_TX_LOS_OFFSET;
            mask = QSFP_TX_LOS_MASK;
            bit_offset = QSFP_TX_LOS_BIT_OFFSET;
            lane_num = QSFP_LANE_NUM;
            break;

        case 0x18: /* QSFP-DD */
        case 0x1e: /* QSFP+ or later with CMIS */
            offset = QSFP_DD_TX_LOS_OFFSET;
            mask = QSFP_DD_TX_LOS_MASK;
            bit_offset = QSFP_DD_TX_LOS_BIT_OFFSET;
            rv = optoe_bin_read_by_index(transceiver_index, &buffer, QSFP_DD_LANE_NUM_OFFSET, 1);
            if(rv < 0)
                return rv;
            lane_num = (buffer >> QSFP_DD_LANE_NUM_MEDIALANE_BIT_OFFSET) & QSFP_DD_LANE_NUM_MEDIALANE_MASK;
            break;

        default:
            return -EPERM;
    }

    rv = optoe_bin_read_by_index(transceiver_index, &buffer, offset, 1);
    tx_los[0] = lane_num;
    tx_los[1] = (buffer >> bit_offset) & mask;

    return rv;
}

int drv_get_eth_tx_disable(unsigned int transceiver_index, unsigned char *tx_disable)
{
    unsigned char identifier = 0;
    unsigned char buffer;
    unsigned int offset = 0;
    unsigned char lane_num = 0;
    unsigned int mask = 0;
    unsigned int bit_offset = 0;
    int rv = 0;

    /* Detect the transceiver type */
    rv = optoe_bin_read_by_index(transceiver_index, &identifier, TRANSCEIVER_IDENTIIFIER_OFFSET, 1);
    if(rv < 0)
        return rv;

    if((identifier == 0x0) || (identifier >= 0x1f))
    {
        /* If identifier is 0, check the extended identifier */
        rv = optoe_bin_read_by_index(transceiver_index, &identifier, TRANSCEIVER_EXTENDED_IDENTIIFIER_OFFSET, 1);
        if(rv < 0)
            return rv;
    }

    switch(identifier)
    {
        case 0x00:
        case 0x03: /* SFP/SFP+/SFP28 */
            offset = SFP_TX_DISABLE_OFFSET;
            mask = SFP_TX_DISABLE_MASK;
            bit_offset = SFP_TX_DISABLE_BIT_OFFSET;
            lane_num = SFP_LANE_NUM;
            break;

        case 0x0d: /* QSFP+ or later with SFF8636 or SFF8436 mgmt interface */
        case 0x11: /* QSFP28 or later with SFF8636 mgmt interface */
            offset = QSFP_TX_DISABLE_OFFSET;
            mask = QSFP_TX_DISABLE_MASK;
            bit_offset = QSFP_TX_DISABLE_BIT_OFFSET;
            lane_num = QSFP_LANE_NUM;
            break;

        case 0x18: /* QSFP-DD */
        case 0x1e: /* QSFP+ or later with CMIS */
            offset = QSFP_DD_TX_DISABLE_OFFSET;
            mask = QSFP_DD_TX_DISABLE_MASK;
            bit_offset = QSFP_DD_TX_DISABLE_BIT_OFFSET;
            rv = optoe_bin_read_by_index(transceiver_index, &buffer, QSFP_DD_LANE_NUM_OFFSET, 1);
            if(rv < 0)
                return rv;
            lane_num = (buffer >> QSFP_DD_LANE_NUM_MEDIALANE_BIT_OFFSET) & QSFP_DD_LANE_NUM_MEDIALANE_MASK;
            break;

        default:
            return -EPERM;
    }

    rv = optoe_bin_read_by_index(transceiver_index, &buffer, offset, 1);
    tx_disable[0] = lane_num;
    tx_disable[1] = (buffer >> bit_offset) & mask;

    return rv;
}

int drv_set_eth_tx_disable(unsigned int transceiver_index, unsigned int tx_disable)
{
    unsigned char identifier = 0;
    unsigned char temp_data = 0;
    unsigned char bitmap = 0;
    unsigned int offset = 0;
    int rv = 0;

    /* Detect the transceiver type */
    rv = optoe_bin_read_by_index(transceiver_index, &identifier, TRANSCEIVER_IDENTIIFIER_OFFSET, 1);
    if(rv < 0)
        return rv;

    if((identifier == 0x0) || (identifier >= 0x1f))
    {
        /* If identifier is 0, check the extended identifier */
        rv = optoe_bin_read_by_index(transceiver_index, &identifier, TRANSCEIVER_EXTENDED_IDENTIIFIER_OFFSET, 1);
        if(rv < 0)
            return rv;
    }

    switch(identifier)
    {
        case 0x00:
        case 0x03: /* SFP/SFP+/SFP28 */
            offset = SFP_TX_DISABLE_OFFSET;
            bitmap = SFP_SOFT_TX_DISABLE_BITMAP;
            break;

        case 0x0d: /* QSFP+ or later with SFF8636 or SFF8436 mgmt interface */
        case 0x11: /* QSFP28 or later with SFF8636 mgmt interface */
            offset = QSFP_TX_DISABLE_OFFSET;
            bitmap = QSFP_TX_DISABLE_BITMAP;
            break;

        case 0x18: /* QSFP-DD */
        case 0x1e: /* QSFP+ or later with CMIS */
            offset = QSFP_DD_TX_DISABLE_OFFSET;
            bitmap = QSFP_DD_TX_DISABLE_BITMAP;
            break;

        default:
            return -EPERM;
    }

    rv = optoe_bin_read_by_index(transceiver_index, &temp_data, offset, 1);
    if(rv < 0)
        return rv;

    temp_data &= ~bitmap;
    temp_data |= (bitmap & tx_disable);
    rv = optoe_bin_write_by_index(transceiver_index, &temp_data, offset, 1);
    if(rv < 0)
        return rv;

    return rv;
}

int drv_get_eth_rx_disable(unsigned int transceiver_index, unsigned char *rx_disable)
{
    unsigned char identifier = 0;
    unsigned char buffer;
    unsigned int offset = 0;
    unsigned char lane_num = 0;
    unsigned int mask = 0;
    unsigned int bit_offset = 0;
    int rv = 0;

    /* Detect the transceiver type */
    rv = optoe_bin_read_by_index(transceiver_index, &identifier, TRANSCEIVER_IDENTIIFIER_OFFSET, 1);
    if(rv < 0)
        return rv;

    if((identifier == 0x0) || (identifier >= 0x1f))
    {
        /* If identifier is 0, check the extended identifier */
        rv = optoe_bin_read_by_index(transceiver_index, &identifier, TRANSCEIVER_EXTENDED_IDENTIIFIER_OFFSET, 1);
        if(rv < 0)
            return rv;
    }

    switch(identifier)
    {
        case 0x00:
        case 0x03: /* SFP/SFP+/SFP28 */
            return -1;

        case 0x0d: /* QSFP+ or later with SFF8636 or SFF8436 mgmt interface */
        case 0x11: /* QSFP28 or later with SFF8636 mgmt interface */
            offset = QSFP_RX_DISABLE_OFFSET;
            mask = QSFP_RX_DISABLE_MASK;
            bit_offset = QSFP_RX_DISABLE_BIT_OFFSET;
            lane_num = QSFP_LANE_NUM;
            break;

        case 0x18: /* QSFP-DD */
        case 0x1e: /* QSFP+ or later with CMIS */
            offset = QSFP_DD_RX_DISABLE_OFFSET;
            mask = QSFP_DD_RX_DISABLE_MASK;
            bit_offset = QSFP_DD_RX_DISABLE_BIT_OFFSET;
            rv = optoe_bin_read_by_index(transceiver_index, &buffer, QSFP_DD_LANE_NUM_OFFSET, 1);
            if(rv < 0)
                return rv;
            lane_num = (buffer >> QSFP_DD_LANE_NUM_MEDIALANE_BIT_OFFSET) & QSFP_DD_LANE_NUM_MEDIALANE_MASK;
            break;

        default:
            return -EPERM;
    }

    rv = optoe_bin_read_by_index(transceiver_index, &buffer, offset, 1);
    rx_disable[0] = lane_num;
    rx_disable[1] = (buffer >> bit_offset) & mask;

    return rv;
}

int drv_get_eth_tx_cdr_lol(unsigned int transceiver_index, unsigned char *tx_cdr_lol)
{
    unsigned char identifier = 0;
    unsigned char buffer;
    unsigned int offset = 0;
    unsigned char lane_num = 0;
    unsigned int mask = 0;
    unsigned int bit_offset = 0;
    int rv = 0;

    /* Detect the transceiver type */
    rv = optoe_bin_read_by_index(transceiver_index, &identifier, TRANSCEIVER_IDENTIIFIER_OFFSET, 1);
    if(rv < 0)
        return rv;

    if((identifier == 0x0) || (identifier >= 0x1f))
    {
        /* If identifier is 0, check the extended identifier */
        rv = optoe_bin_read_by_index(transceiver_index, &identifier, TRANSCEIVER_EXTENDED_IDENTIIFIER_OFFSET, 1);
        if(rv < 0)
            return rv;
    }

    switch(identifier)
    {
        case 0x00:
        case 0x03: /* SFP/SFP+/SFP28 */
            offset = SFP_TX_CDR_LOL_OFFSET;
            mask = SFP_TX_CDR_LOL_MASK;
            bit_offset = SFP_TX_CDR_LOL_BIT_OFFSET;
            lane_num = SFP_LANE_NUM;
            break;

        case 0x0d: /* QSFP+ or later with SFF8636 or SFF8436 mgmt interface */
        case 0x11: /* QSFP28 or later with SFF8636 mgmt interface */
            offset = QSFP_TX_CDR_LOL_OFFSET;
            mask = QSFP_TX_CDR_LOL_MASK;
            bit_offset = QSFP_TX_CDR_LOL_BIT_OFFSET;
            lane_num = QSFP_LANE_NUM;
            break;

        case 0x18: /* QSFP-DD */
        case 0x1e: /* QSFP+ or later with CMIS */
            offset = QSFP_DD_TX_CDR_LOL_OFFSET;
            mask = QSFP_DD_TX_CDR_LOL_MASK;
            bit_offset = QSFP_DD_TX_CDR_LOL_BIT_OFFSET;
            rv = optoe_bin_read_by_index(transceiver_index, &buffer, QSFP_DD_LANE_NUM_OFFSET, 1);
            if(rv < 0)
                return rv;
            lane_num = (buffer >> QSFP_DD_LANE_NUM_MEDIALANE_BIT_OFFSET) & QSFP_DD_LANE_NUM_MEDIALANE_MASK;
            break;

        default:
            return -EPERM;
    }

    rv = optoe_bin_read_by_index(transceiver_index, &buffer, offset, 1);
    tx_cdr_lol[0] = lane_num;
    tx_cdr_lol[1] = (buffer >> bit_offset) & mask;

    return rv;
}

int drv_get_eth_rx_cdr_lol(unsigned int transceiver_index, unsigned char *rx_cdr_lol)
{
    unsigned char identifier = 0;
    unsigned char buffer;
    unsigned int offset = 0;
    unsigned char lane_num = 0;
    unsigned int mask = 0;
    unsigned int bit_offset = 0;
    int rv = 0;

    /* Detect the transceiver type */
    rv = optoe_bin_read_by_index(transceiver_index, &identifier, TRANSCEIVER_IDENTIIFIER_OFFSET, 1);
    if(rv < 0)
        return rv;

    if((identifier == 0x0) || (identifier >= 0x1f))
    {
        /* If identifier is 0, check the extended identifier */
        rv = optoe_bin_read_by_index(transceiver_index, &identifier, TRANSCEIVER_EXTENDED_IDENTIIFIER_OFFSET, 1);
        if(rv < 0)
            return rv;
    }

    switch(identifier)
    {
        case 0x00:
        case 0x03: /* SFP/SFP+/SFP28 */
            offset = SFP_RX_CDR_LOL_OFFSET;
            mask = SFP_RX_CDR_LOL_MASK;
            bit_offset = SFP_RX_CDR_LOL_BIT_OFFSET;
            lane_num = SFP_LANE_NUM;
            break;

        case 0x0d: /* QSFP+ or later with SFF8636 or SFF8436 mgmt interface */
        case 0x11: /* QSFP28 or later with SFF8636 mgmt interface */
            offset = QSFP_RX_CDR_LOL_OFFSET;
            mask = QSFP_RX_CDR_LOL_MASK;
            bit_offset = QSFP_RX_CDR_LOL_BIT_OFFSET;
            lane_num = QSFP_LANE_NUM;
            break;

        case 0x18: /* QSFP-DD */
        case 0x1e: /* QSFP+ or later with CMIS */
            offset = QSFP_DD_RX_CDR_LOL_OFFSET;
            mask = QSFP_DD_RX_CDR_LOL_MASK;
            bit_offset = QSFP_DD_RX_CDR_LOL_BIT_OFFSET;
            rv = optoe_bin_read_by_index(transceiver_index, &buffer, QSFP_DD_LANE_NUM_OFFSET, 1);
            if(rv < 0)
                return rv;
            lane_num = (buffer >> QSFP_DD_LANE_NUM_MEDIALANE_BIT_OFFSET) & QSFP_DD_LANE_NUM_MEDIALANE_MASK;
            break;

        default:
            return -EPERM;
    }

    rv = optoe_bin_read_by_index(transceiver_index, &buffer, offset, 1);
    rx_cdr_lol[0] = lane_num;
    rx_cdr_lol[1] = (buffer >> bit_offset) & mask;

    return rv;
}

int drv_get_eth_tx_fault(unsigned int transceiver_index, unsigned char *tx_fault)
{
    unsigned char identifier = 0;
    unsigned char buffer;
    unsigned int offset = 0;
    unsigned char lane_num = 0;
    unsigned int mask = 0;
    unsigned int bit_offset = 0;
    int rv = 0;

    /* Detect the transceiver type */
    rv = optoe_bin_read_by_index(transceiver_index, &identifier, TRANSCEIVER_IDENTIIFIER_OFFSET, 1);
    if(rv < 0)
        return rv;

    if((identifier == 0x0) || (identifier >= 0x1f))
    {
        /* If identifier is 0, check the extended identifier */
        rv = optoe_bin_read_by_index(transceiver_index, &identifier, TRANSCEIVER_EXTENDED_IDENTIIFIER_OFFSET, 1);
        if(rv < 0)
            return rv;
    }

    switch(identifier)
    {
        case 0x00:
        case 0x03: /* SFP/SFP+/SFP28 */
            offset = SFP_TX_FAULT_OFFSET;
            mask = SFP_TX_FAULT_MASK;
            bit_offset = SFP_TX_FAULT_BIT_OFFSET;
            lane_num = SFP_LANE_NUM;
            break;

        case 0x0d: /* QSFP+ or later with SFF8636 or SFF8436 mgmt interface */
        case 0x11: /* QSFP28 or later with SFF8636 mgmt interface */
            offset = QSFP_TX_FAULT_OFFSET;
            mask = QSFP_TX_FAULT_MASK;
            bit_offset = QSFP_TX_FAULT_BIT_OFFSET;
            lane_num = QSFP_LANE_NUM;
            break;

        case 0x18: /* QSFP-DD */
        case 0x1e: /* QSFP+ or later with CMIS */
            offset = QSFP_DD_TX_FAULT_OFFSET;
            mask = QSFP_DD_TX_FAULT_MASK;
            bit_offset = QSFP_DD_TX_FAULT_BIT_OFFSET;
            rv = optoe_bin_read_by_index(transceiver_index, &buffer, QSFP_DD_LANE_NUM_OFFSET, 1);
            if(rv < 0)
                return rv;
            lane_num = (buffer >> QSFP_DD_LANE_NUM_MEDIALANE_BIT_OFFSET) & QSFP_DD_LANE_NUM_MEDIALANE_MASK;
            break;

        default:
            return -EPERM;
    }

    rv = optoe_bin_read_by_index(transceiver_index, &buffer, offset, 1);
    tx_fault[0] = lane_num;
    tx_fault[1] = (buffer >> bit_offset) & mask;

    return rv;
}

int drv_get_eth_module_status(unsigned int transceiver_index, unsigned char *module_status)
{
    unsigned char identifier = 0;
    unsigned char buffer;
    unsigned int offset = 0;
    int rv = 0;

    /* Detect the transceiver type */
    rv = optoe_bin_read_by_index(transceiver_index, &identifier, TRANSCEIVER_IDENTIIFIER_OFFSET, 1);
    if(rv < 0)
        return rv;

    if((identifier == 0x0) || (identifier >= 0x1f))
    {
        /* If identifier is 0, check the extended identifier */
        rv = optoe_bin_read_by_index(transceiver_index, &identifier, TRANSCEIVER_EXTENDED_IDENTIIFIER_OFFSET, 1);
        if(rv < 0)
            return rv;
    }

    switch(identifier)
    {
        case 0x00:
        case 0x03: /* SFP/SFP+/SFP28 */
            /* not supported */
            return -1;

        case 0x0d: /* QSFP+ or later with SFF8636 or SFF8436 mgmt interface */
        case 0x11: /* QSFP28 or later with SFF8636 mgmt interface */
            /* not supported */
            return -1;

        case 0x18: /* QSFP-DD */
        case 0x1e: /* QSFP+ or later with CMIS */
            offset = QSFP_DD_MODULE_STATUS_OFFSET;
            break;

        default:
            return -EPERM;
    }

    rv = optoe_bin_read_by_index(transceiver_index, &buffer, offset, 1);
    *module_status = ((buffer & QSFP_DD_MODULE_STATUS_BITMAP) >> 1);

    return rv;
}


int drv_get_eth_bus_status(unsigned int transceiver_index, unsigned char *bus_status)
{
    unsigned char identifier = 0;
    int rv = 0;

    /* Detect the transceiver type */
    rv = optoe_bin_read_by_index(transceiver_index, &identifier, TRANSCEIVER_IDENTIIFIER_OFFSET, 1);
    if(rv < 0)
        return rv;

    if((identifier == 0x0) || (identifier >= 0x1f))
    {
        /* If identifier is 0, check the extended identifier */
        rv = optoe_bin_read_by_index(transceiver_index, &identifier, TRANSCEIVER_EXTENDED_IDENTIIFIER_OFFSET, 1);
        if(rv < 0)
            return rv;
    }
    *bus_status = identifier;

    return rv;
}

int drv_get_eth_datapath_status(unsigned int transceiver_index, unsigned int *datapath_status)
{
    unsigned char identifier = 0;
    unsigned int buffer = 0;
    unsigned int offset = 0;
    unsigned char lane_num = 0;
    unsigned char lane_num_buf = 0;
    size_t len = 0;
    int rv = 0;

    /* Detect the transceiver type */
    rv = optoe_bin_read_by_index(transceiver_index, &identifier, TRANSCEIVER_IDENTIIFIER_OFFSET, 1);
    if(rv < 0)
        return rv;

    if((identifier == 0x0) || (identifier >= 0x1f))
    {
        /* If identifier is 0, check the extended identifier */
        rv = optoe_bin_read_by_index(transceiver_index, &identifier, TRANSCEIVER_EXTENDED_IDENTIIFIER_OFFSET, 1);
        if(rv < 0)
            return rv;
    }

    switch(identifier)
    {
        case 0x00:
        case 0x03: /* SFP/SFP+/SFP28 */
            /* not supported */
            return -1;

        case 0x0d: /* QSFP+ or later with SFF8636 or SFF8436 mgmt interface */
        case 0x11: /* QSFP28 or later with SFF8636 mgmt interface */
            /* not supported */
            return -1;

        case 0x18: /* QSFP-DD */
        case 0x1e: /* QSFP+ or later with CMIS */
            offset = QSFP_DD_DATAPATH_STATUS_OFFSET;
            len = QSFP_DD_DATAPATH_STATUS_WIDTH;
            rv = optoe_bin_read_by_index(transceiver_index, &lane_num_buf, QSFP_DD_LANE_NUM_OFFSET, 1);
            if(rv < 0)
                return rv;
            lane_num = (lane_num_buf >> QSFP_DD_LANE_NUM_MEDIALANE_BIT_OFFSET) & QSFP_DD_LANE_NUM_MEDIALANE_MASK;
            break;

        default:
            return -EPERM;
    }

    rv = optoe_bin_read_by_index(transceiver_index, (char*)&buffer, offset, len);
    datapath_status[0] = lane_num;
    datapath_status[1] = buffer;

    return rv;
}

void drv_get_loglevel(long *lev)
{
    *lev = (long)loglevel;
    
    return;
}

void drv_set_loglevel(long lev)
{
    loglevel = (unsigned int)lev;
    
    return;
}

ssize_t drv_debug_help(char *buf)
{
#ifdef C11_ANNEX_K
    return sprintf_s(buf, PAGE_SIZE,
        "For eeprom node, use i2cget/i2cset/i2cdump -f -y <bus_num> 0x50 to debug.\n"
        "For other nodes, use i2cget/i2cset/i2cdump -f -y <bus_num> <reg_addr> to debug.\n"
        "<bus_num>: 601-632 for port1-port32\n"
        "<reg_addr>: address is presented as below, and each register has 8 bits to present status of 8 ports.\n"
        "sysname                debug cmd\n"
        "port1     eeprom       i2cdump -f -y 601 0x50\n"
        "port2     eeprom       i2cdump -f -y 602 0x50\n"
        "...\n"
        "port32    eeprom       i2cdump -f -y 632 0x50\n"
        "port1-8   present      i2cget  -f -y 633 0x62 0x20\n"
        "port9-16  present      i2cget  -f -y 633 0x62 0x21\n"
        "port17-24 present      i2cget  -f -y 634 0x64 0x20\n"
        "port25-32 present      i2cget  -f -y 634 0x64 0x21\n"
        "port1-8   reset        i2cget  -f -y 633 0x62 0x38\n"
        "port9-16  reset        i2cget  -f -y 633 0x62 0x39\n"
        "port17-24 reset        i2cget  -f -y 634 0x64 0x38\n"
        "port25-32 reset        i2cget  -f -y 634 0x64 0x39\n"
        "port1-8   lpmode       i2cget  -f -y 633 0x62 0x30\n"
        "port9-16  lpmode       i2cget  -f -y 633 0x62 0x31\n"
        "port17-24 lpmode       i2cget  -f -y 634 0x64 0x30\n"
        "port25-32 lpmode       i2cget  -f -y 634 0x64 0x31\n"
        "port1-8   power_on     i2cget  -f -y 633 0x62 0x40\n"
        "port9-16  power_on     i2cget  -f -y 633 0x62 0x41\n"
        "port17-24 power_on     i2cget  -f -y 634 0x64 0x40\n"
        "port25-32 power_on     i2cget  -f -y 634 0x64 0x41\n"
        "port1-8   interrupt    i2cget  -f -y 633 0x62 0x10\n"
        "port9-16  interrupt    i2cget  -f -y 633 0x62 0x11\n"
        "port17-24 interrupt    i2cget  -f -y 634 0x64 0x10\n"
        "port25-32 interrupt    i2cget  -f -y 634 0x64 0x11\n");
#else
    return sprintf(buf,
        "For eeprom node, use i2cget/i2cset/i2cdump -f -y <bus_num> 0x50 to debug.\n"
        "For other nodes, use i2cget/i2cset/i2cdump -f -y <bus_num> <reg_addr> to debug.\n"
        "<bus_num>: 601-632 for port1-port32\n"
        "<reg_addr>: address is presented as below, and each register has 8 bits to present status of 8 ports.\n"
        "sysname                debug cmd\n"
        "port1     eeprom       i2cdump -f -y 601 0x50\n"
        "port2     eeprom       i2cdump -f -y 602 0x50\n"
        "...\n"
        "port32    eeprom       i2cdump -f -y 632 0x50\n"
        "port1-8   present      i2cget  -f -y 633 0x62 0x20\n"
        "port9-16  present      i2cget  -f -y 633 0x62 0x21\n"
        "port17-24 present      i2cget  -f -y 634 0x64 0x20\n"
        "port25-32 present      i2cget  -f -y 634 0x64 0x21\n"
        "port1-8   reset        i2cget  -f -y 633 0x62 0x38\n"
        "port9-16  reset        i2cget  -f -y 633 0x62 0x39\n"
        "port17-24 reset        i2cget  -f -y 634 0x64 0x38\n"
        "port25-32 reset        i2cget  -f -y 634 0x64 0x39\n"
        "port1-8   lpmode       i2cget  -f -y 633 0x62 0x30\n"
        "port9-16  lpmode       i2cget  -f -y 633 0x62 0x31\n"
        "port17-24 lpmode       i2cget  -f -y 634 0x64 0x30\n"
        "port25-32 lpmode       i2cget  -f -y 634 0x64 0x31\n"
        "port1-8   power_on     i2cget  -f -y 633 0x62 0x40\n"
        "port9-16  power_on     i2cget  -f -y 633 0x62 0x41\n"
        "port17-24 power_on     i2cget  -f -y 634 0x64 0x40\n"
        "port25-32 power_on     i2cget  -f -y 634 0x64 0x41\n"
        "port1-8   interrupt    i2cget  -f -y 633 0x62 0x10\n"
        "port9-16  interrupt    i2cget  -f -y 633 0x62 0x11\n"
        "port17-24 interrupt    i2cget  -f -y 634 0x64 0x10\n"
        "port25-32 interrupt    i2cget  -f -y 634 0x64 0x11\n");
#endif

}

ssize_t drv_debug(const char *buf, int count)
{
    return 0;
}

// For s3ip
static struct transceiver_drivers_t pfunc = {
    .get_eth_diagnostic = drv_get_eth_diagnostic,
    .get_eth_power_on = drv_get_eth_power_on,
    .set_eth_power_on = drv_set_eth_power_on,
    .get_eth_reset = drv_get_eth_reset,
    .set_eth_reset = drv_set_eth_reset,
    .get_eth_lpmode = drv_get_eth_lpmode,
    .set_eth_lpmode = drv_set_eth_lpmode,
    .get_eth_present = drv_get_eth_present,
    .get_eth_present_history = drv_get_eth_present_history,
    .get_eth_interrupt = drv_get_eth_interrupt,
    .get_eth_eeprom = drv_get_eth_eeprom,
    .set_eth_eeprom = drv_set_eth_eeprom,
    .get_eth_temp = drv_get_eth_temp,
    .get_eth_led_status = drv_get_eth_led_status,
    .set_eth_led_status = drv_set_eth_led_status,
    .get_eth_tx_los = drv_get_eth_tx_los,
    .get_eth_rx_los = drv_get_eth_rx_los,
    .get_eth_tx_disable = drv_get_eth_tx_disable,
    .set_eth_tx_disable = drv_set_eth_tx_disable,
    .get_eth_rx_disable = drv_get_eth_rx_disable,
    .get_eth_tx_cdr_lol = drv_get_eth_tx_cdr_lol,
    .get_eth_rx_cdr_lol = drv_get_eth_rx_cdr_lol,
    .get_eth_tx_fault = drv_get_eth_tx_fault,
    .get_eth_module_status = drv_get_eth_module_status,
    .get_eth_datapath_status = drv_get_eth_datapath_status,
    .get_eth_bus_status = drv_get_eth_bus_status,
    .get_loglevel = drv_get_loglevel,
    .set_loglevel = drv_set_loglevel,
    .debug_help = drv_debug_help,
};

static int drv_transceiver_probe(struct platform_device *pdev)
{   
    s3ip_transceiver_drivers_register(&pfunc);

    return 0;
}

static int drv_transceiver_remove(struct platform_device *pdev)
{
    s3ip_transceiver_drivers_unregister();

    return 0;
}

static struct platform_driver drv_transceiver_driver = {
    .driver = {
        .name   = DRVNAME,
        .owner = THIS_MODULE,
    },
    .probe      = drv_transceiver_probe,
    .remove     = drv_transceiver_remove,
};

static int __init drv_transceiver_init(void)
{
    int err=0;
    int retval=0;
    
    drv_transceiver_device = platform_device_alloc(DRVNAME, 0);
    if(!drv_transceiver_device)
    {
        TRANSCEIVER_DEBUG("%s(#%d): platform_device_alloc fail\n", __func__, __LINE__);
        return -ENOMEM;
    }
    
    retval = platform_device_add(drv_transceiver_device);
    if(retval)
    {
        TRANSCEIVER_DEBUG("%s(#%d): platform_device_add failed\n", __func__, __LINE__);
        err = -retval;
        goto dev_add_failed;
    }

    retval = platform_driver_register(&drv_transceiver_driver);
    if(retval)
    {
        TRANSCEIVER_DEBUG("%s(#%d): platform_driver_register failed(%d)\n", __func__, __LINE__, err);
        err = -retval;
        goto dev_reg_failed;
    }

    return 0;
    
dev_reg_failed:
    platform_device_unregister(drv_transceiver_device);
    return err;

dev_add_failed:
    platform_device_put(drv_transceiver_device);
    return err;
}

static void __exit drv_transceiver_exit(void)
{
    platform_driver_unregister(&drv_transceiver_driver);
    platform_device_unregister(drv_transceiver_device);

    return;
}

MODULE_DESCRIPTION("S3IP Transceiver Driver");
MODULE_VERSION(SWITCH_TRANSCEIVER_DRIVER_VERSION);
MODULE_LICENSE("GPL");

module_init(drv_transceiver_init);
module_exit(drv_transceiver_exit);
