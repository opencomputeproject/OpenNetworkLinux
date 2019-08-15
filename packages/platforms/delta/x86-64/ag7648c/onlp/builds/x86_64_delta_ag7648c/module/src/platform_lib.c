/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright 2015 Accton Technology Corporation.
 *           Copyright 2017 Delta Networks, Inc.
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
#include <unistd.h>
#include <fcntl.h>
#include <AIM/aim.h>
#include "platform_lib.h"

#define I2C_PSU_MODEL_NAME_LEN 13

psu_type_t get_psu_type(int id)
{
	if ((id == PSU1_ID)||(id == PSU2_ID))
		return PSU_TYPE_AC_F2B;
    return PSU_TYPE_UNKNOWN;
}

enum ag7648c_product_id get_product_id(void)
{
    return PID_AG7648C;
}

int chassis_fan_count(void)
{
    return 8 ;
}

int chassis_led_count(void)
{
    return 7;
}
