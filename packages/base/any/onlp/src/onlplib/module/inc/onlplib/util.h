/**************************************************************
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
 **************************************************************
 *
 * Utilities
 *
 ************************************************************/
#ifndef __ONLPLIB_UTIL_H__
#define __ONLPLIB_UTIL_H__

#include <onlplib/onlplib_config.h>

#define ONLP_BITMASK(_bit) ( 1 << (_bit))

/**
 * Set and & or masks for active low bits.
 */
#define ONLP_ACTIVE_LOW_MASKS(_enable, _bitmask, _andmask, _ormask)     \
    do {                                                                \
        _andmask = (_enable) ? ~(_bitmask) : ~0;                        \
        _ormask = (_enable) ? 0 : (_bitmask);                           \
    } while(0)

#define ONLP_ACTIVE_LOW_MASKS_BIT(_enable, _bit, _andmask, _ormask)     \
    ONLP_ACTIVE_LOW_MASKS(_enable, (1 << (_bit)), _andmask, _ormask)

/**
 * Set and & or masks for active high bits.
 */
#define ONLP_ACTIVE_HIGH_MASKS(_enable, _bitmask, _andmask, _ormask)    \
    do {                                                                \
        _andmask = (_enable) ? ~0 : ~(_bitmask);                        \
        _ormask = (_enable) ? (_bitmask) : 0;                           \
    } while(0)

#define ONLP_ACTIVE_HIGH_MASKS_BIT(_enable, _bit, _andmask, _ormask)    \
    ONLP_ACTIVE_HIGH_MASKS(_enable, (1 << (_bit)), _andmask, _ormask)



/**
 * Range Checking
 */
#define ONLP_IN_RANGE(_v, _first, _last) \
    (((_v) >= _first) && ((_v) <= _last))

/**
 * Bit Positions
 */
#define ONLP_BIT_POSITION(_size, _v, _offset, _bit)     \
    do {                                                \
        _offset = (_v) / (_size);                       \
        _bit = (_v) % (_size);                          \
    } while(0)

#define ONLP_BITMASK_POSITION(_size, _v, _offset, _bitmask)     \
    do {                                                        \
        ONLP_BIT_POSITION(_size, _v, _offset, _bitmask);        \
        _bitmask = (1 << _bitmask);                             \
    } while(0)

#define ONLP_BIT_POSITION_8(_v, _offset, _bit) \
    ONLP_BIT_POSITION(8, _v, _offset, _bit)
#define ONLP_BITMASK_POSITION_8(_v, _offset, _bitmask) \
    ONLP_BITMASK_POSITION(8, _v, _offset, _bitmask)

#define ONLP_BIT_POSITION_16(_v, _offset, _bit) \
    ONLP_BIT_POSITION(16, _v, _offset, _bit)
#define ONLP_BITMASK_POSITION_16(_v, _offset, _bitmask) \
    ONLP_BITMASK_POSITION(16, _v, _offset, _bitmask)

#define ONLP_BIT_POSITION_32(_v, _offset, _bit) \
    ONLP_BIT_POSITION(32, _v, _offset, _bit)
#define ONLP_BITMASK_POSITION_32(_v, _offset, _bitmask) \
    ONLP_BITMASK_POSITION(32, _v, _offset, _bitmask)


#define ONLP_VALIDATED_READ_AND_SHIFT_8(_read, _dst, _error)            \
    do {                                                                \
        int _status = _read ;                                           \
        if(_status < 0) {                                               \
            return _error ;                                             \
        }                                                               \
        else {                                                          \
            _dst <<= 8;                                                 \
            _dst |= _status;                                            \
        }                                                               \
    } while(0)



#endif /* __ONLPLIB_UTIL_H__ */
