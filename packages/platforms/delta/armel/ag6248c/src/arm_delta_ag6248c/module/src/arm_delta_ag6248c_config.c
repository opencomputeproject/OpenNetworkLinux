/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 * Copyright 2018, Delta Networks, Inc.
 *
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

/* <auto.start.cdefs(ARM_DELTA_AG6248C_CONFIG_HEADER).source> */
#define __arm_delta_ag6248c_config_STRINGIFY_NAME(_x) #_x
#define __arm_delta_ag6248c_config_STRINGIFY_VALUE(_x) __arm_delta_ag6248c_config_STRINGIFY_NAME(_x)
arm_delta_ag6248c_config_settings_t arm_delta_ag6248c_config_settings[] =
{
#ifdef ARM_DELTA_AG6248C_CONFIG_INCLUDE_LOGGING
    { __arm_delta_ag6248c_config_STRINGIFY_NAME(ARM_DELTA_AG6248C_CONFIG_INCLUDE_LOGGING), __arm_delta_ag6248c_config_STRINGIFY_VALUE(ARM_DELTA_AG6248C_CONFIG_INCLUDE_LOGGING) },
#else
{ ARM_DELTA_AG6248C_CONFIG_INCLUDE_LOGGING(__arm_delta_ag6248c_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef ARM_DELTA_AG6248C_CONFIG_LOG_OPTIONS_DEFAULT
    { __arm_delta_ag6248c_config_STRINGIFY_NAME(ARM_DELTA_AG6248C_CONFIG_LOG_OPTIONS_DEFAULT), __arm_delta_ag6248c_config_STRINGIFY_VALUE(ARM_DELTA_AG6248C_CONFIG_LOG_OPTIONS_DEFAULT) },
#else
{ ARM_DELTA_AG6248C_CONFIG_LOG_OPTIONS_DEFAULT(__arm_delta_ag6248c_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef ARM_DELTA_AG6248C_CONFIG_LOG_BITS_DEFAULT
    { __arm_delta_ag6248c_config_STRINGIFY_NAME(ARM_DELTA_AG6248C_CONFIG_LOG_BITS_DEFAULT), __arm_delta_ag6248c_config_STRINGIFY_VALUE(ARM_DELTA_AG6248C_CONFIG_LOG_BITS_DEFAULT) },
#else
{ ARM_DELTA_AG6248C_CONFIG_LOG_BITS_DEFAULT(__arm_delta_ag6248c_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef ARM_DELTA_AG6248C_CONFIG_LOG_CUSTOM_BITS_DEFAULT
    { __arm_delta_ag6248c_config_STRINGIFY_NAME(ARM_DELTA_AG6248C_CONFIG_LOG_CUSTOM_BITS_DEFAULT), __arm_delta_ag6248c_config_STRINGIFY_VALUE(ARM_DELTA_AG6248C_CONFIG_LOG_CUSTOM_BITS_DEFAULT) },
#else
{ ARM_DELTA_AG6248C_CONFIG_LOG_CUSTOM_BITS_DEFAULT(__arm_delta_ag6248c_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef ARM_DELTA_AG6248C_CONFIG_PORTING_STDLIB
    { __arm_delta_ag6248c_config_STRINGIFY_NAME(ARM_DELTA_AG6248C_CONFIG_PORTING_STDLIB), __arm_delta_ag6248c_config_STRINGIFY_VALUE(ARM_DELTA_AG6248C_CONFIG_PORTING_STDLIB) },
#else
{ ARM_DELTA_AG6248C_CONFIG_PORTING_STDLIB(__arm_delta_ag6248c_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef ARM_DELTA_AG6248C_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS
    { __arm_delta_ag6248c_config_STRINGIFY_NAME(ARM_DELTA_AG6248C_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS), __arm_delta_ag6248c_config_STRINGIFY_VALUE(ARM_DELTA_AG6248C_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS) },
#else
{ ARM_DELTA_AG6248C_CONFIG_PORTING_INCLUDE_STDLIB_HEADERS(__arm_delta_ag6248c_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef ARM_DELTA_AG6248C_CONFIG_INCLUDE_UCLI
    { __arm_delta_ag6248c_config_STRINGIFY_NAME(ARM_DELTA_AG6248C_CONFIG_INCLUDE_UCLI), __arm_delta_ag6248c_config_STRINGIFY_VALUE(ARM_DELTA_AG6248C_CONFIG_INCLUDE_UCLI) },
#else
{ ARM_DELTA_AG6248C_CONFIG_INCLUDE_UCLI(__arm_delta_ag6248c_config_STRINGIFY_NAME), "__undefined__" },
#endif
#ifdef ARM_DELTA_AG6248C_CONFIG_SFP_COUNT
    { __arm_delta_ag6248c_config_STRINGIFY_NAME(ARM_DELTA_AG6248C_CONFIG_SFP_COUNT), __arm_delta_ag6248c_config_STRINGIFY_VALUE(ARM_DELTA_AG6248C_CONFIG_SFP_COUNT) },
#else
{ ARM_DELTA_AG6248C_CONFIG_SFP_COUNT(__arm_delta_ag6248c_config_STRINGIFY_NAME), "__undefined__" },
#endif
    { NULL, NULL }
};
#undef __arm_delta_ag6248c_config_STRINGIFY_VALUE
#undef __arm_delta_ag6248c_config_STRINGIFY_NAME

const char*
arm_delta_ag6248c_config_lookup(const char* setting)
{
    int i;
    for(i = 0; arm_delta_ag6248c_config_settings[i].name; i++) {
        if(strcmp(arm_delta_ag6248c_config_settings[i].name, setting)) {
            return arm_delta_ag6248c_config_settings[i].value;
        }
    }
    return NULL;
}

int
arm_delta_ag6248c_config_show(struct aim_pvs_s* pvs)
{
    int i;
    for(i = 0; arm_delta_ag6248c_config_settings[i].name; i++) {
        aim_printf(pvs, "%s = %s\n", arm_delta_ag6248c_config_settings[i].name, arm_delta_ag6248c_config_settings[i].value);
    }
    return i;
}

/* <auto.end.cdefs(ARM_DELTA_AG6248C_CONFIG_HEADER).source> */

