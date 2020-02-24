#include <linux/version.h>
#include <linux/i2c.h>
//#include <linux/i2c-algo-bit.h>
#include <linux/i2c-gpio.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/platform_device.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,14,0)
#include <linux/platform_data/pca954x.h>
#else
#include <linux/i2c/pca954x.h>
#endif
#include <linux/platform_data/pca953x.h>
#include <linux/platform_data/at24.h>

//#include <asm/gpio.h>
#define IO_EXPAND_BASE    64
#define IO_EXPAND_NGPIO   16

struct inv_i2c_board_info {
    int ch;
    int size;
    struct i2c_board_info *board_info;
};

#define bus_id(id)  (id)
static struct pca954x_platform_mode mux_modes_0[] = {
    {.adap_id = bus_id(1),},    {.adap_id = bus_id(2),},
    {.adap_id = bus_id(3),},    {.adap_id = bus_id(4),},
    {.adap_id = bus_id(5),},    {.adap_id = bus_id(8),},
};
static struct pca954x_platform_mode mux_modes_0_0[] = {
    {.adap_id = bus_id(17),},    {.adap_id = bus_id(18),},
    {.adap_id = bus_id(19),},    {.adap_id = bus_id(20),},
    {.adap_id = bus_id(21),},    {.adap_id = bus_id(22),},
    {.adap_id = bus_id(23),},    {.adap_id = bus_id(24),},
};

static struct pca954x_platform_mode mux_modes_0_1[] = {
    {.adap_id = bus_id(25),},    {.adap_id = bus_id(26),},
    {.adap_id = bus_id(27),},    {.adap_id = bus_id(28),},
    {.adap_id = bus_id(29),},    {.adap_id = bus_id(30),},
    {.adap_id = bus_id(31),},    {.adap_id = bus_id(32),},
};

static struct pca954x_platform_mode mux_modes_0_2[] = {
    {.adap_id = bus_id(33),},    {.adap_id = bus_id(34),},
    {.adap_id = bus_id(35),},    {.adap_id = bus_id(36),},
    {.adap_id = bus_id(37),},    {.adap_id = bus_id(38),},
    {.adap_id = bus_id(39),},    {.adap_id = bus_id(40),},
};

static struct pca954x_platform_mode mux_modes_0_3[] = {
    {.adap_id = bus_id(41),},    {.adap_id = bus_id(42),},
    {.adap_id = bus_id(43),},    {.adap_id = bus_id(44),},
    {.adap_id = bus_id(45),},    {.adap_id = bus_id(46),},
    {.adap_id = bus_id(47),},    {.adap_id = bus_id(48),},
};


static struct pca954x_platform_mode mux_modes_0_7[] = {
    {.adap_id = bus_id(9),},    {.adap_id = bus_id(10),},
    {.adap_id = bus_id(11),},    {.adap_id = bus_id(12),},
};
static struct pca954x_platform_mode mux_modes_0_7_0[] = {
    {.adap_id = bus_id(49),},    {.adap_id = bus_id(50),},
    {.adap_id = bus_id(51),},    {.adap_id = bus_id(52),},
    {.adap_id = bus_id(53),},    {.adap_id = bus_id(54),},
    {.adap_id = bus_id(55),},    {.adap_id = bus_id(56),},
};

static struct pca954x_platform_mode mux_modes_0_7_1[] = {
    {.adap_id = bus_id(57),},    {.adap_id = bus_id(58),},
    {.adap_id = bus_id(59),},    {.adap_id = bus_id(60),},
    {.adap_id = bus_id(61),},    {.adap_id = bus_id(62),},
    {.adap_id = bus_id(63),},    {.adap_id = bus_id(64),},
};

static struct pca954x_platform_mode mux_modes_0_7_2[] = {
    {.adap_id = bus_id(65),},    {.adap_id = bus_id(66),},
    {.adap_id = bus_id(67),},    {.adap_id = bus_id(68),},
    {.adap_id = bus_id(69),},    {.adap_id = bus_id(70),},
    {.adap_id = bus_id(71),},    {.adap_id = bus_id(72),},
};

