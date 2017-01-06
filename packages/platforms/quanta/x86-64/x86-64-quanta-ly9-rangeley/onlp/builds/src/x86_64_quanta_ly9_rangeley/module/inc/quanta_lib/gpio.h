#ifndef __QUANTA_LIB_GPIO_H__
#define __QUANTA_LIB_GPIO_H__

#define GPIO_LOW	0
#define GPIO_HIGH	1

#define GPIO_IN		0
#define GPIO_OUT	1

#define GPIO_PATH		"/sys/class/gpio"
#define GPIO_EXPORT		GPIO_PATH "/export"
#define GPIO_UNEXPORT	GPIO_PATH "/unexport"
#define GPIO_PREF		GPIO_PATH "/gpio"

int pca953x_gpio_value_get(int gpio, int *value);
int pca953x_gpio_direction_set(int gpio, int direction);
int pca953x_gpio_value_set(int gpio, int value);

#endif /* __QUANTA_LIB_GPIO_H__ */
