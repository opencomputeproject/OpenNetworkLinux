#include <linux/i2c.h>
//#include <linux/i2c-algo-bit.h>
#include <linux/i2c-gpio.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/platform_device.h>

#include <linux/i2c/pca954x.h>
#include <linux/platform_data/pca953x.h>
#include <linux/platform_data/at24.h>

struct inv_i2c_board_info {
    int ch;
    int size;
    struct i2c_board_info *board_info;
};

#define bus_id(id)  (id)
static struct pca954x_platform_mode mux_modes_0[] = {
    {.adap_id = bus_id(2),},    {.adap_id = bus_id(3),},
    {.adap_id = bus_id(4),},    {.adap_id = bus_id(5),},
    {.adap_id = bus_id(6),},    {.adap_id = bus_id(7),},
    {.adap_id = bus_id(8),},    {.adap_id = bus_id(9),},
};

static struct pca954x_platform_data mux_data_0 = {
        .modes          = mux_modes_0,
        .num_modes      = 8,
};

static struct i2c_board_info i2c_device_info0[] __initdata = {
//        {"inv_psoc",         0, 0x66, 0, 0, 0},//psoc
//        {"inv_cpld",         0, 0x55, 0, 0, 0},//cpld
        {"pca9548",          0, 0x71, &mux_data_0, 0, 0},	
};

static struct inv_i2c_board_info i2cdev_list[] = {
    {0, ARRAY_SIZE(i2c_device_info0),  i2c_device_info0 },  //smbus 0
};

#if 0
static struct 	i2c_gpio_platform_data 	i2c_gpio_platdata = {
	.scl_pin = 460,//8,
	.sda_pin = 461,//9,
    
	.udelay  = 5, //5:100kHz
        .timeout = 100,
	.sda_is_open_drain = 0,
	.scl_is_open_drain = 0,
	.scl_is_output_only = 0
};

static struct 	platform_device 	inventec_device_i2c_gpio = {
	.name 	= "i2c-gpio",
	.id  	= 0, // adapter number
	.dev.platform_data = &i2c_gpio_platdata,
};
#else
static struct   i2c_gpio_platform_data  i2c_gpio_platdata2 = {
        .scl_pin = 464,//12+452,
        .sda_pin = 463,//11+452,

        .udelay  = 5, //5:100kHz
        .timeout = 100,
        .sda_is_open_drain = 0,
        .scl_is_open_drain = 0,
        .scl_is_output_only = 0
};

static struct   platform_device         inventec_device_i2c_gpio2 = {
        .name   = "i2c-gpio",
        .id     = 1, // adapter number
        .dev.platform_data = &i2c_gpio_platdata2,
};
#endif

static int __init plat_inventec_init(void)
{
    struct i2c_adapter *adap = NULL;
    struct i2c_client *e = NULL;
    int ret = 0;
    int i,j;

    //use i2c-gpio    
    //register i2c gpio
    //config gpio8,9 gpio12,11 to gpio function
#if 0
    outl( inl(0x500) | (1<<8 | 1<<9 ), 0x500);
   
    ret = platform_device_register(&inventec_device_i2c_gpio);
    if (ret) {
	printk(KERN_ERR "i2c-gpio: device_i2c_gpio register fail %d\n", ret);
    }
    else {
        printk(KERN_INFO "i2c-gpio: i2c-0 Success.\n");
    }
#endif
    outl( inl(0x500) | (1<<11 | 1<<12), 0x500);
    ret = platform_device_register(&inventec_device_i2c_gpio2);
    if (ret) {
        printk(KERN_ERR "i2c-gpio: device_i2c_gpio2 register fail %d\n", ret);
    }
    else {
        printk(KERN_INFO "i2c-gpio: i2c-1 Success.\n");
    }
    for(i=0; i<ARRAY_SIZE(i2cdev_list); i++) {
        
        adap = i2c_get_adapter( i2cdev_list[i].ch );
        if (adap == NULL) {
            printk(" cottonwood get channel %d adapter fail\n", i);
            continue;
            return -ENODEV;
        }
    
        i2c_put_adapter(adap);
        for(j=0; j<i2cdev_list[i].size; j++) {
            e = i2c_new_device(adap, &i2cdev_list[i].board_info[j] );
     }
    }

    return ret;    
}
module_init(plat_inventec_init);

MODULE_AUTHOR("Inventec");
MODULE_DESCRIPTION("Inventec Platform devices");
MODULE_LICENSE("GPL");
