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
#ifndef __ONLPLIB_FILE_H__
#define __ONLPLIB_FILE_H__

#include <onlplib/onlplib_config.h>

/**
 * @brief Read the size of the given file.
 * @param fmt Filename format string.
 * @param vargs Filename format arguments.
 */
int onlp_file_vsize(const char* fmt, va_list vargs);

/**
 * @brief Read the size of the given file.
 * @param fmt Filename format string.
 * @param ... Filename format arguments.
 */
int onlp_file_size(const char* fmt, ...);

/**
 * @brief Read and return the contents of the given file.
 * @param data Receives the data.
 * @param max Maximum read size.
 * @param len Receives the actual read length.
 * @param fmt The filename format string.
 * @param vargs The filename format string arguments.
 */
int onlp_file_vread(uint8_t* data, int max, int* len, const char* fmt, va_list vargs);

/**
 * @brief Read and return the contents of the given file.
 * @param data Receives the data.
 * @param max Maximum read size.
 * @param len Receives the actual read length.
 * @param fmt The filename format string.
 * @param ... The filename format string arguments.
 */
int onlp_file_read(uint8_t* data, int max, int* len, const char* fmt, ...);


/**
 * @brief Read and return the contents of the given file.
 * @param[out] data Receives the contents.
 * @param fmt The filename format string.
 * @param vargs The filename format args.
 */
int onlp_file_vread_all(uint8_t** data, const char* fmt, va_list vargs);

/**
 * @brief Read and return the contents of the given file.
 * @param[out] data Receives the contents.
 * @param fmt The filename format string.
 * @param ... The filename format args.
 */
int onlp_file_read_all(uint8_t** data, const char* fmt, ...);

/**
 * @brief Read and return the contents of the given file.
 * @param[out] rv Receives the contents.
 * @param fmt The filename format string.
 * @param vargs The filename format args.
 * @note The contents of the file are assumed to be a string.
 *       Trailing newlines are removed.
 */
int onlp_file_vread_str(char** str, const char* fmt, va_list vargs);

/**
 * @brief Read and return the contents of the given file.
 * @param[out] rv Receives the contents.
 * @param fmt The filename format string.
 * @param ... The filename format args.
 * @note The contents of the file are assumed to be a string.
 *       Trailing newlines are removed.
 */
int onlp_file_read_str(char** str, const char* fmt, ...);

/**
 * @brief Read and return the integer contents of the given file.
 * @param value Receives the integer value.
 * @param fmt The filename format string.
 * @param vargs The filename format string arguments.
 */
int onlp_file_vread_int(int* value, const char* fmt, va_list vargs);

/**
 * @brief Read and return the integer contents of the given file.
 * @param value Receives the integer value.
 * @param fmt The filename format string.
 * @param ... The filename format string arguments.
 */
int onlp_file_read_int(int* value, const char* fmt, ...);

/**
 * @brief Read and return the maximum integer value contained in the given files.
 * @param value Receives the maximum integer value.
 * @param files Null terminated file list.
 */
int onlp_file_read_int_max(int* value, char** files);

/**
 * @brief Write data to the given file.
 * @param data The data to write.
 * @param len The length of the data.
 * @param fmt The filename format string.
 * @param vargs The filename format string arguments.
 */
int onlp_file_vwrite(uint8_t* data, int len, const char* fmt, va_list vargs);

/**
 * @brief Write data to the given file.
 * @param data The data to write.
 * @param len The length of the data.
 * @param fmt The filename format string.
 * @param ... The filename format string arguments.
 */
int onlp_file_write(uint8_t* data, int len, const char* fmt, ...);


/**
 * @brief Write a string to the given file.
 * @param str The string to write.
 * @param fmt The filename format string.
 * @param vargs The filename format string arguments.
 */
int onlp_file_vwrite_str(const char* str, const char* fmt, va_list vargs);


/**
 * @brief Write a string to the given file.
 * @param str The string to write.
 * @param fmt The filename format string.
 * @param ... The filename format string arguments.
 */
int onlp_file_write_str(const char* str, const char* fmt, ...);

/**
 * @brief Write an integer as a string to the given file.
 * @param value The integer.
 * @param fmt The filename format string.
 * @param vargs The filename format string arguments.
 */
int onlp_file_vwrite_int(int value, const char* fmt, va_list vargs);

/**
 * @brief Write an integer as a string to the given file.
 * @param value The integer.
 * @param fmt The filename format string.
 * @param ... The filename format string arguments.
 */
int onlp_file_write_int(int value, const char* fmt, ...);


/**
 * @brief Open a file.
 * @param flags The open flags
 * @param fmt The filename format string
 * @param ... The format arguments.
 */
int onlp_file_open(int flags, int log, const char* fmt, ...);


/**
 * @brief Open a file.
 * @param flags The open flags.
 * @param fmt The filename format string.
 * @param vargs The format arguments.
 */
int onlp_file_vopen(int flags, int log, const char* fmt, va_list vargs);


/**
 * @brief Search a directory tree for the given file.
 */
int onlp_file_find(char* root, char* fname, char** rpath);

#endif /* __ONLPLIB_FILE_H__ */
