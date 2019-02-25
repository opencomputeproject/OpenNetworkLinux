/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/jiffies.h>
#include <linux/i2c.h>
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>
#include <linux/err.h>
#include <linux/mutex.h>
#include <linux/delay.h>
#include <linux/kthread.h>
//#include "I2CHostCommunication.h"
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#define SWITCH_TEMPERATURE_SOCK     "/proc/switch/temp"

#define USE_SMBUS    1

#define FAN_NUM  4 
#define PSU_NUM  2 

static struct device *psoc_led_client_dev = NULL;

struct __attribute__ ((__packed__))  psoc_psu_layout {
    u16 psu1_iin;
    u16 psu2_iin;
    u16 psu1_iout;
    u16 psu2_iout;
    
    u16 psu1_pin;
    u16 psu2_pin;
    u16 psu1_pout;
    u16 psu2_pout;
    
    u16 psu1_vin;
    u16 psu2_vin;
    u16 psu1_vout;
    u16 psu2_vout;

    u8  psu1_vendor[16];
    u8  psu2_vendor[16];
    u8  psu1_model[20];
    u8  psu2_model[20];
    u8  psu1_version[8];
    u8  psu2_version[8];
    u8  psu1_date[6];
    u8  psu2_date[6];
    u8  psu1_sn[20];
    u8  psu2_sn[20];
};

struct clei{
    u8 issue_number[3];
    u8 abbreviation_number[9];
    u8 fc_number[10];
    u8 clei_code[10];
    u8 product_year_and_month[5];
    u8 label_location_code[2];
    u8 serial_number[5];
    u8 pcb_revision[5];
    u8 reserved[15];
};

struct __attribute__ ((__packed__))  psoc_layout {
    u8 ctl;                 //offset: 0
    u16 switch_temp;        //offset: 1
    u8 reserve0;            //offset: 3

    u8 fw_upgrade;          //offset: 4

    //i2c bridge
    u8 i2c_st;              //offset: 5
    u8 i2c_ctl;             //offset: 6
    u8 i2c_addr;            //offset: 7
    u8 i2c_data[0x20];      //offset: 8 

    //gpo
    u8 led_ctl;             //offset: 28

    u8 gpio;                //offset: 29

    //pwm duty
    u8 pwm[FAN_NUM];        //offset: 2a
    u8 pwm_psu[PSU_NUM];    //offset: 2e

    //fan rpm
    u16 fan[FAN_NUM*2];     //offset: 30
    
    u8  reserve1[4];        //offset: 40

    //gpi 
    u8 gpi_fan;             //offset: 44 

    //psu state
    u8 psu_state;           //offset: 45

    //temperature
    u16 temp[5];            //offset: 46
    u16 temp_psu[PSU_NUM];  //offset: 50

    //version
    u8 version[2];          //offset: 54
    
    u8  reserve2[4];        //offset: 56
    struct psoc_psu_layout psu_info;      //offset: 5a
    u8 reserved[0xff-0xfd];

    struct clei clei[6];   //FAN:0~3   PSU:4~5;
};

/* definition */
/* definition */
#define PSOC_OFF(m)    offsetof(struct psoc_layout, m)
#define PSOC_PSU_OFF(m)    offsetof(struct psoc_psu_layout, m)

#define SWITCH_TMP_OFFSET       PSOC_OFF(switch_temp)
#define PWM_OFFSET              PSOC_OFF(pwm)
#define THERMAL_OFFSET          PSOC_OFF(temp)
#define RPM_OFFSET              PSOC_OFF(fan)
#define DIAG_FLAG_OFFSET        PSOC_OFF(ctl)
#define FAN_LED_OFFSET          PSOC_OFF(led_ctl)
#define FAN_GPI_OFFSET          PSOC_OFF(gpi_fan)
#define PSOC_PSU_OFFSET         PSOC_OFF(psu_state)
#define VERSION_OFFSET          PSOC_OFF(version)
#define PSU_INFO_OFFSET         PSOC_OFF(psu_info)

#define CLEI_OFF(m)    offsetof(struct clei, m)
#define FAN1_CLEI_INDEX		0
#define FAN2_CLEI_INDEX		1
#define FAN3_CLEI_INDEX		2
#define FAN4_CLEI_INDEX		3
#define PSU1_CLEI_INDEX		4
#define PSU2_CLEI_INDEX		5
#define ISSUENUMBER_INDEX	0
#define ABBREVIATION_INDEX	1
#define FCNUMBER_INDEX		2
#define CLEICODE_INDEX		3
#define PRODUCTDATE_INDEX	4
#define LABELLOCATION_INDX	5
#define SERIALNUMBER_INDEX	6
#define PCBREVISION_INDEX	7

/* Each client has this additional data */
struct psoc_data {
	struct device		*hwmon_dev;
	struct mutex		update_lock;
	u32                 diag;
    struct task_struct  *auto_update;
};

/*-----------------------------------------------------------------------*/

static ssize_t psoc_i2c_read(struct i2c_client *client, u8 *buf, u16 offset, size_t count)
{
#if USE_SMBUS    
	int i;	
    for(i=0; i<count; i++) {
        i2c_smbus_write_byte_data(client, ((offset+i) >> 8), ((offset+i) & 0xff));
        buf[i] = i2c_smbus_read_byte(client);
    }	

    return count;
#else
	struct i2c_msg msg[2];
	char msgbuf[2];
	int status;

	memset(msg, 0, sizeof(msg));
	
	msgbuf[0] = offset >> 8 ;
	msgbuf[1] = offset & 0xff;
	
	msg[0].addr = client->addr;
	msg[0].buf = msgbuf;
	msg[0].len = 2;

	msg[1].addr = client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].buf = buf;
	msg[1].len = count;
	
	status = i2c_transfer(client->adapter, msg, 2);
	
	if(status == 2)
	    status = count;
	    
	return status;    
#endif	
}

static ssize_t psoc_i2c_write(struct i2c_client *client, char *buf, u16  offset, size_t count)
{
#if USE_SMBUS    
	int i;
	
    for(i=0; i<count; i++) {
	 i2c_smbus_write_word_data(client, ((offset+i) >> 8), ((offset+i) & 0xff) | (buf[i] << 8));
    }	
    return count;
#else
	struct i2c_msg msg;
	int status;
    u8 writebuf[256];
	
	int i = 0;

	msg.addr = client->addr;
	msg.flags = 0;

	/* msg.buf is u8 and casts will mask the values */
	msg.buf = writebuf;
	
	msg.buf[i++] = offset >> 8;
	msg.buf[i++] = offset & 0xff;
	memcpy(&msg.buf[i], buf, count);
	msg.len = i + count;
	
	status = i2c_transfer(client->adapter, &msg, 1);
	if (status == 1)
		status = count;
	
    return status;	
#endif    
}

#if 0
static u32 psoc_read32(struct i2c_client *client, u8 offset)
{
	u32 value = 0;
	u8 buf[4];
    
    if( psoc_i2c_read(client, buf, offset, 4) == 4)
        value = (buf[0]<<24 | buf[1]<<16 | buf[2]<<8 | buf[3]);
    
	return value;
}
#endif

static u16 psoc_read16(struct i2c_client *client, u16 offset)
{
	u16 value = 0;
	u8 buf[2];
    
    if(psoc_i2c_read(client, buf, offset, 2) == 2)
        value = (buf[0]<<8 | buf[1]<<0);
    
	return value;
}

static u8 psoc_read8(struct i2c_client *client, u16 offset)
{
	u8 value = 0;
	u8 buf = 0;
    
    if(psoc_i2c_read(client, &buf, offset, 1) == 1)
        value = buf;
    
	return value;
}

//PSOC i2c bridge regsters
#define PSOC_I2C_STATUS         0x05
#define PSOC_I2C_CNTRL          0x06
#define PSOC_I2C_ADDR           0x07
#define PSOC_I2C_DATA           0x08

//status bit definition
#define PSOC_I2C_START          (1 << 0)
#define PSOC_PMB_SEL            (1 << 7)