static struct pca954x_platform_mode mux_modes_0_7_3[] = {
    {.adap_id = bus_id(73),},    {.adap_id = bus_id(74),},
    {.adap_id = bus_id(75),},    {.adap_id = bus_id(76),},
    {.adap_id = bus_id(77),},    {.adap_id = bus_id(78),},
    {.adap_id = bus_id(79),},    {.adap_id = bus_id(80),},
};


static struct pca954x_platform_data mux_data_0 = {
        .modes          = mux_modes_0,
        .num_modes      = 6,
};
static struct pca954x_platform_data mux_data_0_0 = {
        .modes          = mux_modes_0_0,
        .num_modes      = 8,
};
static struct pca954x_platform_data mux_data_0_1 = {
        .modes          = mux_modes_0_1,
        .num_modes      = 8,
};
static struct pca954x_platform_data mux_data_0_2 = {
        .modes          = mux_modes_0_2,
        .num_modes      = 8,
};
static struct pca954x_platform_data mux_data_0_3 = {
        .modes          = mux_modes_0_3,
        .num_modes      = 8,
};


static struct pca954x_platform_data mux_data_0_7 = {
        .modes          = mux_modes_0_7,
        .num_modes      = 4,
};
static struct pca954x_platform_data mux_data_0_7_0 = {
        .modes          = mux_modes_0_7_0,
        .num_modes      = 8,
};
static struct pca954x_platform_data mux_data_0_7_1 = {
        .modes          = mux_modes_0_7_1,
        .num_modes      = 8,
};
static struct pca954x_platform_data mux_data_0_7_2 = {
        .modes          = mux_modes_0_7_2,
        .num_modes      = 8,
};
static struct pca954x_platform_data mux_data_0_7_3 = {
        .modes          = mux_modes_0_7_3,
        .num_modes      = 8,
};


static struct i2c_board_info i2c_device_info0[] __initdata = {
        {"inv_cpld",         0, 0x55, 0, 0, 0},
        {"pca9548",          0, 0x70, &mux_data_0, 0, 0},	
};

static struct i2c_board_info i2c_device_info1[] __initdata = {
        {"pca9548",         0, 0x72, &mux_data_0_0, 0, 0},	
};

static struct i2c_board_info i2c_device_info2[] __initdata = {
        {"pca9548",         0, 0x72, &mux_data_0_1, 0, 0},	
};

static struct i2c_board_info i2c_device_info3[] __initdata = {
        {"pca9548",         0, 0x72, &mux_data_0_2, 0, 0},	
};

static struct i2c_board_info i2c_device_info4[] __initdata = {
        {"pca9548",         0, 0x72, &mux_data_0_3, 0, 0},	
};

static struct i2c_board_info i2c_device_info5[] __initdata = {
        {"pca9548",         0, 0x72, &mux_data_0_3, 0, 0},
};

static struct i2c_board_info i2c_device_info8[] __initdata = {
        {"inv_cpld",        0, 0x77, 0, 0, 0},
        {"pca9548",         0, 0x71, &mux_data_0_7, 0, 0},
};

static struct i2c_board_info i2c_device_info9[] __initdata = {
        {"pca9548",         0, 0x72, &mux_data_0_7_0, 0, 0},
};

static struct i2c_board_info i2c_device_info10[] __initdata = {
        {"pca9548",         0, 0x72, &mux_data_0_7_1, 0, 0},
};

static struct i2c_board_info i2c_device_info11[] __initdata = {
        {"pca9548",         0, 0x72, &mux_data_0_7_2, 0, 0},
};

static struct i2c_board_info i2c_device_info12[] __initdata = {
        {"pca9548",         0, 0x72, &mux_data_0_7_3, 0, 0},
};

