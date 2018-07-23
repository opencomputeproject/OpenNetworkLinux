#include <onlp/platformi/sfpi.h>
#include <sff/sff.h>

int
onlp_sfpi_sw_init(void)
{
    return 0;
}

int
onlp_sfpi_hw_init(uint32_t flags)
{
    return 0;
}

int
onlp_sfpi_info_get(onlp_oid_t port, onlp_sfp_info_t* info)
{
    return ONLP_STATUS_E_MISSING;
}

int
onlp_sfpi_bitmap_get(onlp_sfp_bitmap_t* bmap)
{
    AIM_BITMAP_CLR_ALL(bmap);
    return 0;
}

int
onlp_sfpi_is_present(int port)
{
    return ONLP_STATUS_E_MISSING;
}

int
onlp_sfpi_presence_bitmap_get(onlp_sfp_bitmap_t* dst)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

int
onlp_sfpi_rx_los_bitmap_get(onlp_sfp_bitmap_t* dst)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}
