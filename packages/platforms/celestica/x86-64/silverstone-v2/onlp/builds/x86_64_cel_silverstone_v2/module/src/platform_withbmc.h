#ifndef _PLATFORM_SILVERSTON_V2_H_
#define _PLATFORM_SILVERSTON_V2_H_
#include <stdint.h>


//CPLD
#define NETFN 0x3A
#define CPLD_OME 0x64
#define FAN_CPLD_NUMBER 2
#define R_FLAG 1

#define ONLP_SENSOR_CACHE_SHARED "/onlp-sensor-cache-shared"
#define ONLP_FRU_CACHE_SHARED "/onlp-fru-cache-shared"
#define ONLP_SENSOR_LIST_CACHE_SHARED "/onlp-sensor-list-cache-shared"

#define ONLP_SENSOR_CACHE_SEM "/onlp-sensor-cache-sem"
#define ONLP_FRU_CACHE_SEM "/onlp-fru-cache-sem"
#define ONLP_SENSOR_LIST_SEM "/onlp-sensor-list-cache-sem"

#define ONLP_SENSOR_CACHE_FILE "/tmp/onlp-sensor-cache.txt"
#define ONLP_FRU_CACHE_FILE "/tmp/onlp-fru-cache.txt"
#define ONLP_SENSOR_LIST_FILE "/tmp/onlp-sensor-list-cache.txt"


#endif /* _PLATFORM_SEASTONE_H_ */