//addr bits definition
#define PSOC_I2C_READ           (1 << 0)

//PMBUS registers definition
#define PMBUS_READ_VIN                  (0x88)
#define PMBUS_READ_IIN                  (0x89)
#define PMBUS_READ_VOUT                 (0x8B)
#define PMBUS_READ_IOUT                 (0x8C)
#define PMBUS_READ_POUT                 (0x96)
#define PMBUS_READ_PIN                  (0x97)
#if 0
static int psoc_i2c_bridge_read(struct i2c_client *client,
                                unsigned char bus, 
                                unsigned char chip, 
                                char *addr,          int alen, 
                                unsigned char *data, int len )
{
    unsigned char txdata[28], rxdata[28];
    int index, timeout;
    
	txdata[PSOC_I2C_STATUS] = 0;		/* the status */
	txdata[PSOC_I2C_CNTRL] = ((alen & 3) << 5) | (len & 0x1f);	/* the sizes */
	txdata[PSOC_I2C_ADDR] = (chip << 1) | PSOC_I2C_READ;		/* read address */
	for(index = 0; index < alen; index++)
		txdata[PSOC_I2C_DATA + index] = addr[index];		/* the chip address */
	for(; index < alen+len; index++)
		txdata[PSOC_I2C_DATA + index] = 0;		/* clear the chip data */
		
    psoc_i2c_write(client, &txdata[PSOC_I2C_CNTRL], PSOC_I2C_CNTRL, 2 + alen + len);
    
    //delay a while ???
    //---------------------------------------------------------------------
    //start write
	txdata[PSOC_I2C_STATUS] = PSOC_I2C_START | PSOC_PMB_SEL;	/* the start bit for the PM bus */
    psoc_i2c_write(client, &txdata[PSOC_I2C_STATUS], PSOC_I2C_STATUS, 1);
    
    //delay a while
    timeout = 40; //40*20==>800 ms
    do {
        psoc_i2c_read(client, &rxdata[PSOC_I2C_STATUS], PSOC_I2C_STATUS, 1);
        
        //check rxdata[5] error bit(1) and complete bit(0) ,TBD
		if((rxdata[PSOC_I2C_STATUS] & 0x2) == 0x2) {
		    //printk("i2c bridge fail!!!\n");
			timeout = 0;
			break;
		}
		if((rxdata[PSOC_I2C_STATUS] & PSOC_I2C_START) == 0) {
		    /* comand complete */
            psoc_i2c_read(client, &rxdata[PSOC_I2C_DATA+alen], PSOC_I2C_DATA+alen, len);
			break;	
		}
		
        //delay
        msleep(20);
    } while(timeout--);

 	if(timeout <= 0) {
 	    return -1;
 	}
    
    //---------------------------------------------------------------------
    
    for(index=0; index < len; index++) {
        data[index] = rxdata[PSOC_I2C_DATA + alen + index];
    }
    
    return 0;
}
#endif
static ssize_t show_clei(struct device *dev, struct device_attribute *da,
                         char *buf)
{
        u16 status;
        struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
        struct i2c_client *client = to_i2c_client(dev);
        struct psoc_data *data = i2c_get_clientdata(client);
	u8 device_index = attr->index & 0xFF;
        u8 clei_index   = (attr->index >> 8) & 0xFF;
	u16 offset = PSOC_OFF(clei) + sizeof(struct clei)* device_index;
	u8 len;
        u8 rxbuf[21] = {0};
        u8 rxbuf1[21] = {0};
        u8 counter = 0;

	switch (clei_index)
	{
		case ISSUENUMBER_INDEX:
			offset += CLEI_OFF(issue_number);
			len = sizeof((struct clei){0}.issue_number);
			break;
		case ABBREVIATION_INDEX:
                        offset += CLEI_OFF(abbreviation_number);
                        len = sizeof((struct clei){0}.abbreviation_number);
                        break;
		case FCNUMBER_INDEX:
                        offset += CLEI_OFF(fc_number);
                        len = sizeof((struct clei){0}.fc_number);
                        break;
                case CLEICODE_INDEX:
                        offset += CLEI_OFF(clei_code);
                        len = sizeof((struct clei){0}.clei_code);
                        break;
                case PRODUCTDATE_INDEX:
                        offset += CLEI_OFF(product_year_and_month);
                        len = sizeof((struct clei){0}.product_year_and_month);
                        break;
                case LABELLOCATION_INDX:
                        offset += CLEI_OFF(label_location_code);
                        len = sizeof((struct clei){0}.label_location_code);
                        break;
                case SERIALNUMBER_INDEX:
                        offset += CLEI_OFF(serial_number);
                        len = sizeof((struct clei){0}.serial_number);
                        break;
                case PCBREVISION_INDEX:
                        offset += CLEI_OFF(pcb_revision);
                        len = sizeof((struct clei){0}.pcb_revision);
                        break;
		default:
			return 0;
	}

        do
        {
           mutex_lock(&data->update_lock);
           status = psoc_i2c_read(client,rxbuf,offset,len);
           mutex_unlock(&data->update_lock);

           mutex_lock(&data->update_lock);
           status = psoc_i2c_read(client,rxbuf1,offset,len);
           mutex_unlock(&data->update_lock);

           counter++;
        }while((strcmp(rxbuf,rxbuf1)!=0) || ((strlen(rxbuf)==0) && (counter <5)));

	//return sprintf(buf, "show_clei: offset:%d len:%d \n",offset,len);

	if(rxbuf[0]==0x00 || rxbuf[0]==0xFF) return 0;

	return sprintf(buf, "%s \n",rxbuf);
}


/*
CPLD report the PSU0 status
000 = PSU normal operation
100 = PSU fault
010 = PSU unpowered
111 = PSU not installed

7 6 | 5 4 3 |  2 1 0
----------------------
    | psu1  |  psu0
*/
static char* psu_str[] = {
    "normal",           //000
    "NA",               //001
    "unpowered",        //010
    "NA",               //011
    "fault",            //100
    "NA",               //101
    "NA",               //110
    "not installed",    //111
};

static ssize_t show_psu_st(struct device *dev, struct device_attribute *da,
			 char *buf)
{
	u32 status;
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	struct i2c_client *client = to_i2c_client(dev);
	struct psoc_data *data = i2c_get_clientdata(client);
	u8 byte;
	int shift = (attr->index == 0)?3:0;
  
	mutex_lock(&data->update_lock);
    status = psoc_i2c_read(client, &byte, PSOC_PSU_OFFSET, 1);
	mutex_unlock(&data->update_lock);

    byte = (byte >> shift) & 0x7;

	status = sprintf (buf, "%d : %s\n", byte, psu_str[byte]);
    
	return strlen(buf);
}

static ssize_t psoc_show_psu_st(char *buf, int psu_index)
{
	u32 status;
	struct i2c_client *client = NULL;
	struct psoc_data *data = NULL;
	u8 byte;
	int shift = (psu_index == 0)?3:0;

	if (!psoc_led_client_dev) {
	    return 0;
	}

	client = to_i2c_client(psoc_led_client_dev);
	data = i2c_get_clientdata(client);

	mutex_lock(&data->update_lock);
    status = psoc_i2c_read(client, &byte, PSOC_PSU_OFFSET, 1);
	mutex_unlock(&data->update_lock);

    byte = (byte >> shift) & 0x7;

	status = sprintf (buf, "%d : %s\n", byte, psu_str[byte]);

	return strlen(buf);
}

ssize_t psoc_show_psu_state(char *buf, int index)
{
	return psoc_show_psu_st(buf, index);
}
EXPORT_SYMBOL(psoc_show_psu_state);

/*-----------------------------------------------------------------------*/

/* sysfs attributes for hwmon */

static ssize_t show_thermal(struct device *dev, struct device_attribute *da,
			 char *buf)
{
	u16 status;
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	struct i2c_client *client = to_i2c_client(dev);
	struct psoc_data *data = i2c_get_clientdata(client);
	u16 offset = attr->index * 2 + THERMAL_OFFSET;
    
	mutex_lock(&data->update_lock);
	
	status = psoc_read16(client, offset);
	
	mutex_unlock(&data->update_lock);
	
	return sprintf(buf, "%d\n",
		       (s8)(status>>8) * 1000  );
}


