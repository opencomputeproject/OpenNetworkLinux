/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *        Copyright 2014, 2015 Big Switch Networks, Inc.
 *        Copyright 2017 Delta Networks, Inc.
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
#include <onlp/platformi/psui.h>
#include "x86_64_dni_wb2448_int.h"

int
onlp_psui_init(void)
{
    return ONLP_STATUS_OK;
}

int 
onlp_psui_info_get(onlp_oid_t id, onlp_psu_info_t* rv)
{
    return ONLP_STATUS_OK;
}

int 
onlp_psui_ioctl(onlp_oid_t pid, va_list vargs)
{
    return ONLP_STATUS_OK;
}
