#include "delta_agc7008s_common.h"

unsigned char port_cpld0_reg_addr;
unsigned char system_cpld_reg_addr;

/* --------------- SWPLD - start --------------- */
static struct platform_device system_cpld_device = {
    .name = "delta-agc7008s-system-cpld",
    .id   = 0,
    .dev  = {
        .platform_data = agc7008s_system_cpld_platform_data,
        .release       = device_release
    },
};

static struct platform_device port_cpld0_device = {
    .name = "delta-agc7008s-port-cpld0",
    .id   = 0,
    .dev  = {
        .platform_data = agc7008s_port_cpld0_platform_data,
        .release       = device_release
    },
};

static ssize_t get_system_cpld_reg(struct device *dev, struct device_attribute *dev_attr, char *buf)
{
    int ret;
    int mask;
    int value;
    int cmd_data_len;
    char note[ATTRIBUTE_NOTE_SIZE];
    uint8_t cmd_data[CMD_DATA_SIZE] = {0};
    uint8_t get_cmd;
    struct sensor_device_attribute *attr = to_sensor_dev_attr(dev_attr);

    cmd_data_len = sizeof(cmd_data);
    get_cmd = CMD_GETDATA;
    cmd_data[0] = BMC_BUS_4;
    cmd_data[1] = SYSTEM_CPLD_ADDR;
    cmd_data[3] = 1;
    mask  = attribute_data[attr->index].mask;

    switch (attr->index) {
        case SYSTEM_CPLD_REG_ADDR:
            return sprintf(buf, "0x%02x\n", system_cpld_reg_addr);
        case SYSTEM_CPLD_REG_VALUE:
            cmd_data[2] = system_cpld_reg_addr;
            ret = dni_bmc_cmd(get_cmd, cmd_data, cmd_data_len);
            ret = ret & 0xff;
            return sprintf(buf, "0x%02x\n", ret);
        case SYSTEM_CPLD_VER ... MAC_VOL:
            cmd_data[2] = attribute_data[attr->index].reg;
            value = dni_bmc_cmd(get_cmd, cmd_data, cmd_data_len);
            sprintf(note, "\n%s\n",attribute_data[attr->index].note);
            value = (value & mask);
            break;
        default:
            return sprintf(buf, "%d not found", attr->index);
    }

    switch (mask) {
        case 0xFF:
            return sprintf(buf, "0x%02x%s", value, note);
        case 0x0F:
        case 0x07:
        case 0x03:
            return sprintf(buf, "0x%01x%s", value, note);
        case 0xF0:
            value = value >> 4;
            return sprintf(buf, "0x%01x%s", value, note);
        default :
            value = value >> dni_log2(mask);
            return sprintf(buf, "%d%s", value, note);
    }
}

static ssize_t set_system_cpld_reg(struct device *dev, struct device_attribute *dev_attr,
             const char *buf, size_t count)
{
    int err;
    int value;
    int set_data;
    int cmd_data_len;
    uint8_t cmd_data[CMD_DATA_SIZE] = {0};
    uint8_t set_cmd;
    uint8_t get_cmd;
    unsigned long set_data_ul;
    unsigned char mask;
    unsigned char mask_out;
    struct sensor_device_attribute *attr = to_sensor_dev_attr(dev_attr);

    cmd_data_len = sizeof(cmd_data);
    set_cmd = CMD_SETDATA;
    get_cmd = CMD_GETDATA;

    err = kstrtoul(buf, 0, &set_data_ul);
    if (err){
        return err;
    }

    set_data = (int)set_data_ul;
    if (set_data > 0xff) {
        printk(KERN_ALERT "address out of range (0x00-0xFF)\n");
        return count;
    }

    switch (attr->index) {
        case SYSTEM_CPLD_REG_ADDR:
            system_cpld_reg_addr = set_data;
            return count;
        case SYSTEM_CPLD_REG_VALUE:
            cmd_data[0] = BMC_BUS_4;
            cmd_data[1] = SYSTEM_CPLD_ADDR;
            cmd_data[2] = system_cpld_reg_addr;
            cmd_data[3] = set_data;
            dni_bmc_cmd(set_cmd, cmd_data, cmd_data_len);
            return count;
        case SYSTEM_CPLD_VER ... MAC_VOL:
            cmd_data[0] = BMC_BUS_4;
            cmd_data[1] = SYSTEM_CPLD_ADDR;
            cmd_data[2] = attribute_data[attr->index].reg;
            cmd_data[3] = 1;
            value = dni_bmc_cmd(get_cmd, cmd_data, cmd_data_len);
            mask  = attribute_data[attr->index].mask;
            mask_out = value & ~(mask);
            break;
        default:
            return sprintf((char *)buf, "%d not found", attr->index);
    }

    switch (mask) {
        case 0xFF:
            set_data = mask_out | (set_data & mask);
            break;
        case 0x0F:
        case 0x07:
        case 0x03:
            set_data = mask_out | (set_data & mask);
            break;
        case 0xF0:
            set_data = set_data << 4;
            set_data = mask_out | (set_data & mask);
            break;
        default :
            set_data = mask_out | (set_data << dni_log2(mask) );
    }

    cmd_data[3] = set_data;
    value = dni_bmc_cmd(get_cmd, cmd_data, cmd_data_len);
    return count;
}