static ssize_t show_pwm(struct device *dev, struct device_attribute *da,
			 char *buf)
{
	int status;
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	struct i2c_client *client = to_i2c_client(dev);
	struct psoc_data *data = i2c_get_clientdata(client);
	u8 offset = attr->index  + PWM_OFFSET;
    
	mutex_lock(&data->update_lock);
	
	status = psoc_read8(client, offset);
	
	mutex_unlock(&data->update_lock);
	
	return sprintf(buf, "%d\n",
		       status);
}

static ssize_t set_pwm(struct device *dev,
			   struct device_attribute *devattr,
			   const char *buf, size_t count)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
	struct i2c_client *client = to_i2c_client(dev);
	struct psoc_data *data = i2c_get_clientdata(client);
	u8 offset = attr->index  + PWM_OFFSET;

	u8 pwm = simple_strtol(buf, NULL, 10);
	if(pwm > 255) pwm = 255;
	
	if(data->diag) {    
    	mutex_lock(&data->update_lock);
    	psoc_i2c_write(client, &pwm, offset, 1);
    	mutex_unlock(&data->update_lock);
    }
	
	return count;
}


static ssize_t show_rpm(struct device *dev, struct device_attribute *da,
			 char *buf)
{
	int status;
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	struct i2c_client *client = to_i2c_client(dev);
	struct psoc_data *data = i2c_get_clientdata(client);
	u8 offset = attr->index*2  + RPM_OFFSET;
    
	mutex_lock(&data->update_lock);
	status = psoc_read16(client, offset);
	mutex_unlock(&data->update_lock);
	
	return sprintf(buf, "%d\n",
		       status);
}

static ssize_t psoc_show_rpm(char *buf, int fan_index)
{
	int status;
	struct i2c_client *client = NULL;
	struct psoc_data *data = NULL;
	u8 offset = fan_index*2  + RPM_OFFSET;

	if (!psoc_led_client_dev) {
	    return 0;
	}

	client = to_i2c_client(psoc_led_client_dev);
	data = i2c_get_clientdata(client);

	mutex_lock(&data->update_lock);
	status = psoc_read16(client, offset);
	mutex_unlock(&data->update_lock);

	return sprintf(buf, "%d\n",
		       status);
}

ssize_t psoc_show_fan_input(char *buf, int index)
{
	return psoc_show_rpm(buf, index);
}
EXPORT_SYMBOL(psoc_show_fan_input);

static ssize_t show_switch_tmp(struct device *dev, struct device_attribute *da,
			 char *buf)
{
	u16 status;
	struct i2c_client *client = to_i2c_client(dev);
	struct psoc_data *data = i2c_get_clientdata(client);
	u16 temp = 0;
    
	mutex_lock(&data->update_lock);
    status = psoc_i2c_read(client, (u8*)&temp, SWITCH_TMP_OFFSET, 2);
	mutex_unlock(&data->update_lock);
	
	status = sprintf (buf, "%d\n",  (s8)(temp>>8) * 1000  );
	    
	return strlen(buf);
}

static ssize_t set_switch_tmp(struct device *dev,
			   struct device_attribute *devattr,
			   const char *buf, size_t count)
{
	//struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
	struct i2c_client *client = to_i2c_client(dev);
	struct psoc_data *data = i2c_get_clientdata(client);

	long temp = simple_strtol(buf, NULL, 10);
    u16 temp2 =  ( (temp/1000) <<8 ) & 0xFF00 ;
    
    //printk("set_switch_tmp temp=%d, temp2=0x%x (%x,%x)\n", temp, temp2, ( ( (temp/1000) <<8 ) & 0xFF00 ),  (( (temp%1000) / 10 ) & 0xFF));
    
	mutex_lock(&data->update_lock);
	psoc_i2c_write(client, (u8*)&temp2, SWITCH_TMP_OFFSET, 2);
	mutex_unlock(&data->update_lock);
	
	return count;
}

static ssize_t show_diag(struct device *dev, struct device_attribute *da,
			 char *buf)
{
	u16 status;
	struct i2c_client *client = to_i2c_client(dev);
	struct psoc_data *data = i2c_get_clientdata(client);
	u8 diag_flag = 0;
    
	mutex_lock(&data->update_lock);
    status = psoc_i2c_read(client, (u8*)&diag_flag, DIAG_FLAG_OFFSET, 1);
	mutex_unlock(&data->update_lock);
	
	data->diag = (diag_flag & 0x80)?1:0;
	status = sprintf (buf, "%d\n", data->diag);
	    
	return strlen(buf);
}

static ssize_t set_diag(struct device *dev,
			   struct device_attribute *devattr,
			   const char *buf, size_t count)
{
	//struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
	struct i2c_client *client = to_i2c_client(dev);
	struct psoc_data *data = i2c_get_clientdata(client);
	u8 value = 0;
	u8 diag = simple_strtol(buf, NULL, 10);
	
    diag = diag?1:0;
	data->diag = diag;
	    
	mutex_lock(&data->update_lock);
	psoc_i2c_read(client, (u8*)&value, DIAG_FLAG_OFFSET, 1);
	if(diag) value |= (1<<7);
	else     value &= ~(1<<7);
	psoc_i2c_write(client, (u8*)&value, DIAG_FLAG_OFFSET, 1);
	mutex_unlock(&data->update_lock);
	
	return count;
}

ssize_t psoc_show_diag(char *buf)
{
	u16 status;
	struct i2c_client *client = NULL;
	struct psoc_data *data = NULL;
	u8 diag_flag = 0;

	if (!psoc_led_client_dev) {
	    return 0;
	}

	client = to_i2c_client(psoc_led_client_dev);
	data = i2c_get_clientdata(client);

	mutex_lock(&data->update_lock);
    status = psoc_i2c_read(client, (u8*)&diag_flag, DIAG_FLAG_OFFSET, 1);
	mutex_unlock(&data->update_lock);

	data->diag = (diag_flag & 0x80)?1:0;
	status = sprintf (buf, "%d\n", data->diag);

	return strlen(buf);
}
EXPORT_SYMBOL(psoc_show_diag);

ssize_t psoc_set_diag(const char *buf, size_t count)
{
	struct i2c_client *client = NULL;
	struct psoc_data *data = NULL;
	u8 value = 0;
	u8 diag = simple_strtol(buf, NULL, 10);

	if (!psoc_led_client_dev) {
	    return 0;
	}

	client = to_i2c_client(psoc_led_client_dev);
	data = i2c_get_clientdata(client);

    diag = diag?1:0;
	data->diag = diag;

	mutex_lock(&data->update_lock);
	psoc_i2c_read(client, (u8*)&value, DIAG_FLAG_OFFSET, 1);
	if(diag) value |= (1<<7);
	else     value &= ~(1<<7);
	psoc_i2c_write(client, (u8*)&value, DIAG_FLAG_OFFSET, 1);
	mutex_unlock(&data->update_lock);

	return count;
}
EXPORT_SYMBOL(psoc_set_diag);

static ssize_t show_version(struct device *dev, struct device_attribute *da,
			 char *buf)
{
	u16 status;
	//struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	struct i2c_client *client = to_i2c_client(dev);
	struct psoc_data *data = i2c_get_clientdata(client);
    
	mutex_lock(&data->update_lock);
	
	status = psoc_read16(client, VERSION_OFFSET);
	
	mutex_unlock(&data->update_lock);
	
	return sprintf(buf, "ver: %x.%x\n", (status & 0xFF00)>>8,  (status & 0xFF) );
}


static ssize_t show_fan_led(struct device *dev, struct device_attribute *da,
			 char *buf)
{
	int status;
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	struct i2c_client *client = to_i2c_client(dev);
	struct psoc_data *data = i2c_get_clientdata(client);
	u8 bit = attr->index;
    
	mutex_lock(&data->update_lock);
	
	status = psoc_read8(client, FAN_LED_OFFSET);
	
	mutex_unlock(&data->update_lock);
	
	return sprintf(buf, "%d\n",
		       (status & (1<<bit))?1:0 );
}

