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
#include <string.h>
#include <onlp/onlp.h>
#include <onlplib/file.h>
#include <onlp/platformi/sfpi.h>
#include "platform_lib.h"

enum onlp_psu_type onlp_get_psu_type(int pid)
{
    int hwmon_idx;
    int val;
    char file[32];
    enum onlp_psu_type type = PSU_TYPE_DC;

    hwmon_idx = onlp_get_psu_hwmon_idx(pid);
    if (hwmon_idx < 0) {
        return type;
    }

    snprintf(file, sizeof(file), "psu%d_type", pid);

    if (onlp_file_read_int(&val, PSU_SYSFS_FORMAT_1, hwmon_idx, file) == 0) {
        if (val >= 0 && val < PSU_TYPE_COUNT) {
            type = (enum onlp_psu_type)val;
        }
    }

    return type;
}

enum onlp_fan_dir onlp_get_fan_dir(int fid)
{
    int len = 0;
    int i = 0;
    int hwmon_idx;
    char *str = NULL;
    char *dirs[FAN_DIR_COUNT] = { "F2B", "B2F" };
    char file[32];
    enum onlp_fan_dir dir = FAN_DIR_F2B;

    hwmon_idx = onlp_get_fan_hwmon_idx();
    if (hwmon_idx >= 0) {
        /* Read attribute */
        snprintf(file, sizeof(file), "fan%d_dir", fid);
        len = onlp_file_read_str(&str, FAN_SYSFS_FORMAT_1, hwmon_idx, file);

        /* Verify Fan dir string length */
        if (!str || len < 3) {
            AIM_FREE_IF_PTR(str);
            return dir;
        }

        for (i = 0; i < AIM_ARRAYSIZE(dirs); i++) {
            if (strncmp(str, dirs[i], strlen(dirs[i])) == 0) {
                dir = (enum onlp_fan_dir)i;
                break;
            }
        }

        AIM_FREE_IF_PTR(str);
    }

    return dir;
}

int onlp_get_psu_hwmon_idx(int pid)
{
    /* find hwmon index */
    char* file = NULL;
    char path[64];
    int ret, hwmon_idx, max_hwmon_idx = 20;

    for (hwmon_idx = 0; hwmon_idx <= max_hwmon_idx; hwmon_idx++) {
        snprintf(path, sizeof(path), "/sys/devices/platform/as7927_50x_psu/hwmon/hwmon%d/", hwmon_idx);

        if (pid == 1)
            ret = onlp_file_find(path, "psu1_present", &file);
        else if (pid == 2)
            ret = onlp_file_find(path, "psu2_present", &file);
        else
            return -1;

        AIM_FREE_IF_PTR(file);

        if (ONLP_STATUS_OK == ret)
            return hwmon_idx;
    }

    return -1;
}

int onlp_get_fan_hwmon_idx(void)
{
    /* find hwmon index */
    char* file = NULL;
    char path[64];
    int ret, hwmon_idx, max_hwmon_idx = 20;

    for (hwmon_idx = 0; hwmon_idx <= max_hwmon_idx; hwmon_idx++) {
        snprintf(path, sizeof(path), "/sys/devices/platform/as7927_50x_fan/hwmon/hwmon%d/", hwmon_idx);

        ret = onlp_file_find(path, "name", &file);
        AIM_FREE_IF_PTR(file);

        if (ONLP_STATUS_OK == ret)
            return hwmon_idx;
    }

    return -1;
}

int
get_xcvr_presence(void)
{
    onlp_sfp_bitmap_t bitmap;
    onlp_sfp_bitmap_t_init(&bitmap);
    onlp_sfp_presence_bitmap_get(&bitmap);
    return !(AIM_BITMAP_COUNT(&bitmap) == 0);
}

// =======================================================
// SFF-8472 (SFP) Temperature and Alarm Getters
// =======================================================
int
get_sff8472_temp(int port, int *temp)
{
    int value;
    int16_t port_temp;

    /* SFF-8472 DDM Support Check is at Address A0h (0x50), Offset 92 */
    value = onlp_sfpi_dev_readb(port, 0x50, 92);
    if (value < 0 || !(value & 0x40)) {
        *temp = ONLP_STATUS_E_MISSING;
        return ONLP_STATUS_E_MISSING;
    }

    /* SFF-8472 Temperature is at Address A2h (0x51), Offset 96-97 */
    value = onlp_sfpi_dev_readb(port, 0x51, 96);
    if (value < 0) {
        *temp = ONLP_STATUS_E_MISSING;
        return ONLP_STATUS_E_MISSING;
    }
    port_temp = (int16_t)((value & 0xFF) << 8);

    value = onlp_sfpi_dev_readb(port, 0x51, 97);
    if (value < 0) {
        *temp = ONLP_STATUS_E_MISSING;
        return ONLP_STATUS_E_MISSING;
    }
    port_temp = (port_temp | (int16_t)(value & 0xFF));

    *temp = (int)port_temp * 1000 / 256;
    return ONLP_STATUS_OK;
}