static ssize_t get_port_cpld0_reg(struct device *dev, struct device_attribute *dev_attr, char *buf)
{
    int mask;
    int value;
    int cmd_data_len;
    char note[ATTRIBUTE_NOTE_SIZE];
    uint8_t cmd_data[CMD_DATA_SIZE] = {0};
    uint8_t set_cmd;
    uint8_t get_cmd;
    struct sensor_device_attribute *attr = to_sensor_dev_attr(dev_attr);

    cmd_data_len = sizeof(cmd_data);
    set_cmd = CMD_SETDATA;
    get_cmd = CMD_GETDATA;

    cmd_data[0] = BMC_BUS_4;

    cmd_data[1] = PAC9528_1_ADDR;
    cmd_data[2] = PAC9528_MUX_REG;
    cmd_data[3] = 0x01;

    dni_bmc_cmd(set_cmd, cmd_data, cmd_data_len);

    cmd_data[1] = PORT_CPLD0_ADDR;
    cmd_data[3] = 1;
    mask  = attribute_data[attr->index].mask;
    sprintf(note, "\n%s\n",attribute_data[attr->index].note);

    switch (attr->index) {
        case PORT_CPLD0_REG_ADDR:
            return sprintf(buf, "0x%02x\n", port_cpld0_reg_addr);
        case PORT_CPLD0_REG_VALUE:
            cmd_data[2] = port_cpld0_reg_addr;
            value = dni_bmc_cmd(get_cmd, cmd_data, cmd_data_len);
            value = value & 0xff;
            return sprintf(buf, "0x%02x\n", value);
        case PORT_CPLD0_VER ... SFP_RX_LOSS_MASK:
            cmd_data[2] = attribute_data[attr->index].reg;
            value = dni_bmc_cmd(get_cmd, cmd_data, cmd_data_len);
            value = value & mask;
            break;
        default:
            return sprintf(buf, "%d not found", attr->index);
    }
    
    switch (mask) {
        case 0xFF:
            return sprintf(buf, "0x%02x%s", value, note);
        case 0x0F:
        case 0x03:
            return sprintf(buf, "0x%01x%s", value, note);
        default :
            value = value >> dni_log2(mask);
            return sprintf(buf, "%d%s", value, note);
    }
}

static ssize_t set_port_cpld0_reg(struct device *dev, struct device_attribute *dev_attr,
             const char *buf, size_t count)
{
    int err;
    int value;
    int set_data;
    int cmd_data_len;
    uint8_t cmd_data[CMD_DATA_SIZE] = {0};
    uint8_t set_cmd;
    uint8_t get_cmd;
    unsigned long set_data_ul;
    unsigned char mask;
    unsigned char mask_out;
    struct sensor_device_attribute *attr = to_sensor_dev_attr(dev_attr);

    cmd_data_len = sizeof(cmd_data);
    set_cmd = CMD_SETDATA;
    get_cmd = CMD_GETDATA;


    cmd_data[0] = BMC_BUS_4;
    cmd_data[1] = PAC9528_1_ADDR;
    cmd_data[2] = PAC9528_MUX_REG;
    cmd_data[3] = 0x01;

    dni_bmc_cmd(set_cmd, cmd_data, cmd_data_len);
   
    err = kstrtoul(buf, 0, &set_data_ul);
    if (err){
        return err;
    }

    set_data = (int)set_data_ul;
    if (set_data > 0xff){
        printk(KERN_ALERT "address out of range (0x00-0xFF)\n");
        return count;
    }

    switch (attr->index) {
        case PORT_CPLD0_REG_ADDR:
            port_cpld0_reg_addr = set_data;
            return count;
        case PORT_CPLD0_REG_VALUE:
            cmd_data[0] = BMC_BUS_4;
            cmd_data[1] = PORT_CPLD0_ADDR;
            cmd_data[2] = port_cpld0_reg_addr;
            cmd_data[3] = set_data;
            dni_bmc_cmd(set_cmd, cmd_data, cmd_data_len);
            return count;
        case PORT_CPLD0_VER ... SFP_RX_LOSS_MASK:
            cmd_data[0] = BMC_BUS_4;
            cmd_data[1] = PORT_CPLD0_ADDR;
            cmd_data[2] = attribute_data[attr->index].reg;
            cmd_data[3] = 1;
            value = dni_bmc_cmd(get_cmd, cmd_data, cmd_data_len);
            mask  = attribute_data[attr->index].mask;
            mask_out = value & ~(mask);
            break;
        default :
            return sprintf((char *)buf, "%d not found", attr->index);
    }
   
    switch (mask) {
        case 0xFF:
        case 0x0F:
        case 0x03:
            set_data = mask_out | (set_data & mask);
            break;
        default :
            set_data = mask_out | (set_data << dni_log2(mask) );
    }
    cmd_data[3] = set_data;
    dni_bmc_cmd(set_cmd, cmd_data, cmd_data_len);
    return count;
}