static ssize_t set_fan_led(struct device *dev,
			   struct device_attribute *devattr,
			   const char *buf, size_t count)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
	struct i2c_client *client = to_i2c_client(dev);
	struct psoc_data *data = i2c_get_clientdata(client);
	u8 bit = attr->index;
	u8 led_state = 0;

	u8 v = simple_strtol(buf, NULL, 10);
	
	if(data->diag) {    
    	mutex_lock(&data->update_lock);
    	led_state = psoc_read8(client, FAN_LED_OFFSET);
    	if(v) led_state |=  (1<<bit);
    	else  led_state &= ~(1<<bit);    
    	psoc_i2c_write(client, &led_state, FAN_LED_OFFSET, 1);
    	mutex_unlock(&data->update_lock);
    }
	
	return count;
}

static ssize_t show_value8(struct device *dev, struct device_attribute *da,
			 char *buf)
{
	int status;
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	struct i2c_client *client = to_i2c_client(dev);
	struct psoc_data *data = i2c_get_clientdata(client);
	u8 offset = attr->index;
    
	mutex_lock(&data->update_lock);
	
	status = psoc_read8(client, offset);
	
	mutex_unlock(&data->update_lock);
	
	return sprintf(buf, "0x%02X\n", status );
}

ssize_t psoc_show_value8(char *buf, int offset)
{
	int status;
	struct i2c_client *client = NULL;
	struct psoc_data *data = NULL;

	if (!psoc_led_client_dev) {
	    return 0;
	}

	client = to_i2c_client(psoc_led_client_dev);
	data = i2c_get_clientdata(client);

	mutex_lock(&data->update_lock);
	status = psoc_read8(client, offset);
	mutex_unlock(&data->update_lock);

	return sprintf(buf, "0x%02X\n", status );
}

static char prev_fan_state[8] = { 0 };

ssize_t psoc_show_fan_state(char *buf)
{
    int i;
    int rv = psoc_show_value8(buf, FAN_GPI_OFFSET);
    for (i = 0; i < 2; i++) {
	if (strncmp(prev_fan_state, buf, 4) == 0) {
	    return rv;
	}
	msleep(50);
	rv = psoc_show_value8(buf, FAN_GPI_OFFSET);
    }
    strcpy(prev_fan_state, buf);
    return rv;
}
EXPORT_SYMBOL(psoc_show_fan_state);

static long pmbus_reg2data_linear(int data, int linear16)
{
    s16 exponent;
    s32 mantissa;
    long val;

    if (linear16) { /* LINEAR16 */
        exponent = -9;
        mantissa = (u16) data;
    } else {  /* LINEAR11 */
        exponent = ((s16)data) >> 11;
        exponent = ((s16)( data & 0xF800) ) >> 11;
        mantissa = ((s32)((data & 0x7ff) << 5)) >> 5;
    }
    
    //printk("data=%d,  m=%d, e=%d\n", data, exponent, mantissa);
    val = mantissa;

    /* scale result to micro-units for power sensors */
    val = val * 1000L;

    if (exponent >= 0)
        val <<= exponent;
    else
        val >>= -exponent;

    return val;
}

#if 0
static ssize_t show_psu(struct device *dev, struct device_attribute *da,
			 char *buf)
{
	int status;
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	struct i2c_client *client = to_i2c_client(dev);
	struct psoc_data *data = i2c_get_clientdata(client);
	u8 reg =  attr->index & 0xFF;
	u8 chip = (attr->index >> 16)? 0x59:0x58;
	u8 bus  = 1;
	unsigned char value[2] = {0,0};;    
    
	mutex_lock(&data->update_lock);
	psoc_i2c_bridge_read(client, bus, chip, &reg, 1, value, 2);
	mutex_unlock(&data->update_lock);
	
	status =  value[1]<<8 | value[0];
	//status1 =  value[1]<<8 | value[0];
	
	return sprintf(buf, "%ld\n", pmbus_reg2data_linear(status, (reg==PMBUS_READ_VOUT)?1:0) );
}
#endif

static ssize_t show_psu_psoc(struct device *dev, struct device_attribute *da,
			 char *buf)
{
	u16 status;
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	struct i2c_client *client = to_i2c_client(dev);
	struct psoc_data *data = i2c_get_clientdata(client);
	u8 offset = (attr->index & 0xFF) + PSU_INFO_OFFSET;
        u8 len = (attr->index >> 8)& 0xFF;
        u8 rxbuf[21] = {0};
        u8 rxbuf1[21] = {0};
        u8 counter = 0;
        u8 count = 0;
        u8 index = 0;
        if (len == 2)
        {
       	   mutex_lock(&data->update_lock);
	   status = psoc_read16(client, offset);
 	   mutex_unlock(&data->update_lock);
	
	   return sprintf(buf, "%ld \n", pmbus_reg2data_linear(status, strstr(attr->dev_attr.attr.name, "vout")? 1:0 ));
        }
        else
        {
           do 
           {
           mutex_lock(&data->update_lock);
           status = psoc_i2c_read(client,rxbuf,offset,len);
           mutex_unlock(&data->update_lock);
           mutex_lock(&data->update_lock);
           status = psoc_i2c_read(client,rxbuf1,offset,len);
           mutex_unlock(&data->update_lock);
           counter++;
           }while((strcmp(rxbuf,rxbuf1)!=0) || ((strlen(rxbuf)==0) && (counter <5)));
           for(index = 0; index < len; index++)
           {
              if(rxbuf[index] == 0xFF)
              count++;
           }
	   if(len == count)
           return sprintf(buf, "%s \n","NA");
           if(status ==0)
           return sprintf(buf,"%s \n", "NA");
           else if((strlen(rxbuf)==0))
           return sprintf(buf, "%s \n", "N/A");
           else
           return sprintf(buf, "%s \n",rxbuf);
        }
}

static ssize_t psoc_show_psu_psoc(char *buf, int psu_index, char *attr_name)
{
	u16 status;
	struct i2c_client *client = NULL;
	struct psoc_data *data = NULL;
	u8 offset = psu_index + PSU_INFO_OFFSET;

	if (!psoc_led_client_dev) {
	    return 0;
	}

	client = to_i2c_client(psoc_led_client_dev);
	data = i2c_get_clientdata(client);

	mutex_lock(&data->update_lock);
	status = psoc_read16(client, offset);
	mutex_unlock(&data->update_lock);

	return sprintf(buf, "%ld \n", pmbus_reg2data_linear(status, strstr(attr_name, "vout")? 1:0 ));
}

ssize_t psoc_show_psu_vin(char *buf, int index)
{
	return psoc_show_psu_psoc(buf, index, "vin");
}
EXPORT_SYMBOL(psoc_show_psu_vin);

#define PSOC_POLLING_PERIOD 1000
#define PSU_NOT_INSTALLED   7
#define PSOC_WARNING(fmt, args...) printk( KERN_WARNING "[PSOC]" fmt, ##args)
static u8 psu_info[PSU_NUM];
static u8 fan_info[FAN_NUM];
static void get_init_psu_fan_status(struct i2c_client *client)
{
    struct psoc_data *data = i2c_get_clientdata(client);
    u32 status = 0;
    u8 byte = 0;
    int i = 0;
    mutex_lock(&data->update_lock);
    status = psoc_i2c_read(client, &byte, PSOC_PSU_OFFSET,1);
    mutex_unlock(&data->update_lock);
    psu_info[0] = (byte >> 3) & 0x7;
    psu_info[1] = (byte >> 0) & 0x7;
    mutex_lock(&data->update_lock);
    status = psoc_read8(client, FAN_GPI_OFFSET);
    mutex_unlock(&data->update_lock);
    for (i=0; i<2 ; i++)
    {
        fan_info[i] = ((status >> i) & 0x01);
    }
}

