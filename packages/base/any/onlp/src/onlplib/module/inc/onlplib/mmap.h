/************************************************************
 * <bsn.cl v=2014 v=onl>
 * 
 *           Copyright 2015 Big Switch Networks, Inc.          
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
 * Common mmap() platform support.
 *
 ***********************************************************/
#ifndef __ONLP_MMAP_H__
#define __ONLP_MMAP_H__

#include <onlplib/onlplib_config.h>
#include <unistd.h>
#include <sys/mman.h>

/**
 * @brief Map a physical address range.
 * @param addr The physical
 * @param size The size of the region to map.
 * @param name The name of the memory region for debugging/logging purposes.
 */
void* onlp_mmap(off_t pa, uint32_t size, const char* name);





#endif /* __ONLP_MMAP_H__ */
