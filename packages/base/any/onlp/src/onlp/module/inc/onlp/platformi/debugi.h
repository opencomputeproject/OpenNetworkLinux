/**************************************************************************//**
 *
 *
 *
 *
 *****************************************************************************/
#ifndef __ONLP_DEBUGI_H__
#define __ONLP_DEBUGI_H__

#include <onlp/debug.h>

/**
 * @brief Generic debug tool entry point.
 * @note This does not hold the API lock.
 */
int onlp_debugi(aim_pvs_t* pvs, int argc, char* argv[]);

#endif /* __ONLP_DEBUGI_H__ */
