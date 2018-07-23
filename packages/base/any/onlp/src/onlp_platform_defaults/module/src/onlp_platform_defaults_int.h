/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *        Copyright 2014, 2015 Big Switch Networks, Inc.
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

/**************************************************************************//**
 *
 * onlp_platform_defaults Internal Header
 *
 *****************************************************************************/
#ifndef __ONLP_PLATFORM_DEFAULTS_INT_H__
#define __ONLP_PLATFORM_DEFAULTS_INT_H__

#include <onlp_platform_defaults/onlp_platform_defaults_config.h>

/* <auto.start.enum(ALL).header> */
/** onlp_platform_defaults_log_flag */
typedef enum onlp_platform_defaults_log_flag_e {
    ONLP_PLATFORM_DEFAULTS_LOG_FLAG_CALLED,
    ONLP_PLATFORM_DEFAULTS_LOG_FLAG_LAST = ONLP_PLATFORM_DEFAULTS_LOG_FLAG_CALLED,
    ONLP_PLATFORM_DEFAULTS_LOG_FLAG_COUNT,
    ONLP_PLATFORM_DEFAULTS_LOG_FLAG_INVALID = -1,
} onlp_platform_defaults_log_flag_t;

/** Strings macro. */
#define ONLP_PLATFORM_DEFAULTS_LOG_FLAG_STRINGS \
{\
    "called", \
}
/** Enum names. */
const char* onlp_platform_defaults_log_flag_name(onlp_platform_defaults_log_flag_t e);

/** Enum values. */
int onlp_platform_defaults_log_flag_value(const char* str, onlp_platform_defaults_log_flag_t* e, int substr);

/** Enum descriptions. */
const char* onlp_platform_defaults_log_flag_desc(onlp_platform_defaults_log_flag_t e);

/** validator */
#define ONLP_PLATFORM_DEFAULTS_LOG_FLAG_VALID(_e) \
    ( (0 <= (_e)) && ((_e) <= ONLP_PLATFORM_DEFAULTS_LOG_FLAG_CALLED))

/** onlp_platform_defaults_log_flag_map table. */
extern aim_map_si_t onlp_platform_defaults_log_flag_map[];
/** onlp_platform_defaults_log_flag_desc_map table. */
extern aim_map_si_t onlp_platform_defaults_log_flag_desc_map[];
/* <auto.end.enum(ALL).header> */


#define __ONLP_DEFAULTI __attribute__((weak))

#define ONLP_SYSI_PLATFORM_NAME_DEFAULT "onlp-sysi-platform-default"

#define __ONLP_DEFAULT_IMPLEMENTATION__(_f, _rc)                        \
    int __ONLP_DEFAULTI _f                                              \
    {                                                                   \
        ONLP_PLATFORM_DEFAULTS_LOG_CALLED("using default %s",           \
                                          __func__);                    \
        return _rc;                                                     \
    }

#define __ONLP_DEFAULTI_IMPLEMENTATION(_f)                              \
    __ONLP_DEFAULT_IMPLEMENTATION__(_f, ONLP_STATUS_E_UNSUPPORTED)

#define __ONLP_DEFAULTI_IMPLEMENTATION_OPTIONAL(_f)                     \
    __ONLP_DEFAULT_IMPLEMENTATION__(_f, ONLP_STATUS_OK)

#define __ONLP_DEFAULTI_VIMPLEMENTATION(_f)                             \
    void __ONLP_DEFAULTI _f                                             \
    {                                                                   \
        ONLP_PLATFORM_DEFAULTS_LOG_CALLED("using default %s",           \
                                          __func__);                    \
    }


#endif /* __ONLP_PLATFORM_DEFAULTS_INT_H__ */