static void check_fan_status(struct i2c_client *client)
{
    struct psoc_data *data = i2c_get_clientdata(client);
    u32 status;
    int i = 0;
    u8 rst = 0;
    mutex_lock(&data->update_lock);
    status = psoc_read8(client, FAN_GPI_OFFSET);
    mutex_unlock(&data->update_lock);
    for (i = 0; i < FAN_NUM; i++)
    {
        rst = ((status >> i) & 0x01);
        if(rst != fan_info[i])
        {
            fan_info[i] = rst;
            if (rst)
            PSOC_WARNING("Detect FAN%d is removed \n",(i+1));
            else
            PSOC_WARNING("Detect FAN%d is present \n",(i+1));
        }
    }
}
#if 0
// [Remove] Bacuase cottonwood don't have PSU info
static void check_psu_status(struct i2c_client *client)
{
    static u8 psu_retry[]={0,0};
    struct psoc_data *data = i2c_get_clientdata(client);
    u32 status;
    u8 byte = 0;
    u8 psu1, psu2;
    mutex_lock(&data->update_lock);
    status = psoc_i2c_read(client, &byte, PSOC_PSU_OFFSET, 1);
    mutex_unlock(&data->update_lock);
    psu1 = (byte >> 3) & 0x7;
    psu2 = (byte >> 0) & 0x7;
    if(psu1 != psu_info[0])
    {
	if ((psu_retry[0]++)==0) { return; }
        if (psu1 == PSU_NOT_INSTALLED)
        PSOC_WARNING("Detect PSU1 is removed \n");
        else
        PSOC_WARNING("Detect PSU1 is present \n");
 
        psu_info[0] = psu1;
	psu_retry[0] = 0;
    }
    if(psu2 != psu_info[1])
    {
	if((psu_retry[1]++)==0) { return; }
        if(psu2 == PSU_NOT_INSTALLED)
        PSOC_WARNING("Detect PSU2 is removed \n");
        else
        PSOC_WARNING("Detect PSU2 is present \n");

        psu_info[1] = psu2;
        psu_retry[1] = 0;
    }
}
#endif
static void check_switch_temp(struct i2c_client * client)
{
	struct psoc_data *data = i2c_get_clientdata(client);
	static struct file *f;
	mm_segment_t old_fs;

	set_fs(get_ds());
	f = filp_open(SWITCH_TEMPERATURE_SOCK,O_RDONLY,0644);
	if(IS_ERR(f)) { 
		return; 
	}
	else { 
		char temp_str[]={0,0,0,0,0,0,0};
		loff_t pos = 0;
		u16 temp2 = 0;
		old_fs = get_fs();
		set_fs(KERNEL_DS);
		vfs_read(f, temp_str,6,&pos);
		temp2 = ((simple_strtoul(temp_str,NULL,10)/1000) <<8 ) & 0xFF00 ; 
                mutex_lock(&data->update_lock);
                psoc_i2c_write(client, (u8*)&temp2, SWITCH_TMP_OFFSET, 2);
                mutex_unlock(&data->update_lock);
	}
	filp_close(f,NULL);
	set_fs(old_fs);
}


static int psoc_polling_thread(void *p)
{
    struct i2c_client *client = p;
    get_init_psu_fan_status(client); 
    while (!kthread_should_stop())
    {
//        check_psu_status(client);
//        check_fan_status(client);
        check_switch_temp(client);
        set_current_state(TASK_INTERRUPTIBLE);
        if(kthread_should_stop())
            break;
 
        schedule_timeout(msecs_to_jiffies(PSOC_POLLING_PERIOD));
    }
    return 0;
}


static SENSOR_DEVICE_ATTR(temp1_input, S_IRUGO,			show_thermal, 0, 0);
static SENSOR_DEVICE_ATTR(temp2_input, S_IRUGO,			show_thermal, 0, 1);
static SENSOR_DEVICE_ATTR(temp3_input, S_IRUGO,			show_thermal, 0, 2);
static SENSOR_DEVICE_ATTR(temp4_input, S_IRUGO,			show_thermal, 0, 3);
static SENSOR_DEVICE_ATTR(temp5_input, S_IRUGO,			show_thermal, 0, 4);
static SENSOR_DEVICE_ATTR(thermal_psu1, S_IRUGO,		show_thermal, 0, 5);
static SENSOR_DEVICE_ATTR(thermal_psu2, S_IRUGO,		show_thermal, 0, 6);

static SENSOR_DEVICE_ATTR(pwm1, S_IWUSR|S_IRUGO,			show_pwm, set_pwm, 0);
static SENSOR_DEVICE_ATTR(pwm2, S_IWUSR|S_IRUGO,			show_pwm, set_pwm, 1);
static SENSOR_DEVICE_ATTR(pwm3, S_IWUSR|S_IRUGO,			show_pwm, set_pwm, 2);
static SENSOR_DEVICE_ATTR(pwm4, S_IWUSR|S_IRUGO,			show_pwm, set_pwm, 3);
static SENSOR_DEVICE_ATTR(pwm_psu1, S_IWUSR|S_IRUGO,		show_pwm, set_pwm, 4);
static SENSOR_DEVICE_ATTR(pwm_psu2, S_IWUSR|S_IRUGO,		show_pwm, set_pwm, 5);

static SENSOR_DEVICE_ATTR(psu0,  S_IRUGO,			        show_psu_st, 0, 0);
static SENSOR_DEVICE_ATTR(psu1,  S_IRUGO,			        show_psu_st, 0, 1);

static SENSOR_DEVICE_ATTR(fan1_input, S_IRUGO,			show_rpm, 0, 0);
static SENSOR_DEVICE_ATTR(fan2_input, S_IRUGO,			show_rpm, 0, 1);
static SENSOR_DEVICE_ATTR(fan3_input, S_IRUGO,			show_rpm, 0, 2);
static SENSOR_DEVICE_ATTR(fan4_input, S_IRUGO,			show_rpm, 0, 3);
static SENSOR_DEVICE_ATTR(fan5_input, S_IRUGO,			show_rpm, 0, 4);
static SENSOR_DEVICE_ATTR(fan6_input, S_IRUGO,			show_rpm, 0, 5);
static SENSOR_DEVICE_ATTR(fan7_input, S_IRUGO,			show_rpm, 0, 6);
static SENSOR_DEVICE_ATTR(fan8_input, S_IRUGO,			show_rpm, 0, 7);