static SENSOR_DEVICE_ATTR(system_cpld_reg_addr,     S_IRUGO | S_IWUSR, get_system_cpld_reg, set_system_cpld_reg, SYSTEM_CPLD_REG_ADDR);
static SENSOR_DEVICE_ATTR(system_cpld_reg_value,    S_IRUGO | S_IWUSR, get_system_cpld_reg, set_system_cpld_reg, SYSTEM_CPLD_REG_VALUE);
static SENSOR_DEVICE_ATTR(system_cpld_ver,          S_IRUGO,           get_system_cpld_reg, NULL,         SYSTEM_CPLD_VER);
static SENSOR_DEVICE_ATTR(mb_pwr_recycle,    S_IRUGO | S_IWUSR, get_system_cpld_reg, set_system_cpld_reg, MB_PWR_RECYCLE);
static SENSOR_DEVICE_ATTR(cpu_pwr_recycle,   S_IRUGO | S_IWUSR, get_system_cpld_reg, set_system_cpld_reg, CPU_PWR_RECYCLE);
static SENSOR_DEVICE_ATTR(mb_pwr_good,       S_IRUGO,           get_system_cpld_reg, NULL,         MB_PWR_GOOD);
static SENSOR_DEVICE_ATTR(mac_rst,           S_IRUGO | S_IWUSR, get_system_cpld_reg, set_system_cpld_reg, MAC_RST);
static SENSOR_DEVICE_ATTR(phy_1g_rst,        S_IRUGO | S_IWUSR, get_system_cpld_reg, set_system_cpld_reg, PHY_1G_RST);
static SENSOR_DEVICE_ATTR(phy_10g_rst,       S_IRUGO | S_IWUSR, get_system_cpld_reg, set_system_cpld_reg, PHY_10G_RST);
static SENSOR_DEVICE_ATTR(phy_25g_rst,       S_IRUGO | S_IWUSR, get_system_cpld_reg, set_system_cpld_reg, PHY_25G_RST);
static SENSOR_DEVICE_ATTR(phy_100g_rst,      S_IRUGO | S_IWUSR, get_system_cpld_reg, set_system_cpld_reg, PHY_100G_RST);
static SENSOR_DEVICE_ATTR(system_cpld0_rst,         S_IRUGO | S_IWUSR, get_system_cpld_reg, set_system_cpld_reg, CPLD0_RST);
static SENSOR_DEVICE_ATTR(pca9548_0_rst,     S_IRUGO | S_IWUSR, get_system_cpld_reg, set_system_cpld_reg, PCA9548_0_RST);
static SENSOR_DEVICE_ATTR(pca9548_0_1_rst,   S_IRUGO | S_IWUSR, get_system_cpld_reg, set_system_cpld_reg, PCA9548_0_1_RST);
static SENSOR_DEVICE_ATTR(pca9548_0_2_rst,   S_IRUGO | S_IWUSR, get_system_cpld_reg, set_system_cpld_reg, PCA9548_0_2_RST);
static SENSOR_DEVICE_ATTR(pca9548_2_rst,     S_IRUGO | S_IWUSR, get_system_cpld_reg, set_system_cpld_reg, PCA9548_2_RST);
static SENSOR_DEVICE_ATTR(pca9546_3_rst,     S_IRUGO | S_IWUSR, get_system_cpld_reg, set_system_cpld_reg, PCA9546_3_RST);
static SENSOR_DEVICE_ATTR(pca9546_4_rst,     S_IRUGO | S_IWUSR, get_system_cpld_reg, set_system_cpld_reg, PCA9546_4_RST);
static SENSOR_DEVICE_ATTR(gps_uart_rst,      S_IRUGO | S_IWUSR, get_system_cpld_reg, set_system_cpld_reg, GPS_UART_RST);
static SENSOR_DEVICE_ATTR(tod_uart_rst,      S_IRUGO | S_IWUSR, get_system_cpld_reg, set_system_cpld_reg, TOD_UART_RST);
static SENSOR_DEVICE_ATTR(gps_mod_rst,       S_IRUGO | S_IWUSR, get_system_cpld_reg, set_system_cpld_reg, GPS_MOD_RST);
static SENSOR_DEVICE_ATTR(zl30364_rst,       S_IRUGO | S_IWUSR, get_system_cpld_reg, set_system_cpld_reg, ZL30364_RST);
static SENSOR_DEVICE_ATTR(chip_81000_rst,    S_IRUGO | S_IWUSR, get_system_cpld_reg, set_system_cpld_reg, CHIP_81000_RST);
static SENSOR_DEVICE_ATTR(nic_i210_rst,      S_IRUGO | S_IWUSR, get_system_cpld_reg, set_system_cpld_reg, NIC_I210_RST);
static SENSOR_DEVICE_ATTR(usb_hub_rst,       S_IRUGO | S_IWUSR, get_system_cpld_reg, set_system_cpld_reg, USB_HUB_RST);
static SENSOR_DEVICE_ATTR(button_int,        S_IRUGO,           get_system_cpld_reg, NULL,         BUTTON_INT);
static SENSOR_DEVICE_ATTR(watchdog_int,      S_IRUGO,           get_system_cpld_reg, NULL,         WATCHDOG_INT);
static SENSOR_DEVICE_ATTR(mac_int,           S_IRUGO,           get_system_cpld_reg, NULL,         MAC_INT);
static SENSOR_DEVICE_ATTR(chip_81000_int,    S_IRUGO,           get_system_cpld_reg, NULL,         CHIP_81000_INT);
static SENSOR_DEVICE_ATTR(chip_54140_int,    S_IRUGO,           get_system_cpld_reg, NULL,         CHIP_54140_INT);
static SENSOR_DEVICE_ATTR(gbic_mod_prst,     S_IRUGO,           get_system_cpld_reg, NULL,         GBIC_MOD_PRST);
static SENSOR_DEVICE_ATTR(gbic_mod_int,      S_IRUGO,           get_system_cpld_reg, NULL,         GBIC_MOD_INT);
static SENSOR_DEVICE_ATTR(psu1_int,          S_IRUGO,           get_system_cpld_reg, NULL,         PSU1_INT);
static SENSOR_DEVICE_ATTR(psu2_int,          S_IRUGO,           get_system_cpld_reg, NULL,         PSU2_INT);
static SENSOR_DEVICE_ATTR(button_mask,       S_IRUGO | S_IWUSR, get_system_cpld_reg, set_system_cpld_reg, BUTTON_MASK);
static SENSOR_DEVICE_ATTR(watchdog_mask,     S_IRUGO | S_IWUSR, get_system_cpld_reg, set_system_cpld_reg, WATCHDOG_MASK);
static SENSOR_DEVICE_ATTR(mac_mask,          S_IRUGO | S_IWUSR, get_system_cpld_reg, set_system_cpld_reg, MAC_MASK);
static SENSOR_DEVICE_ATTR(chip_81000_mask,   S_IRUGO | S_IWUSR, get_system_cpld_reg, set_system_cpld_reg, CHIP_81000_MASK);
static SENSOR_DEVICE_ATTR(chip_54140_mask,   S_IRUGO | S_IWUSR, get_system_cpld_reg, set_system_cpld_reg, CHIP_54140_MASK);
static SENSOR_DEVICE_ATTR(gbic_mod_prst_mask,S_IRUGO | S_IWUSR, get_system_cpld_reg, set_system_cpld_reg, GBIC_MOD_PRST_MASK);
static SENSOR_DEVICE_ATTR(gbic_mod_int_mask, S_IRUGO | S_IWUSR, get_system_cpld_reg, set_system_cpld_reg, GBIC_MOD_INT_MASK);
static SENSOR_DEVICE_ATTR(psu1_mask,         S_IRUGO | S_IWUSR, get_system_cpld_reg, set_system_cpld_reg, PSU1_MASK);
static SENSOR_DEVICE_ATTR(psu2_mask,         S_IRUGO | S_IWUSR, get_system_cpld_reg, set_system_cpld_reg, PSU2_MASK);
static SENSOR_DEVICE_ATTR(psu1_on,           S_IRUGO | S_IWUSR, get_system_cpld_reg, set_system_cpld_reg, PSU1_ON);
static SENSOR_DEVICE_ATTR(psu2_on,           S_IRUGO | S_IWUSR, get_system_cpld_reg, set_system_cpld_reg, PSU2_ON);
static SENSOR_DEVICE_ATTR(psu1_prst,         S_IRUGO,           get_system_cpld_reg, NULL,         PSU1_PRST);
static SENSOR_DEVICE_ATTR(psu2_prst,         S_IRUGO,           get_system_cpld_reg, NULL,         PSU2_PRST);
static SENSOR_DEVICE_ATTR(psu1_smbalert,     S_IRUGO,           get_system_cpld_reg, NULL,         PSU1_SMBALERT);
static SENSOR_DEVICE_ATTR(psu2_smbalert,     S_IRUGO,           get_system_cpld_reg, NULL,         PSU2_SMBALERT);
static SENSOR_DEVICE_ATTR(psu1_pwr_good,     S_IRUGO,           get_system_cpld_reg, NULL,         PSU1_PWR_GOOD);
static SENSOR_DEVICE_ATTR(psu2_pwr_good,     S_IRUGO,           get_system_cpld_reg, NULL,         PSU2_PWR_GOOD);
static SENSOR_DEVICE_ATTR(psu1_ac_ok,        S_IRUGO,           get_system_cpld_reg, NULL,         PSU1_AC_OK);
static SENSOR_DEVICE_ATTR(psu2_ac_ok,        S_IRUGO,           get_system_cpld_reg, NULL,         PSU2_AC_OK);
static SENSOR_DEVICE_ATTR(uart_1588_sel,     S_IRUGO | S_IWUSR, get_system_cpld_reg, set_system_cpld_reg, UART_1588_SEL);
static SENSOR_DEVICE_ATTR(bf3_1588,          S_IRUGO | S_IWUSR, get_system_cpld_reg, set_system_cpld_reg, BF3_1588);
static SENSOR_DEVICE_ATTR(bf2_1588,          S_IRUGO | S_IWUSR, get_system_cpld_reg, set_system_cpld_reg, BF2_1588);
static SENSOR_DEVICE_ATTR(bf1_1588,          S_IRUGO | S_IWUSR, get_system_cpld_reg, set_system_cpld_reg, BF1_1588);
static SENSOR_DEVICE_ATTR(isp_1041a_en,      S_IRUGO | S_IWUSR, get_system_cpld_reg, set_system_cpld_reg, ISP_1014A_EN);
static SENSOR_DEVICE_ATTR(isp_fpga_en,       S_IRUGO | S_IWUSR, get_system_cpld_reg, set_system_cpld_reg, ISP_FPGA_EN);
static SENSOR_DEVICE_ATTR(uart_sel,          S_IRUGO | S_IWUSR, get_system_cpld_reg, set_system_cpld_reg, UART_SEL);
static SENSOR_DEVICE_ATTR(i2c_bus_status,    S_IRUGO | S_IWUSR, get_system_cpld_reg, set_system_cpld_reg, I2C_BUS_STATUS);
static SENSOR_DEVICE_ATTR(bmc_flash1,        S_IRUGO | S_IWUSR, get_system_cpld_reg, set_system_cpld_reg, BMC_FLASH1);
static SENSOR_DEVICE_ATTR(bmc_flash2,        S_IRUGO | S_IWUSR, get_system_cpld_reg, set_system_cpld_reg, BMC_FLASH2);
static SENSOR_DEVICE_ATTR(pwr_led_ctrl,      S_IRUGO | S_IWUSR, get_system_cpld_reg, set_system_cpld_reg, PWR_LED_CTRL);
static SENSOR_DEVICE_ATTR(fan_led,           S_IRUGO | S_IWUSR, get_system_cpld_reg, set_system_cpld_reg, FAN_LED);
static SENSOR_DEVICE_ATTR(sys_led,           S_IRUGO | S_IWUSR, get_system_cpld_reg, set_system_cpld_reg, SYS_LED);
static SENSOR_DEVICE_ATTR(pwr_led,           S_IRUGO | S_IWUSR, get_system_cpld_reg, set_system_cpld_reg, PWR_LED);
static SENSOR_DEVICE_ATTR(tod_1pps_rx,       S_IRUGO | S_IWUSR, get_system_cpld_reg, set_system_cpld_reg, TOD_1PPS_RX);
static SENSOR_DEVICE_ATTR(tod_1pps_tx,       S_IRUGO | S_IWUSR, get_system_cpld_reg, set_system_cpld_reg, TOD_1PPS_TX);
static SENSOR_DEVICE_ATTR(tod_rx,            S_IRUGO | S_IWUSR, get_system_cpld_reg, set_system_cpld_reg, TOD_RX);
static SENSOR_DEVICE_ATTR(tod_tx,            S_IRUGO | S_IWUSR, get_system_cpld_reg, set_system_cpld_reg, TOD_TX);
static SENSOR_DEVICE_ATTR(gps_mod_en,        S_IRUGO | S_IWUSR, get_system_cpld_reg, set_system_cpld_reg, GPS_MOD_EN);
static SENSOR_DEVICE_ATTR(heater_en,         S_IRUGO | S_IWUSR, get_system_cpld_reg, set_system_cpld_reg, HEATER_EN);
static SENSOR_DEVICE_ATTR(mac_core_freq,     S_IRUGO | S_IWUSR, get_system_cpld_reg, set_system_cpld_reg, MAC_CORE_FREQ);
static SENSOR_DEVICE_ATTR(watchdog_timer,    S_IRUGO | S_IWUSR, get_system_cpld_reg, set_system_cpld_reg, WATCHDOG_TIMER);
static SENSOR_DEVICE_ATTR(watchdog_en,       S_IRUGO | S_IWUSR, get_system_cpld_reg, set_system_cpld_reg, WATCHDOG_EN);
static SENSOR_DEVICE_ATTR(watchdog_timer_clear, S_IRUGO | S_IWUSR, get_system_cpld_reg, set_system_cpld_reg, WATCHDOG_TIMER_CLEAR);
static SENSOR_DEVICE_ATTR(mac_vol,           S_IRUGO | S_IWUSR, get_system_cpld_reg, set_system_cpld_reg, MAC_VOL);

