/************************************************************
 * <bsn.cl fy=2014 v=onl>
 * </bsn.cl>
 ************************************************************
 *
 *
 *
 ***********************************************************/
#include <x86_64_quanta_ly4r/x86_64_quanta_ly4r_config.h>
#include <x86_64_quanta_ly4r/x86_64_quanta_ly4r_gpio_table.h>
#include <onlp/platformi/psui.h>
#include <onlplib/file.h>
#include <onlplib/gpio.h>
#include "x86_64_quanta_ly4r_int.h"
#include "x86_64_quanta_ly4r_log.h"
#include <AIM/aim_string.h>

int
onlp_psui_init(void)
{
    AIM_LOG_MSG("ONLP is not supported for RPSU");
    return ONLP_STATUS_E_UNSUPPORTED;
}
