#ifndef __INV_DEF_H
#define __INV_DEF_H

/*customized definition*/

#define BUF_SIZE (PAGE_SIZE)
#define DEFAULT_CLTE (4)
/* <TBD> use enum to define reply code*/
/*use define temporary*/
#define REC_SFF_IO_UNSUPPORTED (0x06)

//#include "inv_type.h"
typedef enum {

    PLATFORM_MAPLE,
    PLATFORM_BANYAN,
    PLATFORM_CEDAR,
    PLATFORM_CYPRESS,
    PLATFORM_END,

} platform_name_t;

#define PLATFORM_NAME (PLATFORM_BANYAN)
#define SWPS_VERSION ("swps_1U_v1.0.3")

#endif /*__INV_DEF_H*/
