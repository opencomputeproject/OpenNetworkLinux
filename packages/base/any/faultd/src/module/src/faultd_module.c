/**************************************************************************//**
 * 
 * <bsn.cl fy=2013 v=onl>
 * 
 *        Copyright 2013, 2014 BigSwitch Networks, Inc.        
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
 * 
 * 
 *****************************************************************************/
#include <faultd/faultd_config.h>

#include "faultd_log.h"

static int
datatypes_init__(void)
{
#define FAULTD_ENUMERATION_ENTRY(_enum_name, _desc)     AIM_DATATYPE_MAP_REGISTER(_enum_name, _enum_name##_map, _desc,                               AIM_LOG_INTERNAL); 
#include <faultd/faultd.x>
    return 0; 
}

void __faultd_module_init__(void)
{
    AIM_LOG_STRUCT_REGISTER(); 
    datatypes_init__(); 
}