static SENSOR_DEVICE_ATTR(port_cpld0_reg_addr,   S_IRUGO | S_IWUSR, get_port_cpld0_reg, set_port_cpld0_reg, PORT_CPLD0_REG_ADDR);
static SENSOR_DEVICE_ATTR(port_cpld0_reg_value,  S_IRUGO | S_IWUSR, get_port_cpld0_reg, set_port_cpld0_reg, PORT_CPLD0_REG_VALUE);
static SENSOR_DEVICE_ATTR(port_cpld0_ver,        S_IRUGO,           get_port_cpld0_reg, NULL,          PORT_CPLD0_VER);
static SENSOR_DEVICE_ATTR(sfpp_tx_fault,         S_IRUGO | S_IWUSR, get_port_cpld0_reg, set_port_cpld0_reg, SFPP_TX_FAULT);
static SENSOR_DEVICE_ATTR(sfpp_tx_fault_mask,    S_IRUGO | S_IWUSR, get_port_cpld0_reg, set_port_cpld0_reg, SFPP_TX_FAULT_MASK);
static SENSOR_DEVICE_ATTR(sfpp_tx_disable,       S_IRUGO | S_IWUSR, get_port_cpld0_reg, set_port_cpld0_reg, SFPP_TX_DISABLE);
static SENSOR_DEVICE_ATTR(sfpp_prst,             S_IRUGO | S_IWUSR, get_port_cpld0_reg, set_port_cpld0_reg, SFPP_PRST);
static SENSOR_DEVICE_ATTR(sfpp_prst_mask,        S_IRUGO | S_IWUSR, get_port_cpld0_reg, set_port_cpld0_reg, SFPP_PRST_MASK);
static SENSOR_DEVICE_ATTR(sfpp_rx_loss,          S_IRUGO | S_IWUSR, get_port_cpld0_reg, set_port_cpld0_reg, SFPP_RX_LOSS);
static SENSOR_DEVICE_ATTR(sfpp_rx_loss_mask,     S_IRUGO | S_IWUSR, get_port_cpld0_reg, set_port_cpld0_reg, SFPP_RX_LOSS_MASK);
static SENSOR_DEVICE_ATTR(sfp_tx_fault,          S_IRUGO | S_IWUSR, get_port_cpld0_reg, set_port_cpld0_reg, SFP_TX_FAULT);
static SENSOR_DEVICE_ATTR(sfp_tx_fault_mask,     S_IRUGO | S_IWUSR, get_port_cpld0_reg, set_port_cpld0_reg, SFP_TX_FAULT_MASK);
static SENSOR_DEVICE_ATTR(sfp_tx_disable,        S_IRUGO | S_IWUSR, get_port_cpld0_reg, set_port_cpld0_reg, SFP_TX_DISABLE);
static SENSOR_DEVICE_ATTR(sfp_prst,              S_IRUGO | S_IWUSR, get_port_cpld0_reg, set_port_cpld0_reg, SFP_PRST);
static SENSOR_DEVICE_ATTR(sfp_prst_mask,         S_IRUGO | S_IWUSR, get_port_cpld0_reg, set_port_cpld0_reg, SFP_PRST_MASK);
static SENSOR_DEVICE_ATTR(sfp_rx_loss,           S_IRUGO | S_IWUSR, get_port_cpld0_reg, set_port_cpld0_reg, SFP_RX_LOSS);
static SENSOR_DEVICE_ATTR(sfp_rx_loss_mask,      S_IRUGO | S_IWUSR, get_port_cpld0_reg, set_port_cpld0_reg, SFP_RX_LOSS_MASK);

