#ifndef __ONLP_PLATFORMI_COMMON_H__
#define __ONLP_PLATFORMI_COMMON_H__

/*
 * Interchange includes between subsystems
 */
#include <onlp/platformi/chassisi.h>
#include <onlp/platformi/attributei.h>
#include <onlp/platformi/fani.h>
#include <onlp/platformi/psui.h>
#include <onlp/platformi/thermali.h>
#include <onlp/platformi/sfpi.h>
#include <onlp/platformi/ledi.h>
#include <onlp/platformi/platformi.h>
#include <onlp/platformi/debugi.h>

/*
 * Common includes needed for subsystem implementations.
 */
#include <onlplib/i2c.h>
#include <onlplib/file.h>
#include <onlplib/mmap.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdio.h>
#include <string.h>

#endif /* __ONLP_PLATFORMI_COMMON_H__ */
