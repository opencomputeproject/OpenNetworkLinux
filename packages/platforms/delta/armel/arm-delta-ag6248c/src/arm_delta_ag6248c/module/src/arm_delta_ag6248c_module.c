/************************************************************
 * <bsn.cl fy=2014 v=onl>
 * 
 * Copyright 2018, Delta Networks, Inc.       
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

#include <arm_delta_ag6248c/arm_delta_ag6248c_config.h>

#include "arm_delta_ag6248c_log.h"

static int
datatypes_init__(void)
{
#define ARM_DELTA_AG6248C_ENUMERATION_ENTRY(_enum_name, _desc)     AIM_DATATYPE_MAP_REGISTER(_enum_name, _enum_name##_map, _desc,                               AIM_LOG_INTERNAL);
#include <arm_delta_ag6248c/arm_delta_ag6248c.x>
    return 0;
}

void __arm_delta_ag6248c_module_init__(void)
{
    AIM_LOG_STRUCT_REGISTER();
    datatypes_init__();
}

int __onlp_platform_version__ = 1;
