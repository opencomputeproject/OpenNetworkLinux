/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
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
#include <x86_64_quanta_ly4r/x86_64_quanta_ly4r_config.h>
#include <x86_64_quanta_ly4r/x86_64_quanta_ly4r_gpio_table.h>
#include <onlp/platformi/fani.h>

#include "x86_64_quanta_ly4r_int.h"
#include "x86_64_quanta_ly4r_log.h"

#include <onlplib/file.h>
#include <onlplib/gpio.h>

int
onlp_fani_init(void)
{
    AIM_LOG_MSG("ONLP is not supported for FAN");
    return ONLP_STATUS_E_UNSUPPORTED;
}
