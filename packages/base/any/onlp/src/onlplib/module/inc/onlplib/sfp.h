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
 * Common SFP support routines.
 *
 ************************************************************/
#ifndef __ONLPLIB_SFP_H__
#define __ONLPLIB_SFP_H__
#include <onlplib/onlplib_config.h>

/**
 *
 * @brief Determine SFP Presence from the given file.
 * @param fname The filename
 * @param present The string value that indicates 'SFP is present'.
 * @param notpresent The string valud that indicates 'SFP is absent'.
 * @notes If your SFP module presence is indicated through
 * a file (like a GPIO) then you can use this to implement
 * your onlp_sfpi_is_present() interface.
 */

int onlplib_sfp_is_present_file(const char* fname,
                                const char* present,
                                const char* notpresent);
/**
 * @brief Reset an SFP using writes to the given file.
 * @param fname The filename.
 * @param first This string will be written to the file.
 * @param delay The amount of time to wait before the second write.
 * @param second If specified, this string will be written to the
 * the file after waiting the given delay.
 * @notes If your SFP module reset is performed through
 * a file (like a GPIO) you can use this function to implement your
 * onlp_sfpi_reset() vector.
 */
int onlplib_sfp_reset_file(const char* file, const char* first, int delay_ms,
                           const char* second);
/**
 * @brief Read an SFP eeprom from the given file.
 * @param fname The filename.
 * @param data Receives the data.
 * @notes If your SFP module's eeprom is exported via a file
 * (usually in /sys) then you can use this function
 * to implement your onlp_sfpi_eeprom_read() interface. */
int onlplib_sfp_eeprom_read_file(const char* fname, uint8_t data[256]);

#endif /* __ONLPLIB_SFP_H__ */
