#ifndef SWITCH_H
#define SWITCH_H
#ifdef C11_ANNEX_K
#include "libboundscheck/include/securec.h"
#endif

#ifndef FALSE
#define FALSE   0
#endif

#ifndef TRUE
#define TRUE    (!FALSE)
#endif

enum LOG_LEVEL{
    ERR         = 0x01,
    WARNING     = 0x02,
    INFO        = 0x04,
    DBG         = 0x08,
};

#define LOG_ERR(_prefix, fmt, args...) \
    do { \
        if (loglevel & ERR) \
        { \
            printk( KERN_ERR _prefix "%s "fmt, __FUNCTION__, ##args); \
        } \
    } while (0)

#define LOG_WARNING(_prefix, fmt, args...) \
    do { \
        if (loglevel & WARNING) \
        { \
            printk( KERN_WARNING _prefix "%s "fmt, __FUNCTION__, ##args); \
        } \
    } while (0)

#define LOG_INFO(_prefix, fmt, args...) \
    do { \
        if (loglevel & INFO) \
        { \
            printk( KERN_INFO _prefix "%s "fmt, __FUNCTION__, ##args); \
        } \
    } while (0)

#define LOG_DBG(_prefix, fmt, args...) \
    do { \
        if (loglevel & DBG) \
        { \
            printk( KERN_DEBUG _prefix "%s "fmt, __FUNCTION__, ##args); \
        } \
    } while (0)

// For S3IP always return success with NA for HW fail.
#ifdef C11_ANNEX_K
#define ERROR_RETURN_NA(ret) do { return sprintf_s(buf, PAGE_SIZE, "%s\n", "NA"); } while(0)
#else
#define ERROR_RETURN_NA(ret) do { return sprintf(buf, "%s\n", "NA"); } while(0)
#endif

#endif /* SWITCH_H */