static struct attribute *system_cpld_device_attrs[] = {
    &sensor_dev_attr_system_cpld_reg_value.dev_attr.attr,
    &sensor_dev_attr_system_cpld_reg_addr.dev_attr.attr,
    &sensor_dev_attr_system_cpld_ver.dev_attr.attr,
    &sensor_dev_attr_mb_pwr_recycle.dev_attr.attr,
    &sensor_dev_attr_cpu_pwr_recycle.dev_attr.attr,
    &sensor_dev_attr_mb_pwr_good.dev_attr.attr,
    &sensor_dev_attr_mac_rst.dev_attr.attr,
    &sensor_dev_attr_phy_1g_rst.dev_attr.attr,
    &sensor_dev_attr_phy_10g_rst.dev_attr.attr,
    &sensor_dev_attr_phy_25g_rst.dev_attr.attr,
    &sensor_dev_attr_phy_100g_rst.dev_attr.attr,
    &sensor_dev_attr_system_cpld0_rst.dev_attr.attr,
    &sensor_dev_attr_pca9548_0_rst.dev_attr.attr,
    &sensor_dev_attr_pca9548_0_1_rst.dev_attr.attr,
    &sensor_dev_attr_pca9548_0_2_rst.dev_attr.attr,
    &sensor_dev_attr_pca9548_2_rst.dev_attr.attr,
    &sensor_dev_attr_pca9546_3_rst.dev_attr.attr,
    &sensor_dev_attr_pca9546_4_rst.dev_attr.attr,
    &sensor_dev_attr_gps_uart_rst.dev_attr.attr,
    &sensor_dev_attr_tod_uart_rst.dev_attr.attr,
    &sensor_dev_attr_gps_mod_rst.dev_attr.attr,
    &sensor_dev_attr_zl30364_rst.dev_attr.attr,
    &sensor_dev_attr_chip_81000_rst.dev_attr.attr,
    &sensor_dev_attr_nic_i210_rst.dev_attr.attr,
    &sensor_dev_attr_usb_hub_rst.dev_attr.attr,
    &sensor_dev_attr_button_int.dev_attr.attr,
    &sensor_dev_attr_watchdog_int.dev_attr.attr,
    &sensor_dev_attr_mac_int.dev_attr.attr,
    &sensor_dev_attr_chip_81000_int.dev_attr.attr,
    &sensor_dev_attr_chip_54140_int.dev_attr.attr,
    &sensor_dev_attr_gbic_mod_prst.dev_attr.attr,
    &sensor_dev_attr_gbic_mod_int.dev_attr.attr,
    &sensor_dev_attr_psu1_int.dev_attr.attr,
    &sensor_dev_attr_psu2_int.dev_attr.attr,
    &sensor_dev_attr_button_mask.dev_attr.attr,
    &sensor_dev_attr_watchdog_mask.dev_attr.attr,
    &sensor_dev_attr_mac_mask.dev_attr.attr,
    &sensor_dev_attr_chip_81000_mask.dev_attr.attr,
    &sensor_dev_attr_chip_54140_mask.dev_attr.attr,
    &sensor_dev_attr_gbic_mod_prst_mask.dev_attr.attr,
    &sensor_dev_attr_gbic_mod_int_mask.dev_attr.attr,
    &sensor_dev_attr_psu1_mask.dev_attr.attr,
    &sensor_dev_attr_psu2_mask.dev_attr.attr,
    &sensor_dev_attr_psu1_on.dev_attr.attr,
    &sensor_dev_attr_psu2_on.dev_attr.attr,
    &sensor_dev_attr_psu1_prst.dev_attr.attr,
    &sensor_dev_attr_psu2_prst.dev_attr.attr,
    &sensor_dev_attr_psu1_smbalert.dev_attr.attr,
    &sensor_dev_attr_psu2_smbalert.dev_attr.attr,
    &sensor_dev_attr_psu1_pwr_good.dev_attr.attr,
    &sensor_dev_attr_psu2_pwr_good.dev_attr.attr,
    &sensor_dev_attr_psu1_ac_ok.dev_attr.attr,
    &sensor_dev_attr_psu2_ac_ok.dev_attr.attr,
    &sensor_dev_attr_uart_1588_sel.dev_attr.attr,
    &sensor_dev_attr_bf3_1588.dev_attr.attr,
    &sensor_dev_attr_bf2_1588.dev_attr.attr,
    &sensor_dev_attr_bf1_1588.dev_attr.attr,
    &sensor_dev_attr_isp_1041a_en.dev_attr.attr,
    &sensor_dev_attr_isp_fpga_en.dev_attr.attr,
    &sensor_dev_attr_uart_sel.dev_attr.attr,
    &sensor_dev_attr_i2c_bus_status.dev_attr.attr,
    &sensor_dev_attr_bmc_flash1.dev_attr.attr,
    &sensor_dev_attr_bmc_flash2.dev_attr.attr,
    &sensor_dev_attr_pwr_led_ctrl.dev_attr.attr,
    &sensor_dev_attr_fan_led.dev_attr.attr,
    &sensor_dev_attr_sys_led.dev_attr.attr,
    &sensor_dev_attr_pwr_led.dev_attr.attr,
    &sensor_dev_attr_tod_1pps_rx.dev_attr.attr,
    &sensor_dev_attr_tod_1pps_tx.dev_attr.attr,
    &sensor_dev_attr_tod_rx.dev_attr.attr,
    &sensor_dev_attr_tod_tx.dev_attr.attr,
    &sensor_dev_attr_gps_mod_en.dev_attr.attr,
    &sensor_dev_attr_heater_en.dev_attr.attr,
    &sensor_dev_attr_mac_core_freq.dev_attr.attr,
    &sensor_dev_attr_watchdog_timer.dev_attr.attr,
    &sensor_dev_attr_watchdog_en.dev_attr.attr,
    &sensor_dev_attr_watchdog_timer_clear.dev_attr.attr,
    &sensor_dev_attr_mac_vol.dev_attr.attr,
    NULL,
};

