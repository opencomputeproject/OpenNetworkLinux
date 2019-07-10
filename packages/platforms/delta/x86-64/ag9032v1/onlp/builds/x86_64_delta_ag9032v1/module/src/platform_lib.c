/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright (C) 2017 Delta Networks, Inc.
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
#include <sys/mman.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <AIM/aim.h>
#include "platform_lib.h"
#include <onlplib/i2c.h>

static   onlp_shlock_t* dni_lock = NULL;

#define DNI_BUS_LOCK()                           \
    do{    \
        onlp_shlock_take(dni_lock);    \
    }while(0)

#define DNI_BUS_UNLOCK()                           \
    do{  \
        onlp_shlock_give(dni_lock);\
    }while(0)

#define DNILOCK_MAGIC 0xE3A0B4E6 

void lockinit()
{
    static int sem_inited =0;
    if(!sem_inited)
    {
        onlp_shlock_create(DNILOCK_MAGIC, &dni_lock, "bus-lock");
        sem_inited =1;
    }
}

int dni_fan_speed_good()
{
    int rpm = 0, rpm1 = 0, speed_good = 0;

    rpm = dni_i2c_lock_read_attribute(NULL, FAN1_FRONT);
    rpm1 = dni_i2c_lock_read_attribute(NULL, FAN1_REAR);
    if(rpm != 0 && rpm != 960 && rpm1 != 0 && rpm1 != 960)
        speed_good++;
    rpm = dni_i2c_lock_read_attribute(NULL, FAN2_FRONT);
    rpm1 = dni_i2c_lock_read_attribute(NULL, FAN2_REAR);
    if(rpm != 0 && rpm != 960 && rpm1 != 0 && rpm1 != 960)
        speed_good++;
    rpm = dni_i2c_lock_read_attribute(NULL, FAN3_FRONT);
    rpm1 = dni_i2c_lock_read_attribute(NULL, FAN3_REAR);
    if(rpm != 0 && rpm != 960 && rpm1 != 0 && rpm1 != 960)
        speed_good++;
    rpm = dni_i2c_lock_read_attribute(NULL, FAN4_FRONT);
    rpm1 = dni_i2c_lock_read_attribute(NULL, FAN4_REAR);
    if(rpm != 0 && rpm != 960 && rpm1 != 0 && rpm1 != 960)
        speed_good++;
    rpm = dni_i2c_lock_read_attribute(NULL, FAN5_FRONT);
    rpm1 = dni_i2c_lock_read_attribute(NULL, FAN5_REAR);
    if(rpm != 0 && rpm != 960 && rpm1 != 0 && rpm1 != 960)
        speed_good++;
    return speed_good;
}

int dni_i2c_read_attribute_binary(char *filename, char *buffer, int buf_size, int data_len)
{
    int fd,rv=0;
    int len;
    DNI_BUS_LOCK();

    if ((buffer == NULL) || (buf_size < 0)) {
        rv=-1;
        goto ERROR;
    }

    if ((fd = open(filename, O_RDONLY)) == -1) {
        rv=-1;
        goto ERROR;
    }

    if ((len = read(fd, buffer, buf_size)) < 0) {
        close(fd);
        rv= -1;
        goto ERROR;
    }

    if ((close(fd) == -1)) {
        rv= -1;
        goto ERROR;
    }

    if ((len > buf_size) || (data_len != 0 && len != data_len)) {
        rv= -1;
        goto ERROR;
    }

ERROR:
    DNI_BUS_UNLOCK();
    return rv;
}

int dni_i2c_read_attribute_string(char *filename, char *buffer, int buf_size, int data_len)
{
    int ret;

    if (data_len >= buf_size) {
	    return -1;
    }

    ret = dni_i2c_read_attribute_binary(filename, buffer, buf_size-1, data_len);

    if (ret == 0) {
        buffer[buf_size-1] = '\0';
    }

    return ret;
}

/* Lock function */
int dni_i2c_lock_read( mux_info_t * mux_info, dev_info_t * dev_info)
{
    int r_data=0;
    DNI_BUS_LOCK();
    if(mux_info != NULL)
        dni_swpld_write_attribute(mux_info->offset, mux_info->channel,BUS_LOCKED);   
    
    if(dev_info->size == 1)
        r_data = onlp_i2c_readb(dev_info->bus, dev_info->addr, dev_info->offset, dev_info->flags);
    else
        r_data = onlp_i2c_readw(dev_info->bus, dev_info->addr, dev_info->offset, dev_info->flags);
 
    DNI_BUS_UNLOCK();
    return r_data;
}