int
get_sff8472_temp_alarm(int port, int *alarm)
{
    int value;
    int16_t port_alarm;

    /* SFF-8472 High Alarm is at Address A2h (0x51), Offset 00-01 */
    value = onlp_sfpi_dev_readb(port, 0x51, 0);
    if (value < 0) {
        *alarm = ONLP_STATUS_E_MISSING;
        return ONLP_STATUS_E_MISSING;
    }
    port_alarm = (int16_t)((value & 0xFF) << 8);

    value = onlp_sfpi_dev_readb(port, 0x51, 1);
    if (value < 0) {
        *alarm = ONLP_STATUS_E_MISSING;
        return ONLP_STATUS_E_MISSING;
    }
    port_alarm = (port_alarm | (int16_t)(value & 0xFF));

    *alarm = (int)port_alarm * 1000 / 256;
    return ONLP_STATUS_OK;
}

// =======================================================
// SFF-8436 (QSFP+) Temperature and Alarm Getters
// =======================================================
int
get_sff8436_temp(int port, int *temp)
{
    int value;
    int16_t port_temp;

    /* QSFP+ Temperature is at Address A0h (0x50), Page 00h, Offset 22-23 */
    value = onlp_sfpi_dev_readb(port, 0x50, 22);
    if (value < 0) {
        *temp = ONLP_STATUS_E_MISSING;
        return ONLP_STATUS_E_MISSING;
    }
    port_temp = (int16_t)((value & 0xFF) << 8);

    value = onlp_sfpi_dev_readb(port, 0x50, 23);
    if (value < 0) {
        *temp = ONLP_STATUS_E_MISSING;
        return ONLP_STATUS_E_MISSING;
    }
    port_temp = (port_temp | (int16_t)(value & 0xFF));

    *temp = (int)port_temp * 1000 / 256;
    return ONLP_STATUS_OK;
}

int
get_sff8436_temp_alarm(int port, int *alarm)
{
    int value;
    int16_t port_alarm;

    /* QSFP+ Memory model check is at Address A0h (0x50), Offset 2 */
    value = onlp_sfpi_dev_readb(port, 0x50, 0x2);
    if (value < 0 || (value & 0x04)) {
        *alarm = ONLP_STATUS_E_MISSING;
        return ONLP_STATUS_E_MISSING;
    }

    /* QSFP+ Temp High Alarm is at Page 03h，Offset 128-129 */
    if (onlp_sfpi_dev_writeb(port, 0x50, 127, 0x03) < 0) {
        *alarm = ONLP_STATUS_E_MISSING;
        return ONLP_STATUS_E_MISSING;
    }

    value = onlp_sfpi_dev_readb(port, 0x50, 128);
    if (value < 0) {
        *alarm = ONLP_STATUS_E_MISSING;
        onlp_sfpi_dev_writeb(port, 0x50, 127, 0x00);
        return ONLP_STATUS_E_MISSING;
    }
    port_alarm = (int16_t)((value & 0xFF) << 8);

    value = onlp_sfpi_dev_readb(port, 0x50, 129);
    if (value < 0) {
        *alarm = ONLP_STATUS_E_MISSING;
        onlp_sfpi_dev_writeb(port, 0x50, 127, 0x00);
        return ONLP_STATUS_E_MISSING;
    }
    port_alarm = (port_alarm | (int16_t)(value & 0xFF));
    *alarm = (int)port_alarm * 1000 / 256;

    onlp_sfpi_dev_writeb(port, 0x50, 127, 0x00);
    return ONLP_STATUS_OK;
}

