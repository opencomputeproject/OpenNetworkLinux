/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014, 2015 Big Switch Networks, Inc.
 *           Copyright 2016 Accton Technology Corporation.
 *           Copyright 2017 Delta Networks, Inc
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
#include <unistd.h>
#include <fcntl.h>
#include <AIM/aim.h>
#include "platform_lib.h"
#include "arm_delta_i2c.h"


psu_type_t get_psu_type(int id)
{
	if ((id == PSU1_ID)||(id == PSU2_ID))
		return PSU_TYPE_AC_B2F;
    return PSU_TYPE_UNKNOWN;
}

enum ag6248c_product_id get_product_id(void)
{
    int ret;
    int pid = PID_UNKNOWN;
	
	ret = i2c_devname_read_byte("CPLD", 0X01);
	
	if(ret<0)
		return PID_UNKNOWN;
	
	pid = ((ret&0xf0)>>4);
    
	
    if (pid >= PID_UNKNOWN || pid < PID_AG6248C_48) {
        return PID_UNKNOWN;
    }

    return pid;
}

int chassis_fan_count(void)
{
    enum ag6248c_product_id pid = get_product_id();

	if ((pid == PID_AG6248C_48P)||(pid == PID_AG6248C_48)) {
		return 4;
	}

    return 0 ;
}

int chassis_led_count(void)
{
    enum ag6248c_product_id pid = get_product_id();

	if (pid == PID_AG6248C_48P)
		return 5;
	else if(pid == PID_AG6248C_48)
		return 6;
	else 
		return 0;
}
