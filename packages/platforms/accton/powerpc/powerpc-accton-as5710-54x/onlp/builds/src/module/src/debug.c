#include "powerpc_accton_as5710_54x_int.h"

#if POWERPC_ACCTON_AS5710_54X_R0_CONFIG_INCLUDE_DEBUG == 1

#include <unistd.h>

static char help__[] =
    "Usage: debug [options]\n"
    "    -c CPLD Versions\n"
    "    -h Help\n"
    ;


static void
print_cpld_version__(void)
{
    /** Temporary until the ONLP I2C support is enabled for PPC */
    system("/usr/sbin/i2cget -y -f 3 0x60 0x1");
    system("/usr/sbin/i2cget -y -f 3 0x61 0x1");
    system("/usr/sbin/i2cget -y -f 3 0x62 0x1");
}


int
powerpc_accton_as5710_54x_debug_main(int argc, char* argv[])
{
    int c = 0;
    int help = 0;
    int rv = 0;

    while( (c = getopt(argc, argv, "ch")) != -1) {
        switch(c)
            {
            case 'c': c = 1; break;
            case 'h': help = 1; rv = 0; break;
            default: help = 1; rv = 1; break;
            }

    }

    if(help || argc == 1) {
        printf("%s", help__);
        return rv;
    }

    if(c) {
        print_cpld_version__();
    }


    return 0;
}

#endif


