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
 * CRC32 Calculations
 *
 ***********************************************************/
#ifndef __ONLP_CRC32_H__
#define __ONLP_CRC32_H__

#include <stdint.h>

/**
 * @brief Calculate CRC32 on the given data buffer.
 * @param crc CRC start
 * @param buf The data buffer
 * @param size The size of the data buffer.
 * @returns The CRC32 value.
 */

uint32_t onlp_crc32(uint32_t crc, const void *buf, int size);

#endif /* __ONLP_CRC32_H__ */
