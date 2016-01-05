#include <onlp/onlp.h>
#include <onlplib/file.h>
#include <quanta_lib/gpio.h>
#include <sys/stat.h>
#include <limits.h>

static int file_exists(char *filename) {
    struct stat st;
    return (stat(filename, &st) == 0);
}

static int try_export_gpio(int gpio) {
    char filename[PATH_MAX];

    memset(filename, 0, sizeof(filename));
    sprintf(filename, "%s%d/value", GPIO_PREF, gpio);

    if(!file_exists(filename)) {
        onlp_file_write_int(gpio, GPIO_EXPORT);
    }

    return ONLP_STATUS_OK;
}

static int try_unexport_gpio(int gpio) {
    char filename[PATH_MAX];

    memset(filename, 0, sizeof(filename));
    sprintf(filename, "%s%d/value", GPIO_PREF, gpio);

    if(file_exists(filename)) {
		onlp_file_write_int(gpio, GPIO_UNEXPORT);
	}

	return ONLP_STATUS_OK;
}

int pca953x_gpio_value_get(int gpio, int *value) {
	int ret;

    try_export_gpio(gpio);
    ret = onlp_file_read_int(value, "%s%d/value", GPIO_PREF, gpio);
    try_unexport_gpio(gpio);

			return ret;
	}

int pca953x_gpio_direction_set(int gpio, int direction) {
    int ret;

    try_export_gpio(gpio);

	switch(direction) {
		case GPIO_IN:
			ret = onlp_file_write_str("in", "%s%d/direction", GPIO_PREF, gpio);
			break;

		case GPIO_OUT:
	ret = onlp_file_write_str("out", "%s%d/direction", GPIO_PREF, gpio);
			break;

		default:
			ret = ONLP_STATUS_E_UNSUPPORTED;
			break;
	}

    try_unexport_gpio(gpio);

		return ret;
}

int pca953x_gpio_value_set(int gpio, int value) {
    int ret;

    pca953x_gpio_direction_set(gpio, GPIO_OUT);

    try_export_gpio(gpio);
	ret = onlp_file_write_int(value, "%s%d/value", GPIO_PREF, gpio);
    try_unexport_gpio(gpio);

		return ret;
}