static struct inv_i2c_board_info i2cdev_list[] = {
    {0, ARRAY_SIZE(i2c_device_info0),  i2c_device_info0 },  //smbus 0, mux0
    {8, ARRAY_SIZE(i2c_device_info8),  i2c_device_info8 },  //mux9

    {bus_id(1), ARRAY_SIZE(i2c_device_info1),  i2c_device_info1 },  //mux1
    {bus_id(2), ARRAY_SIZE(i2c_device_info2),  i2c_device_info2 },  //mux2
    {bus_id(3), ARRAY_SIZE(i2c_device_info3),  i2c_device_info3 },  //mux3
    {bus_id(4), ARRAY_SIZE(i2c_device_info4),  i2c_device_info4 },  //mux4
    {bus_id(5), ARRAY_SIZE(i2c_device_info5),  i2c_device_info5 },  

    {bus_id(9), ARRAY_SIZE(i2c_device_info9),  i2c_device_info9 },    //mux5
    {bus_id(10), ARRAY_SIZE(i2c_device_info10),  i2c_device_info10 }, //mux6 
    {bus_id(11), ARRAY_SIZE(i2c_device_info11),  i2c_device_info11 }, //mux7 
    {bus_id(12), ARRAY_SIZE(i2c_device_info12),  i2c_device_info12 }, //mux8
};

/////////////////////////////////////////////////////////////////////////////////////////
static struct 	i2c_gpio_platform_data 	i2c_gpio_platdata0 = {
	.scl_pin = 8,
	.sda_pin = 9,
    
	.udelay  = 5, //5:100kHz
	.sda_is_open_drain = 0,
	.scl_is_open_drain = 0,
	.scl_is_output_only = 0
};

static struct 	i2c_gpio_platform_data 	i2c_gpio_platdata1 = {
	.scl_pin = 12,
	.sda_pin = 11,
    
	.udelay  = 5, //5:100kHz
	.sda_is_open_drain = 0,
	.scl_is_open_drain = 0,
	.scl_is_output_only = 0
};

static struct 	platform_device 	device_i2c_gpio0 = {
	.name 	= "i2c-gpio",
	.id  	= 0, // adapter number
	.dev.platform_data = &i2c_gpio_platdata0,
};

static struct 	platform_device 	device_i2c_gpio1 = {
	.name 	= "i2c-gpio",
	.id  	= 1, // adapter number
	.dev.platform_data = &i2c_gpio_platdata1,
};

static int __init plat_lavender_x86_init(void)
{
    struct i2c_adapter *adap = NULL;
    struct i2c_client *e = NULL;
    int ret = 0;
    int i,j;

    printk("el6661 plat_lavender_x86_init  \n");

#if 0  //disable for ICOS
    //use i2c-gpio    
    //register i2c gpio
    //config gpio8,9 to gpio function
    outl( inl(0x500) | (1<<8 | 1<<9), 0x500);
    
	ret = platform_device_register(&device_i2c_gpio0);
	if (ret) {
		printk(KERN_ERR "i2c-gpio: device_i2c_gpio0 register fail %d\n", ret);
	}
	
    outl( inl(0x500) | (1<<11 | 1<<12), 0x500);
	ret = platform_device_register(&device_i2c_gpio1);
	if (ret) {
		printk(KERN_ERR "i2c-gpio: device_i2c_gpio1 register fail %d\n", ret);
	}
#endif
    
    for(i=0; i<ARRAY_SIZE(i2cdev_list); i++) {
        
        adap = i2c_get_adapter( i2cdev_list[i].ch );
        if (adap == NULL) {
            printk("lavender_x86 get channel %d adapter fail\n", i);
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

module_init(plat_lavender_x86_init);
//arch_initcall(plat_lavender_x86_init);

MODULE_AUTHOR("Inventec");
MODULE_DESCRIPTION("Lavender_x86 Platform devices");
MODULE_LICENSE("GPL");