//CLEI
static SENSOR_DEVICE_ATTR(fan1_issue_number, S_IRUGO, show_clei, 0, FAN1_CLEI_INDEX | ISSUENUMBER_INDEX << 8 );
static SENSOR_DEVICE_ATTR(fan2_issue_number, S_IRUGO, show_clei, 0, FAN2_CLEI_INDEX | ISSUENUMBER_INDEX << 8 );
static SENSOR_DEVICE_ATTR(fan3_issue_number, S_IRUGO, show_clei, 0, FAN3_CLEI_INDEX | ISSUENUMBER_INDEX << 8 );
static SENSOR_DEVICE_ATTR(fan4_issue_number, S_IRUGO, show_clei, 0, FAN4_CLEI_INDEX | ISSUENUMBER_INDEX << 8 );
static SENSOR_DEVICE_ATTR(fan1_abbreviation_number, S_IRUGO, show_clei, 0, FAN1_CLEI_INDEX | ABBREVIATION_INDEX << 8 );
static SENSOR_DEVICE_ATTR(fan2_abbreviation_number, S_IRUGO, show_clei, 0, FAN2_CLEI_INDEX | ABBREVIATION_INDEX << 8 );
static SENSOR_DEVICE_ATTR(fan3_abbreviation_number, S_IRUGO, show_clei, 0, FAN3_CLEI_INDEX | ABBREVIATION_INDEX << 8 );
static SENSOR_DEVICE_ATTR(fan4_abbreviation_number, S_IRUGO, show_clei, 0, FAN4_CLEI_INDEX | ABBREVIATION_INDEX << 8 );
static SENSOR_DEVICE_ATTR(fan1_fc_number, S_IRUGO, show_clei, 0, FAN1_CLEI_INDEX | FCNUMBER_INDEX << 8 );
static SENSOR_DEVICE_ATTR(fan2_fc_number, S_IRUGO, show_clei, 0, FAN2_CLEI_INDEX | FCNUMBER_INDEX << 8 );
static SENSOR_DEVICE_ATTR(fan3_fc_number, S_IRUGO, show_clei, 0, FAN3_CLEI_INDEX | FCNUMBER_INDEX << 8 );
static SENSOR_DEVICE_ATTR(fan4_fc_number, S_IRUGO, show_clei, 0, FAN4_CLEI_INDEX | FCNUMBER_INDEX << 8 );
static SENSOR_DEVICE_ATTR(fan1_clei_code, S_IRUGO, show_clei, 0, FAN1_CLEI_INDEX | CLEICODE_INDEX << 8 );
static SENSOR_DEVICE_ATTR(fan2_clei_code, S_IRUGO, show_clei, 0, FAN2_CLEI_INDEX | CLEICODE_INDEX << 8 );
static SENSOR_DEVICE_ATTR(fan3_clei_code, S_IRUGO, show_clei, 0, FAN3_CLEI_INDEX | CLEICODE_INDEX << 8 );
static SENSOR_DEVICE_ATTR(fan4_clei_code, S_IRUGO, show_clei, 0, FAN4_CLEI_INDEX | CLEICODE_INDEX << 8 );
static SENSOR_DEVICE_ATTR(fan1_product_year_and_month, S_IRUGO, show_clei, 0, FAN1_CLEI_INDEX | PRODUCTDATE_INDEX << 8 );
static SENSOR_DEVICE_ATTR(fan2_product_year_and_month, S_IRUGO, show_clei, 0, FAN2_CLEI_INDEX | PRODUCTDATE_INDEX << 8 );
static SENSOR_DEVICE_ATTR(fan3_product_year_and_month, S_IRUGO, show_clei, 0, FAN3_CLEI_INDEX | PRODUCTDATE_INDEX << 8 );
static SENSOR_DEVICE_ATTR(fan4_product_year_and_month, S_IRUGO, show_clei, 0, FAN4_CLEI_INDEX | PRODUCTDATE_INDEX << 8 );
static SENSOR_DEVICE_ATTR(fan1_label_location_code, S_IRUGO, show_clei, 0, FAN1_CLEI_INDEX | LABELLOCATION_INDX << 8 );
static SENSOR_DEVICE_ATTR(fan2_label_location_code, S_IRUGO, show_clei, 0, FAN2_CLEI_INDEX | LABELLOCATION_INDX << 8 );
static SENSOR_DEVICE_ATTR(fan3_label_location_code, S_IRUGO, show_clei, 0, FAN3_CLEI_INDEX | LABELLOCATION_INDX << 8 );
static SENSOR_DEVICE_ATTR(fan4_label_location_code, S_IRUGO, show_clei, 0, FAN4_CLEI_INDEX | LABELLOCATION_INDX << 8 );
static SENSOR_DEVICE_ATTR(fan1_serial_number, S_IRUGO, show_clei, 0, FAN1_CLEI_INDEX | SERIALNUMBER_INDEX << 8 );
static SENSOR_DEVICE_ATTR(fan2_serial_number, S_IRUGO, show_clei, 0, FAN2_CLEI_INDEX | SERIALNUMBER_INDEX << 8 );
static SENSOR_DEVICE_ATTR(fan3_serial_number, S_IRUGO, show_clei, 0, FAN3_CLEI_INDEX | SERIALNUMBER_INDEX << 8 );
static SENSOR_DEVICE_ATTR(fan4_serial_number, S_IRUGO, show_clei, 0, FAN4_CLEI_INDEX | SERIALNUMBER_INDEX << 8 );
static SENSOR_DEVICE_ATTR(fan1_pcb_revision, S_IRUGO, show_clei, 0, FAN1_CLEI_INDEX | PCBREVISION_INDEX << 8 );
static SENSOR_DEVICE_ATTR(fan2_pcb_revision, S_IRUGO, show_clei, 0, FAN2_CLEI_INDEX | PCBREVISION_INDEX << 8 );
static SENSOR_DEVICE_ATTR(fan3_pcb_revision, S_IRUGO, show_clei, 0, FAN3_CLEI_INDEX | PCBREVISION_INDEX << 8 );
static SENSOR_DEVICE_ATTR(fan4_pcb_revision, S_IRUGO, show_clei, 0, FAN4_CLEI_INDEX | PCBREVISION_INDEX << 8 );

static SENSOR_DEVICE_ATTR(rpm_psu1, S_IRUGO,		show_rpm, 0, 8);
static SENSOR_DEVICE_ATTR(rpm_psu2, S_IRUGO,		show_rpm, 0, 9);

static SENSOR_DEVICE_ATTR(switch_tmp, S_IWUSR|S_IRUGO,			show_switch_tmp, set_switch_tmp, 0);

static SENSOR_DEVICE_ATTR(diag, S_IWUSR|S_IRUGO,			show_diag, set_diag, 0);
static SENSOR_DEVICE_ATTR(version, S_IRUGO,			show_version, 0, 0);

static SENSOR_DEVICE_ATTR(fan1_led_grn, S_IWUSR|S_IRUGO,			show_fan_led, set_fan_led, 0);
static SENSOR_DEVICE_ATTR(fan2_led_grn, S_IWUSR|S_IRUGO,			show_fan_led, set_fan_led, 1);
static SENSOR_DEVICE_ATTR(fan3_led_grn, S_IWUSR|S_IRUGO,			show_fan_led, set_fan_led, 2);
static SENSOR_DEVICE_ATTR(fan4_led_grn, S_IWUSR|S_IRUGO,			show_fan_led, set_fan_led, 3);
static SENSOR_DEVICE_ATTR(fan1_led_red, S_IWUSR|S_IRUGO,			show_fan_led, set_fan_led, 4);
static SENSOR_DEVICE_ATTR(fan2_led_red, S_IWUSR|S_IRUGO,			show_fan_led, set_fan_led, 5);
static SENSOR_DEVICE_ATTR(fan3_led_red, S_IWUSR|S_IRUGO,			show_fan_led, set_fan_led, 6);
static SENSOR_DEVICE_ATTR(fan4_led_red, S_IWUSR|S_IRUGO,			show_fan_led, set_fan_led, 7);

static SENSOR_DEVICE_ATTR(fan_gpi,      S_IRUGO,			        show_value8,  0,           FAN_GPI_OFFSET);
#if 0
static SENSOR_DEVICE_ATTR(psu1_vin,      S_IRUGO,			        show_psu,  0,           PMBUS_READ_VIN);
static SENSOR_DEVICE_ATTR(psu1_vout,     S_IRUGO,			        show_psu,  0,           PMBUS_READ_VOUT);
static SENSOR_DEVICE_ATTR(psu1_iin,      S_IRUGO,			        show_psu,  0,           PMBUS_READ_IIN);
static SENSOR_DEVICE_ATTR(psu1_iout,     S_IRUGO,			        show_psu,  0,           PMBUS_READ_IOUT);
static SENSOR_DEVICE_ATTR(psu1_pin,      S_IRUGO,			        show_psu,  0,           PMBUS_READ_PIN);
static SENSOR_DEVICE_ATTR(psu1_pout,     S_IRUGO,			        show_psu,  0,           PMBUS_READ_POUT);


static SENSOR_DEVICE_ATTR(psu2_vin,      S_IRUGO,			        show_psu,  0,           (1<<16) | PMBUS_READ_VIN);
static SENSOR_DEVICE_ATTR(psu2_vout,     S_IRUGO,			        show_psu,  0,           (1<<16) | PMBUS_READ_VOUT);
static SENSOR_DEVICE_ATTR(psu2_iin,      S_IRUGO,			        show_psu,  0,           (1<<16) | PMBUS_READ_IIN);
static SENSOR_DEVICE_ATTR(psu2_iout,     S_IRUGO,			        show_psu,  0,           (1<<16) | PMBUS_READ_IOUT);
static SENSOR_DEVICE_ATTR(psu2_pin,      S_IRUGO,			        show_psu,  0,           (1<<16) | PMBUS_READ_PIN);
static SENSOR_DEVICE_ATTR(psu2_pout,     S_IRUGO,			        show_psu,  0,           (1<<16) | PMBUS_READ_POUT);
#endif

