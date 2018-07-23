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

#include <onlp/onlp_config.h>

#include "onlp_log.h"
#include "onlp_int.h"

/*
 * onlp log struct.
 */
AIM_LOG_STRUCT_DEFINE(
                      ONLP_CONFIG_LOG_OPTIONS_DEFAULT,
                      ONLP_CONFIG_LOG_BITS_DEFAULT,
                      onlp_log_flag_map,
                      ONLP_CONFIG_LOG_CUSTOM_BITS_DEFAULT
                     );

static int
log_enabled__(int rv, uint32_t flags)
{
    return ONLP_FAILURE(rv);
}

int
onlp_vlog_error(uint32_t flags, int rv, const char* fmt, va_list vargs)
{
    if(log_enabled__(rv, flags)) {
        char* s1 = aim_vdfstrdup(fmt, vargs);
        char* s2;
        if(ONLP_FAILURE(rv)) {
            s2 = aim_dfstrdup(" failed: %{onlp_status}", rv);
            AIM_LOG_ERROR("%s%s", s1, s2);
        }
        else {
            s2 = aim_dfstrdup(" %{onlp_status}", rv);
            AIM_LOG_INFO("%s%s", s1, s2);
        }
        aim_free(s1);
        aim_free(s2);
    }
    return rv;
}

int
onlp_log_error(uint32_t flags, int rv, const char* fmt, ...)
{
    int rc;
    va_list vargs;
    va_start(vargs, fmt);
    rc = onlp_vlog_error(flags, rv, fmt, vargs);
    va_end(vargs);
    return rc;
}
