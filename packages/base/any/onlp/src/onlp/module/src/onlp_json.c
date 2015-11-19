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
#include "onlp_json.h"
#include "onlp_log.h"
#include <onlp/onlp.h>

static cJSON* root__ = NULL;
static char* file__ = NULL;

void
onlp_json_init(const char* fname)
{
    int rv;
    onlp_json_denit();

    rv = cjson_util_parse_file(fname, &root__);
    if(rv < 0 || root__ == NULL) {
        root__ = cJSON_Parse("{}");
    }
    else {
        file__ = aim_strdup(fname);
    }

}

cJSON*
onlp_json_get(int reload)
{
    if(reload) {
        onlp_json_init(file__);
    }
    return root__;
}

void
onlp_json_denit(void)
{
    if(root__) {
        cJSON_Delete(root__);
        root__ = NULL;
    }
    if(file__) {
        aim_free(file__);
        file__ = NULL;
    }
}