// =======================================================
// CMIS (QSFP-DD) Temperature and Alarm Getters
// =======================================================
int get_cmis_temp(int port, int *temp)
{
    int value;
    int16_t port_temp;

    /* CMIS Temperature is at Address A0h (0x50), Page 00h, Offset 14-15 */
    value = onlp_sfpi_dev_readb(port, 0x50, 14);
    if (value < 0) {
        *temp = ONLP_STATUS_E_MISSING;
        return ONLP_STATUS_E_MISSING;
    }
    port_temp = (int16_t)((value & 0xFF) << 8);

    value = onlp_sfpi_dev_readb(port, 0x50, 15);
    if (value < 0) {
        *temp = ONLP_STATUS_E_MISSING;
        return ONLP_STATUS_E_MISSING;
    }
    port_temp = (port_temp | (int16_t)(value & 0xFF));

    *temp = (int)port_temp * 1000 / 256;
    return ONLP_STATUS_OK;
}

int get_cmis_temp_alarm(int port, int *alarm)
{
    int value;
    int16_t port_alarm;

    /* CMIS Memory model check is at Address A0h (0x50), Offset 2 */
    value = onlp_sfpi_dev_readb(port, 0x50, 0x2);
    if (value < 0 || (value & 0x80)) {
        *alarm = ONLP_STATUS_E_MISSING;
        return ONLP_STATUS_E_MISSING;
    }

    /* CMIS Temp High Alarm is at Page 02h，Offset 128-129 */
    if (onlp_sfpi_dev_writeb(port, 0x50, 127, 0x02) < 0) {
        *alarm = ONLP_STATUS_E_MISSING;
        return ONLP_STATUS_E_MISSING;
    }

    value = onlp_sfpi_dev_readb(port, 0x50, 128);
    if (value < 0) {
        *alarm = ONLP_STATUS_E_MISSING;
        onlp_sfpi_dev_writeb(port, 0x50, 127, 0x00);
        return ONLP_STATUS_E_MISSING;
    }
    port_alarm = (int16_t)((value & 0xFF) << 8);

    value = onlp_sfpi_dev_readb(port, 0x50, 129);
    if (value < 0) {
        *alarm = ONLP_STATUS_E_MISSING;
        onlp_sfpi_dev_writeb(port, 0x50, 127, 0x00);
        return ONLP_STATUS_E_MISSING;
    }
    port_alarm = (port_alarm | (int16_t)(value & 0xFF));
    *alarm = (int)port_alarm * 1000 / 256;

    onlp_sfpi_dev_writeb(port, 0x50, 127, 0x00);
    return ONLP_STATUS_OK;
}

int
get_xcvr_temp(temp_reader_data_t *temp)
{
    int ret = ONLP_STATUS_OK;
    int value, port;
    int port_temp = ONLP_STATUS_E_MISSING, port_alarm = ONLP_STATUS_E_MISSING;

    memset(temp, 0, sizeof(temp_reader_data_t));

    if (!get_xcvr_presence()) {
        return ONLP_STATUS_OK;
    }

    for (port = 1; port <= PORT_NUM; port++) {
        temp->ports[port].temp = ONLP_STATUS_E_MISSING;
        temp->ports[port].high_alarm = ONLP_STATUS_E_MISSING;
        temp->ports[port].present = 0;

        port_temp = ONLP_STATUS_E_MISSING;
        port_alarm = ONLP_STATUS_E_MISSING;

        if (onlp_sfpi_is_present(port) != 1) {
            continue;
        }

        temp->ports[port].present = 1;

        value = onlp_sfpi_dev_readb(port, 0x50, 0);
        if (value < 0) {
            AIM_LOG_ERROR("Unable to get read port(%d) eeprom\r\n", port);
            continue;
        }

        if (value == 0x18 || value == 0x19 || value == 0x1E) {
            ret = get_cmis_temp(port, &port_temp);
            if (ret != ONLP_STATUS_OK) {
                continue;
            }
            get_cmis_temp_alarm(port, &port_alarm);
        }
        else if (value == 0x0C || value == 0x0D || value == 0x11 || value ==  0xE1) {
            ret = get_sff8436_temp(port, &port_temp);
            if (ret != ONLP_STATUS_OK) {
                continue;
            }
            get_sff8436_temp_alarm(port, &port_alarm);
        }
        else if(value == 0x03 || value == 0x0b) {
            ret = get_sff8472_temp(port, &port_temp);
            if (ret != ONLP_STATUS_OK) {
                continue;
            }
            get_sff8472_temp_alarm(port, &port_alarm);
        }
        else {
            continue;
        }

        temp->ports[port].temp = port_temp;
        temp->ports[port].high_alarm = (port_alarm != ONLP_STATUS_E_MISSING) ? port_alarm : 75000;
    }

    return ONLP_STATUS_OK;
}