static struct attribute *port_cpld0_device_attrs[] = {
    &sensor_dev_attr_port_cpld0_reg_addr.dev_attr.attr,
    &sensor_dev_attr_port_cpld0_reg_value.dev_attr.attr,
    &sensor_dev_attr_port_cpld0_ver.dev_attr.attr,
    &sensor_dev_attr_sfpp_tx_fault.dev_attr.attr,
    &sensor_dev_attr_sfpp_tx_fault_mask.dev_attr.attr,
    &sensor_dev_attr_sfpp_tx_disable.dev_attr.attr,
    &sensor_dev_attr_sfpp_prst.dev_attr.attr,
    &sensor_dev_attr_sfpp_prst_mask.dev_attr.attr,
    &sensor_dev_attr_sfpp_rx_loss.dev_attr.attr,
    &sensor_dev_attr_sfpp_rx_loss_mask.dev_attr.attr,
    &sensor_dev_attr_sfp_tx_fault.dev_attr.attr,
    &sensor_dev_attr_sfp_tx_fault_mask.dev_attr.attr,
    &sensor_dev_attr_sfp_tx_disable.dev_attr.attr,
    &sensor_dev_attr_sfp_prst.dev_attr.attr,
    &sensor_dev_attr_sfp_prst_mask.dev_attr.attr,
    &sensor_dev_attr_sfp_rx_loss.dev_attr.attr,
    &sensor_dev_attr_sfp_rx_loss_mask.dev_attr.attr,
    NULL,
};

