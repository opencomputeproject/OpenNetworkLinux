#ifndef __INV_DEF_H
#define __INV_DEF_H

/*customized definition*/

#define DEFAULT_CLTE (4)
/* <TBD> use enum to define reply code*/
/*use define temporary*/
#define REC_SFF_IO_UNSUPPORTED (0x06)

//#include "inv_type.h"
typedef enum {

    PLATFORM_MAPLE,
    PLATFORM_BANYAN,
    PLATFORM_CEDAR,
    PLATFORM_4U,
    PLATFORM_END,

} platform_id_t;

#define PLATFORM_ID (PLATFORM_BANYAN)
#define SWPS_VERSION ("SWPS_GA_v0.0.8")

#endif /*__INV_DEF_H*/
