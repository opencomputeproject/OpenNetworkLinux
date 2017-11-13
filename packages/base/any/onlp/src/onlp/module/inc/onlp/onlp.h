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
#ifndef __ONLP_ONLP_H__
#define __ONLP_ONLP_H__

#include <onlp/onlp_config.h>

/* <auto.start.enum(tag:onlp).define> */
/** onlp_status */
typedef enum onlp_status_e {
    ONLP_STATUS_OK = 0,
    ONLP_STATUS_E_GENERIC = -1,
    ONLP_STATUS_E_UNSUPPORTED = -10,
    ONLP_STATUS_E_MISSING = -11,
    ONLP_STATUS_E_INVALID = -12,
    ONLP_STATUS_E_INTERNAL = -13,
    ONLP_STATUS_E_PARAM = -14,
    ONLP_STATUS_E_I2C = -15,
} onlp_status_t;
/* <auto.end.enum(tag:onlp).define> */

#define ONLP_IF_ERROR_RETURN(_expr)             \
    do {                                        \
        int _rv = (_expr);                      \
        if(_rv < 0) {                           \
            return _rv;                         \
        }                                       \
    } while(0)

#define ONLP_FAILURE(_rv) ((_rv) < 0)
#define ONLP_SUCCESS(_rv) (!(ONLP_FAILURE(_rv)))
#define ONLP_UNSUPPORTED(_rv) \
    ((_rv) == ONLP_STATUS_E_UNSUPPORTED)

/**
 * @brief Initialize all subsystems.
 */
int onlp_init(void);

int onlp_denit(void);

/**
 * @brief Dump the current platform data.
 * @param pvs The output pvs
 */

void onlp_platform_dump(aim_pvs_t* pvs, uint32_t flags);
void onlp_platform_show(aim_pvs_t* pvs, uint32_t flags);

/** Standardized macros for dealing with sensor milli-values */
#define ONLP_MILLI_NORMAL_INTEGER(_m) (_m / 1000)
#define ONLP_MILLI_NORMAL_TENTHS(_m) ( (_m % 1000) / 100)
#define ONLP_MILLI_NORMAL_INTEGER_TENTHS(_m) ONLP_MILLI_NORMAL_INTEGER(_m), ONLP_MILLI_NORMAL_TENTHS(_m)





/******************************************************************************
 *
 * Enumeration Support Definitions.
 *
 * Please do not add additional code beyond this point.
 *
 *****************************************************************************/
/* <auto.start.enum(tag:onlp).supportheader> */
/** Enum names. */
const char* onlp_status_name(onlp_status_t e);

/** Enum values. */
int onlp_status_value(const char* str, onlp_status_t* e, int substr);

/** Enum descriptions. */
const char* onlp_status_desc(onlp_status_t e);

/** Enum validator. */
int onlp_status_valid(onlp_status_t e);

/** validator */
#define ONLP_STATUS_VALID(_e) \
    (onlp_status_valid((_e)))

/** onlp_status_map table. */
extern aim_map_si_t onlp_status_map[];
/** onlp_status_desc_map table. */
extern aim_map_si_t onlp_status_desc_map[];
/* <auto.end.enum(tag:onlp).supportheader> */

#endif /* __ONLP_ONLP_H__ */
