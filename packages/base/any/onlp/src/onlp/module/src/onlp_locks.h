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
#ifndef __ONLP_LOCKS_H__
#define __ONLP_LOCKS_H__

#include <onlp/onlp_config.h>

#if ONLP_CONFIG_INCLUDE_API_LOCK == 1

/**
 * @brief Initialize the ONLP API lock infrastructure.
 */
void onlp_api_lock_init();

/**
 * @brief Take the ONLP API lock.
 */
void onlp_api_lock(const char* api);

/**
 * @brief Give the ONLP API lock.
 */
void onlp_api_unlock(void);


#define ONLP_API_LOCK_INIT() onlp_api_lock_init()
#define ONLP_API_LOCK(_api)      onlp_api_lock(_api)
#define ONLP_API_UNLOCK()    onlp_api_unlock()

#else

#define ONLP_API_LOCK_INIT()
#define ONLP_API_LOCK(_api)
#define ONLP_API_UNLOCK()

#endif /** ONLP_CONFIG_INCLUDE_API_LOCK */


/****************************************************************************
 *
 * These macros are used the instantiate the public (and potentially locked)
 * ONLP API entry points.
 *
 ***************************************************************************/

#define ONLP_LOCKED_API_NAME(_name) _name##_locked__

#define ONLP_LOCKED_API0(_name)                     \
    int _name (void)                                \
    {                                               \
        ONLP_API_LOCK(#_name);                       \
        int _rv = ONLP_LOCKED_API_NAME(_name)();    \
        ONLP_API_UNLOCK();                          \
        return _rv;                                 \
    }

#define ONLP_LOCKED_API1(_name, _t, _v)                 \
    int _name (_t _v)                                   \
    {                                                   \
        ONLP_API_LOCK(#_name);                           \
        int _rv = ONLP_LOCKED_API_NAME(_name)(_v);      \
        ONLP_API_UNLOCK();                              \
        return _rv;                                     \
    }

#define ONLP_LOCKED_API2(_name, _t1, _v1, _t2, _v2)              \
    int _name (_t1 _v1, _t2 _v2)                                 \
    {                                                            \
        ONLP_API_LOCK(#_name);                                    \
        int _rv = ONLP_LOCKED_API_NAME(_name) (_v1, _v2);        \
        ONLP_API_UNLOCK();                                       \
        return _rv;                                              \
    }

#define ONLP_LOCKED_API3(_name, _t1, _v1, _t2, _v2, _t3, _v3)           \
    int _name (_t1 _v1, _t2 _v2, _t3 _v3)                               \
    {                                                                   \
        ONLP_API_LOCK(#_name);                                           \
        int _rv = ONLP_LOCKED_API_NAME(_name) (_v1, _v2, _v3);          \
        ONLP_API_UNLOCK();                                              \
        return _rv;                                                     \
    }

#define ONLP_LOCKED_API4(_name, _t1, _v1, _t2, _v2, _t3, _v3, _t4, _v4) \
    int _name (_t1 _v1, _t2 _v2, _t3 _v3, _t4 _v4)                      \
    {                                                                   \
        ONLP_API_LOCK(#_name);                                           \
        int _rv = ONLP_LOCKED_API_NAME(_name) (_v1, _v2, _v3, _v4);     \
        ONLP_API_UNLOCK();                                              \
        return _rv;                                                     \
    }

#define ONLP_LOCKED_API5(_name, _t1, _v1, _t2, _v2, _t3, _v3, _t4, _v4, _t5, _v5) \
    int _name (_t1 _v1, _t2 _v2, _t3 _v3, _t4 _v4, _t5 _v5)             \
    {                                                                   \
        ONLP_API_LOCK(#_name);                                           \
        int _rv = ONLP_LOCKED_API_NAME(_name) (_v1, _v2, _v3, _v4, _v5); \
        ONLP_API_UNLOCK();                                              \
        return _rv;                                                     \
    }

#define ONLP_LOCKED_VAPI0(_name)                         \
    void _name (void)                                    \
    {                                                    \
        ONLP_API_LOCK(#_name);                            \
        ONLP_LOCKED_API_NAME(_name)();                   \
        ONLP_API_UNLOCK();                               \
    }

#define ONLP_LOCKED_VAPI1(_name, _t, _v)                 \
    void _name (_t _v)                                   \
    {                                                    \
        ONLP_API_LOCK(#_name);                            \
        ONLP_LOCKED_API_NAME(_name)(_v);                 \
        ONLP_API_UNLOCK();                               \
    }

#define ONLP_LOCKED_VAPI2(_name, _t1, _v1, _t2, _v2)             \
    void _name (_t1 _v1, _t2 _v2)                                \
    {                                                            \
        ONLP_API_LOCK(#_name);                                    \
        ONLP_LOCKED_API_NAME(_name) (_v1, _v2);                  \
        ONLP_API_UNLOCK();                                       \
    }

#define ONLP_LOCKED_VAPI3(_name, _t1, _v1, _t2, _v2, _t3, _v3)          \
    void _name (_t1 _v1, _t2 _v2, _t3 _v3)                              \
    {                                                                   \
        ONLP_API_LOCK(#_name);                                           \
        ONLP_LOCKED_API_NAME(_name) (_v1, _v2, _v3);                    \
        ONLP_API_UNLOCK();                                              \
    }

#define ONLP_LOCKED_VAPI4(_name, _t1, _v1, _t2, _v2, _t3, _v3, _t4, _v4) \
    void _name (_t1 _v1, _t2 _v2, _t3 _v3, _t4 _v4)                     \
    {                                                                   \
        ONLP_API_LOCK(#_name);                                           \
        ONLP_LOCKED_API_NAME(_name) (_v1, _v2, _v3, _v4);               \
        ONLP_API_UNLOCK();                                              \
    }

#define ONLP_LOCKED_VAPI5(_name, _t1, _v1, _t2, _v2, _t3, _v3, _t4, _v4, _t5, _v5) \
    void _name (_t1 _v1, _t2 _v2, _t3 _v3, _t4 _v4, _t5 _v5)            \
    {                                                                   \
        ONLP_API_LOCK(#_name);                                           \
        ONLP_LOCKED_API_NAME(_name) (_v1, _v2, _v3, _v4, _v5);          \
        ONLP_API_UNLOCK();                                              \
    }




#endif /* __ONLP_LOCKS_H__ */