static struct attribute_group system_cpld_device_attr_grp = {
    .attrs = system_cpld_device_attrs,
};

static struct attribute_group port_cpld0_device_attr_grp = {
    .attrs = port_cpld0_device_attrs,
};

static int __init system_cpld_probe(struct platform_device *pdev)
{
    int ret;
    ret = sysfs_create_group(&pdev->dev.kobj, &system_cpld_device_attr_grp);
    if (ret) {
        printk(KERN_WARNING "Fail to create system cpld attribute group");
        return -ENODEV;
    }
    return 0;
}

static int __exit system_cpld_remove(struct platform_device *pdev)
{
    sysfs_remove_group(&pdev->dev.kobj, &system_cpld_device_attr_grp);
    return 0;
}

static struct platform_driver system_cpld_driver = {
    .probe  = system_cpld_probe,
    .remove = __exit_p(system_cpld_remove),
    .driver = {
        .owner = THIS_MODULE,
        .name  = "delta-agc7008s-system-cpld",
    },
};

static int __init port_cpld0_probe(struct platform_device *pdev)
{
    int ret;
    ret = sysfs_create_group(&pdev->dev.kobj, &port_cpld0_device_attr_grp);
    if (ret) {
        printk(KERN_WARNING "Fail to create port cpld0 attribute group");
        return -ENODEV;
    }
    return 0;
}