int dni_i2c_lock_write( mux_info_t * mux_info, dev_info_t * dev_info)
{
    DNI_BUS_LOCK();
    if(mux_info != NULL)
        dni_swpld_write_attribute(mux_info->offset, mux_info->channel,BUS_LOCKED);   
    /* Write size */
    if(dev_info->size == 1)
        onlp_i2c_write(dev_info->bus, dev_info->addr, dev_info->offset, 1, &dev_info->data_8, dev_info->flags);
    else
        onlp_i2c_writew(dev_info->bus, dev_info->addr, dev_info->offset, dev_info->data_16, dev_info->flags);

    DNI_BUS_UNLOCK();
    return 0;
}

int dni_i2c_lock_read_attribute(mux_info_t * mux_info, char * fullpath)
{
    int fd, len, nbytes = 10,rv = -1;
    char  r_data[10]   = {0};

    DNI_BUS_LOCK();
    if(mux_info != NULL)
        dni_swpld_write_attribute(mux_info->offset, mux_info->channel,BUS_LOCKED);   
    if ((fd = open(fullpath, O_RDONLY)) >= 0)
    {
        if ((len = read(fd, r_data, nbytes)) > 0)
        {
            rv=atoi(r_data);
        }
    }
    close(fd);
    DNI_BUS_UNLOCK();
    return rv;
}

int dni_i2c_lock_write_attribute(mux_info_t * mux_info, char * data,char * fullpath)
{
    int fd, nbytes = 10, rv = -1;
    DNI_BUS_LOCK();
    if(mux_info!=NULL)
        dni_swpld_write_attribute(mux_info->offset, mux_info->channel,BUS_LOCKED);
    /* Create output file descriptor */
    if((fd = open(fullpath, O_WRONLY,  0644)) >= 0)
    {
        if(write(fd, data, (ssize_t) nbytes) > 0)
        {
            fsync(fd);
            rv = 0;
        }
    }
    close(fd);
    DNI_BUS_UNLOCK();
    return rv;
}


/*  SWPLD modulize in AG9032v1 platform at bus 6 on address 0x31.
    Use this function to select address & read the data.          */
int dni_lock_swpld_read_attribute(int addr)
{
    int fd , fd1, nbytes = 10,data = 0, rv=-1;
    char r_data[10]   = {0};
    char address[10] = {0};
    sprintf(address, "%02x", addr);
    DNI_BUS_LOCK();
    /* Create output file descriptor */
    if((fd = open(SWPLD_ADDR_PATH, O_WRONLY,  0644)) >=0)
    {
        if(write (fd, address, 2) >0)
        {
            fsync(fd);
            if ((fd1 = open(SWPLD_DATA_PATH, O_RDONLY,0644)) >= 0)
            {
                if ((read(fd1, r_data, nbytes)) > 0)
                {
                    sscanf( r_data, "%x", &data);
                    rv=data;
                }
            }
            close(fd1);
        }
    }
    close(fd);
    DNI_BUS_UNLOCK();
    return rv;
}

/*  SWPLD modulize in AG9032v1 platform at bus 6 on address 0x31.
    Use this function to select address the & write the data.     */
int dni_swpld_write_attribute(int addr, int data,int bus_lock)
{
    int fd,fd1,rv = -1;
    char address[10] = {0};
    sprintf(address, "%02x", addr);
    if(bus_lock == BUS_LOCK)
        DNI_BUS_LOCK();
    /* Create output file descriptor */
    if((fd= open(SWPLD_ADDR_PATH, O_WRONLY,  0644)) >= 0)
    {
        if( write(fd, address, 2) > 0)
        {
            fsync(fd);
            if((fd1 = open(SWPLD_DATA_PATH, O_WRONLY,  0644)) >= 0)
            {
                sprintf(address, "%02x", data);
                if( write (fd1, address, 2) >0 )
                {
                    rv=0;
                    fsync(fd1);
                }
            }
            close(fd1);
        }
    }
    close(fd);
    if(bus_lock == BUS_LOCK)
        DNI_BUS_UNLOCK();
    return rv;
}