static SENSOR_DEVICE_ATTR(psoc_psu1_vin,      S_IRUGO,			        show_psu_psoc,  0,      (0x02<<8)|PSOC_PSU_OFF(psu1_vin));
static SENSOR_DEVICE_ATTR(psoc_psu1_vout,     S_IRUGO,			        show_psu_psoc,  0,      (0x02<<8)|PSOC_PSU_OFF(psu1_vout));
static SENSOR_DEVICE_ATTR(psoc_psu1_iin,      S_IRUGO,			        show_psu_psoc,  0,      (0x02<<8)|PSOC_PSU_OFF(psu1_iin));
static SENSOR_DEVICE_ATTR(psoc_psu1_iout,     S_IRUGO,			        show_psu_psoc,  0,      (0x02<<8)|PSOC_PSU_OFF(psu1_iout));
static SENSOR_DEVICE_ATTR(psoc_psu1_pin,      S_IRUGO,			        show_psu_psoc,  0,      (0x02<<8)|PSOC_PSU_OFF(psu1_pin));
static SENSOR_DEVICE_ATTR(psoc_psu1_pout,     S_IRUGO,			        show_psu_psoc,  0,      (0x02<<8)|PSOC_PSU_OFF(psu1_pout));


static SENSOR_DEVICE_ATTR(psoc_psu2_vin,      S_IRUGO,			        show_psu_psoc,  0,      (0x02<<8)|PSOC_PSU_OFF(psu2_vin)); 
static SENSOR_DEVICE_ATTR(psoc_psu2_vout,     S_IRUGO,			        show_psu_psoc,  0,      (0x02<<8)|PSOC_PSU_OFF(psu2_vout));
static SENSOR_DEVICE_ATTR(psoc_psu2_iin,      S_IRUGO,			        show_psu_psoc,  0,      (0x02<<8)|PSOC_PSU_OFF(psu2_iin)); 
static SENSOR_DEVICE_ATTR(psoc_psu2_iout,     S_IRUGO,			        show_psu_psoc,  0,      (0x02<<8)|PSOC_PSU_OFF(psu2_iout));
static SENSOR_DEVICE_ATTR(psoc_psu2_pin,      S_IRUGO,			        show_psu_psoc,  0,      (0x02<<8)|PSOC_PSU_OFF(psu2_pin)); 
static SENSOR_DEVICE_ATTR(psoc_psu2_pout,     S_IRUGO,			        show_psu_psoc,  0,      (0x02<<8)|PSOC_PSU_OFF(psu2_pout));

static SENSOR_DEVICE_ATTR(psoc_psu1_vendor,   S_IRUGO,                          show_psu_psoc,  0,   (0x10<<8)|PSOC_PSU_OFF(psu1_vendor));
static SENSOR_DEVICE_ATTR(psoc_psu1_model,    S_IRUGO,                          show_psu_psoc,  0,   (0x14<<8)|PSOC_PSU_OFF(psu1_model));
static SENSOR_DEVICE_ATTR(psoc_psu1_version,  S_IRUGO,                          show_psu_psoc,  0,   (0x08<<8)|PSOC_PSU_OFF(psu1_version));
static SENSOR_DEVICE_ATTR(psoc_psu1_date,     S_IRUGO,                          show_psu_psoc,  0,   (0x06<<8)|PSOC_PSU_OFF(psu1_date));
static SENSOR_DEVICE_ATTR(psoc_psu1_sn,       S_IRUGO,                          show_psu_psoc,  0,   (0x14<<8)|PSOC_PSU_OFF(psu1_sn));
static SENSOR_DEVICE_ATTR(psoc_psu2_vendor,   S_IRUGO,                          show_psu_psoc,  0,   (0x10<<8)|PSOC_PSU_OFF(psu2_vendor));
static SENSOR_DEVICE_ATTR(psoc_psu2_model,    S_IRUGO,                          show_psu_psoc,  0,   (0x14<<8)|PSOC_PSU_OFF(psu2_model));
static SENSOR_DEVICE_ATTR(psoc_psu2_version,  S_IRUGO,                          show_psu_psoc,  0,   (0x08<<8)|PSOC_PSU_OFF(psu2_version));
static SENSOR_DEVICE_ATTR(psoc_psu2_date,     S_IRUGO,                          show_psu_psoc,  0,   (0x06<<8)|PSOC_PSU_OFF(psu2_date));
static SENSOR_DEVICE_ATTR(psoc_psu2_sn,       S_IRUGO,                          show_psu_psoc,  0,   (0x14<<8)|PSOC_PSU_OFF(psu2_sn));
			
static struct attribute *psoc_attributes[] = {
    //thermal
	&sensor_dev_attr_temp1_input.dev_attr.attr,
	&sensor_dev_attr_temp2_input.dev_attr.attr,
	&sensor_dev_attr_temp3_input.dev_attr.attr,
	&sensor_dev_attr_temp4_input.dev_attr.attr,
	&sensor_dev_attr_temp5_input.dev_attr.attr,
	
	&sensor_dev_attr_thermal_psu1.dev_attr.attr,
	&sensor_dev_attr_thermal_psu2.dev_attr.attr,


    //pwm
	&sensor_dev_attr_pwm1.dev_attr.attr,
	&sensor_dev_attr_pwm2.dev_attr.attr,
	&sensor_dev_attr_pwm3.dev_attr.attr,
	&sensor_dev_attr_pwm4.dev_attr.attr,
	&sensor_dev_attr_pwm_psu1.dev_attr.attr,
	&sensor_dev_attr_pwm_psu2.dev_attr.attr,
	
	//rpm
	&sensor_dev_attr_fan1_input.dev_attr.attr,
	&sensor_dev_attr_fan2_input.dev_attr.attr,
	&sensor_dev_attr_fan3_input.dev_attr.attr,
	&sensor_dev_attr_fan4_input.dev_attr.attr,
	&sensor_dev_attr_fan5_input.dev_attr.attr,
	&sensor_dev_attr_fan6_input.dev_attr.attr,
	&sensor_dev_attr_fan7_input.dev_attr.attr,
	&sensor_dev_attr_fan8_input.dev_attr.attr,
	
	&sensor_dev_attr_rpm_psu1.dev_attr.attr,
	&sensor_dev_attr_rpm_psu2.dev_attr.attr,
	
	//clei
        &sensor_dev_attr_fan1_issue_number.dev_attr.attr,
        &sensor_dev_attr_fan2_issue_number.dev_attr.attr,
        &sensor_dev_attr_fan3_issue_number.dev_attr.attr,
        &sensor_dev_attr_fan4_issue_number.dev_attr.attr,
        &sensor_dev_attr_fan1_abbreviation_number.dev_attr.attr,
        &sensor_dev_attr_fan2_abbreviation_number.dev_attr.attr,
        &sensor_dev_attr_fan3_abbreviation_number.dev_attr.attr,
        &sensor_dev_attr_fan4_abbreviation_number.dev_attr.attr,
        &sensor_dev_attr_fan1_fc_number.dev_attr.attr,
        &sensor_dev_attr_fan2_fc_number.dev_attr.attr,
        &sensor_dev_attr_fan3_fc_number.dev_attr.attr,
        &sensor_dev_attr_fan4_fc_number.dev_attr.attr,
        &sensor_dev_attr_fan1_clei_code.dev_attr.attr,
        &sensor_dev_attr_fan2_clei_code.dev_attr.attr,
        &sensor_dev_attr_fan3_clei_code.dev_attr.attr,
        &sensor_dev_attr_fan4_clei_code.dev_attr.attr,