static int __exit port_cpld0_remove(struct platform_device *pdev)
{
    sysfs_remove_group(&pdev->dev.kobj, &port_cpld0_device_attr_grp);
    return 0;
}

static struct platform_driver port_cpld0_driver = {
    .probe  = port_cpld0_probe,
    .remove = __exit_p(port_cpld0_remove),
    .driver = {
        .owner = THIS_MODULE,
        .name  = "delta-agc7008s-port-cpld0",
    },
};

/* --------------- SWPLD - end --------------- */

/* --------------- module initialization --------------- */
static int __init delta_agc7008s_swpld_init(void)
{
    int ret;
    printk(KERN_WARNING "agc7008s_platform_swpld module initialization\n");

    ret = dni_create_user();
    if (ret != 0){
        printk(KERN_WARNING "Fail to create IPMI user\n");
    }

    // set the SYSTEM_CPLD prob and remove
    ret = platform_driver_register(&system_cpld_driver);
    if (ret) {
        printk(KERN_WARNING "Fail to register system_cpld driver\n");
        goto error_system_cpld_driver;
    }

    // register the SYSTEM_CPLD
    ret = platform_device_register(&system_cpld_device);
    if (ret) {
        printk(KERN_WARNING "Fail to create system_cpld device\n");
        goto error_system_cpld_device;
    }

    // set the PORT_CPLD0 prob and remove
    ret = platform_driver_register(&port_cpld0_driver);
    if (ret) {
        printk(KERN_WARNING "Fail to register port_cpld0 driver\n");
        goto error_port_cpld0_driver;
    }

    // register the PORT_CPLD0
    ret = platform_device_register(&port_cpld0_device);
    if (ret) {
        printk(KERN_WARNING "Fail to create port_cpld0 device\n");
        goto error_port_cpld0_device;
    }

    return 0;

error_port_cpld0_device:
    platform_driver_unregister(&port_cpld0_driver);
error_port_cpld0_driver:
    platform_driver_unregister(&system_cpld_device);
error_system_cpld_device:
    platform_driver_unregister(&system_cpld_driver);
error_system_cpld_driver:
    return ret;
}

static void __exit delta_agc7008s_swpld_exit(void)
{
    platform_device_unregister(&port_cpld0_device);
    platform_driver_unregister(&port_cpld0_driver);
    platform_device_unregister(&system_cpld_device);
    platform_driver_unregister(&system_cpld_driver);
}
module_init(delta_agc7008s_swpld_init);
module_exit(delta_agc7008s_swpld_exit);

MODULE_DESCRIPTION("DNI agc7008s SWPLD Platform Support");
MODULE_AUTHOR("Jacky Liu <jacky.js.liu@deltaww.com>");
MODULE_LICENSE("GPL");