        &sensor_dev_attr_fan1_product_year_and_month.dev_attr.attr,
        &sensor_dev_attr_fan2_product_year_and_month.dev_attr.attr,
        &sensor_dev_attr_fan3_product_year_and_month.dev_attr.attr,
        &sensor_dev_attr_fan4_product_year_and_month.dev_attr.attr,
        &sensor_dev_attr_fan1_label_location_code.dev_attr.attr,
        &sensor_dev_attr_fan2_label_location_code.dev_attr.attr,
        &sensor_dev_attr_fan3_label_location_code.dev_attr.attr,
        &sensor_dev_attr_fan4_label_location_code.dev_attr.attr,
        &sensor_dev_attr_fan1_serial_number.dev_attr.attr,
        &sensor_dev_attr_fan2_serial_number.dev_attr.attr,
        &sensor_dev_attr_fan3_serial_number.dev_attr.attr,
        &sensor_dev_attr_fan4_serial_number.dev_attr.attr,
        &sensor_dev_attr_fan1_pcb_revision.dev_attr.attr,
        &sensor_dev_attr_fan2_pcb_revision.dev_attr.attr,
        &sensor_dev_attr_fan3_pcb_revision.dev_attr.attr,
        &sensor_dev_attr_fan4_pcb_revision.dev_attr.attr,

    
    //switch temperature
	&sensor_dev_attr_switch_tmp.dev_attr.attr,

    //diag flag
	&sensor_dev_attr_diag.dev_attr.attr,
	
	//version
	&sensor_dev_attr_version.dev_attr.attr,
	
	//fan led 
	&sensor_dev_attr_fan1_led_grn.dev_attr.attr,
	&sensor_dev_attr_fan2_led_grn.dev_attr.attr,
	&sensor_dev_attr_fan3_led_grn.dev_attr.attr,
	&sensor_dev_attr_fan4_led_grn.dev_attr.attr,
	&sensor_dev_attr_fan1_led_red.dev_attr.attr,
	&sensor_dev_attr_fan2_led_red.dev_attr.attr,
	&sensor_dev_attr_fan3_led_red.dev_attr.attr,
	&sensor_dev_attr_fan4_led_red.dev_attr.attr,

	//fan GPI 
	&sensor_dev_attr_fan_gpi.dev_attr.attr,
	
	//psu
#if 0
	&sensor_dev_attr_psu1_vin.dev_attr.attr,
	&sensor_dev_attr_psu1_vout.dev_attr.attr,
	&sensor_dev_attr_psu1_iin.dev_attr.attr,
	&sensor_dev_attr_psu1_iout.dev_attr.attr,
	&sensor_dev_attr_psu1_pin.dev_attr.attr,
	&sensor_dev_attr_psu1_pout.dev_attr.attr,
	&sensor_dev_attr_psu2_vin.dev_attr.attr,
	&sensor_dev_attr_psu2_vout.dev_attr.attr,
	&sensor_dev_attr_psu2_iin.dev_attr.attr,
	&sensor_dev_attr_psu2_iout.dev_attr.attr,
	&sensor_dev_attr_psu2_pin.dev_attr.attr,
	&sensor_dev_attr_psu2_pout.dev_attr.attr,
#endif
	&sensor_dev_attr_psu0.dev_attr.attr,
	&sensor_dev_attr_psu1.dev_attr.attr,


	//psu_psoc, new added on psoc 1.9
	&sensor_dev_attr_psoc_psu1_vin.dev_attr.attr,
	&sensor_dev_attr_psoc_psu1_vout.dev_attr.attr,
	&sensor_dev_attr_psoc_psu1_iin.dev_attr.attr,
	&sensor_dev_attr_psoc_psu1_iout.dev_attr.attr,
	&sensor_dev_attr_psoc_psu1_pin.dev_attr.attr,
	&sensor_dev_attr_psoc_psu1_pout.dev_attr.attr,
	&sensor_dev_attr_psoc_psu2_vin.dev_attr.attr,
	&sensor_dev_attr_psoc_psu2_vout.dev_attr.attr,
	&sensor_dev_attr_psoc_psu2_iin.dev_attr.attr,
	&sensor_dev_attr_psoc_psu2_iout.dev_attr.attr,
	&sensor_dev_attr_psoc_psu2_pin.dev_attr.attr,
	&sensor_dev_attr_psoc_psu2_pout.dev_attr.attr,

        //add info
        &sensor_dev_attr_psoc_psu1_vendor.dev_attr.attr,
        &sensor_dev_attr_psoc_psu1_model.dev_attr.attr,
        &sensor_dev_attr_psoc_psu1_version.dev_attr.attr,
        &sensor_dev_attr_psoc_psu1_date.dev_attr.attr,
        &sensor_dev_attr_psoc_psu1_sn.dev_attr.attr,
        &sensor_dev_attr_psoc_psu2_vendor.dev_attr.attr,
        &sensor_dev_attr_psoc_psu2_model.dev_attr.attr,
        &sensor_dev_attr_psoc_psu2_version.dev_attr.attr,
        &sensor_dev_attr_psoc_psu2_date.dev_attr.attr,
        &sensor_dev_attr_psoc_psu2_sn.dev_attr.attr,
	NULL
};

static const struct attribute_group psoc_group = {
	.attrs = psoc_attributes,
};

/*-----------------------------------------------------------------------*/

/* device probe and removal */

static int
psoc_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct psoc_data *data;
	int status;

//    printk("+%s\n", __func__);
    
	if (!i2c_check_functionality(client->adapter,
			I2C_FUNC_SMBUS_BYTE_DATA | I2C_FUNC_SMBUS_WORD_DATA))
		return -EIO;

	data = kzalloc(sizeof(struct psoc_data), GFP_KERNEL);
	if (!data)
		return -ENOMEM;

	i2c_set_clientdata(client, data);
	mutex_init(&data->update_lock);
	data->diag    = 0;
	
	/* Register sysfs hooks */
	status = sysfs_create_group(&client->dev.kobj, &psoc_group);
	if (status)
		goto exit_free;

	data->hwmon_dev = hwmon_device_register(&client->dev);
	if (IS_ERR(data->hwmon_dev)) {
		status = PTR_ERR(data->hwmon_dev);
		goto exit_remove;
	}
    data->auto_update = kthread_run(psoc_polling_thread,client,"%s",dev_name(data->hwmon_dev));
    if (IS_ERR(data->auto_update))
    {
    	status = PTR_ERR(data->auto_update);
    	goto exit_remove;
    }

	dev_info(&client->dev, "%s: sensor '%s'\n",
		 dev_name(data->hwmon_dev), client->name);

	psoc_led_client_dev = &client->dev;
	return 0;

exit_remove:
	sysfs_remove_group(&client->dev.kobj, &psoc_group);
exit_free:
	i2c_set_clientdata(client, NULL);
	kfree(data);
	psoc_led_client_dev = NULL;
	return status;
}

static int psoc_remove(struct i2c_client *client)
{
	struct psoc_data *data = i2c_get_clientdata(client);
    kthread_stop(data->auto_update);
	hwmon_device_unregister(data->hwmon_dev);
	sysfs_remove_group(&client->dev.kobj, &psoc_group);
	i2c_set_clientdata(client, NULL);
	kfree(data);
	psoc_led_client_dev = NULL;
	return 0;
}

static const struct i2c_device_id psoc_ids[] = {
	{ "inv_psoc", 0, },
	{ /* LIST END */ }
};
MODULE_DEVICE_TABLE(i2c, psoc_ids);

static struct i2c_driver psoc_driver = {
	.class		= I2C_CLASS_HWMON,
	.driver = {
		.name	= "inv_psoc",
	},
	.probe		= psoc_probe,
	.remove		= psoc_remove,
	.id_table	= psoc_ids,
};

/*-----------------------------------------------------------------------*/

/* module glue */

static int __init inv_psoc_init(void)
{
	return i2c_add_driver(&psoc_driver);
}

static void __exit inv_psoc_exit(void)
{
	i2c_del_driver(&psoc_driver);
}

MODULE_AUTHOR("Jack Ting <ting.jack@inventec>");
MODULE_DESCRIPTION("inv psoc driver");
MODULE_LICENSE("GPL");

module_init(inv_psoc_init);
module_exit(inv_psoc_exit);
