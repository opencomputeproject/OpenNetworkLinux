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

//#include <asm/gpio.h>
#define IO_EXPAND_BASE    64
#define IO_EXPAND_NGPIO   16

struct inv_i2c_board_info {
    int ch;
    int size;
    struct i2c_board_info *board_info;
};

/////////////////////////////////////////////////////////////////////////////////////////
static struct at24_platform_data at24c64_eeprom_data = {
        .byte_len = 256,//SZ_64K/8
        .page_size = 1,
        .flags = 0,//AT24_FLAG_ADDR8,
};

static int	pca9555_setup(struct i2c_client *client, unsigned gpio, unsigned ngpio, void *context)
{
    //TBD
    printk("%s : gpio=%d, ngpio=%d\n ", __func__, gpio, ngpio);
    return 0;
}

static struct pca954x_platform_mode mux_modes_0[] = {
    {.adap_id = 2,},    {.adap_id = 3,},
    {.adap_id = 4,},    {.adap_id = 5,},
    {.adap_id = 6,},    {.adap_id = 7,},
    {.adap_id = 8,},    {.adap_id = 9,},
};
static struct pca954x_platform_mode mux_modes_0_0[] = {
    {.adap_id = 10,},    {.adap_id = 11,},
    {.adap_id = 12,},    {.adap_id = 13,},
    {.adap_id = 14,},    {.adap_id = 15,},
    {.adap_id = 16,},    {.adap_id = 17,},
};

static struct pca954x_platform_mode mux_modes_0_1[] = {
    {.adap_id = 18,},    {.adap_id = 19,},
    {.adap_id = 20,},    {.adap_id = 21,},
    {.adap_id = 22,},    {.adap_id = 23,},
    {.adap_id = 24,},    {.adap_id = 25,},
};

static struct pca954x_platform_mode mux_modes_0_2[] = {
    {.adap_id = 26,},    {.adap_id = 27,},
    {.adap_id = 28,},    {.adap_id = 29,},
    {.adap_id = 30,},    {.adap_id = 31,},
    {.adap_id = 32,},    {.adap_id = 33,},
};

static struct pca954x_platform_mode mux_modes_0_3[] = {
    {.adap_id = 34,},    {.adap_id = 35,},
    {.adap_id = 36,},    {.adap_id = 37,},
    {.adap_id = 38,},    {.adap_id = 39,},
    {.adap_id = 40,},    {.adap_id = 41,},
};

static struct pca954x_platform_mode mux_modes_0_4[] = {
    {.adap_id = 42,},    {.adap_id = 43,},
    {.adap_id = 44,},    {.adap_id = 45,},
    {.adap_id = 46,},    {.adap_id = 47,},
    {.adap_id = 48,},    {.adap_id = 49,},
};

static struct pca954x_platform_mode mux_modes_0_5[] = {
    {.adap_id = 50,},    {.adap_id = 51,},
    {.adap_id = 52,},    {.adap_id = 53,},
    {.adap_id = 54,},    {.adap_id = 55,},
    {.adap_id = 56,},    {.adap_id = 57,},
};

static struct pca954x_platform_mode mux_modes_0_6[] = {
    {.adap_id = 58,},    {.adap_id = 59,},
    {.adap_id = 60,},    {.adap_id = 61,},
    {.adap_id = 62,},    {.adap_id = 63,},
    {.adap_id = 64,},    {.adap_id = 65,},
};

//no i2c device driver attach to mux 7


static struct pca954x_platform_data mux_data_0 = {
        .modes          = mux_modes_0,
        .num_modes      = 8,
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
static struct pca954x_platform_data mux_data_0_4 = {
        .modes          = mux_modes_0_4,
        .num_modes      = 8,
};
static struct pca954x_platform_data mux_data_0_5 = {
        .modes          = mux_modes_0_5,
        .num_modes      = 8,
};
static struct pca954x_platform_data mux_data_0_6 = {
        .modes          = mux_modes_0_6,
        .num_modes      = 8,
};


#define IO_EXPAND_BASE_CHIP    (IO_EXPAND_BASE) //64
#define IO_EXPAND_BASE_CHIP00  (IO_EXPAND_BASE + IO_EXPAND_NGPIO)
#define IO_EXPAND_BASE_CHIP01  (IO_EXPAND_BASE_CHIP00 + IO_EXPAND_NGPIO)
#define IO_EXPAND_BASE_CHIP10  (IO_EXPAND_BASE_CHIP01 + IO_EXPAND_NGPIO)
#define IO_EXPAND_BASE_CHIP11  (IO_EXPAND_BASE_CHIP10 + IO_EXPAND_NGPIO)
#define IO_EXPAND_BASE_CHIP20  (IO_EXPAND_BASE_CHIP11 + IO_EXPAND_NGPIO)
#define IO_EXPAND_BASE_CHIP21  (IO_EXPAND_BASE_CHIP20 + IO_EXPAND_NGPIO)
#define IO_EXPAND_BASE_CHIP30  (IO_EXPAND_BASE_CHIP21 + IO_EXPAND_NGPIO)
#define IO_EXPAND_BASE_CHIP31  (IO_EXPAND_BASE_CHIP30 + IO_EXPAND_NGPIO)
#define IO_EXPAND_BASE_CHIP40  (IO_EXPAND_BASE_CHIP31 + IO_EXPAND_NGPIO)
#define IO_EXPAND_BASE_CHIP41  (IO_EXPAND_BASE_CHIP40 + IO_EXPAND_NGPIO)
#define IO_EXPAND_BASE_CHIP50  (IO_EXPAND_BASE_CHIP41 + IO_EXPAND_NGPIO)
#define IO_EXPAND_BASE_CHIP51  (IO_EXPAND_BASE_CHIP50 + IO_EXPAND_NGPIO)
#define IO_EXPAND_BASE_CHIP60  (IO_EXPAND_BASE_CHIP51 + IO_EXPAND_NGPIO)
#define IO_EXPAND_BASE_CHIP61  (IO_EXPAND_BASE_CHIP60 + IO_EXPAND_NGPIO)

static struct pca953x_platform_data   pca9555_data = {
    .gpio_base = IO_EXPAND_BASE_CHIP,
    .setup     = pca9555_setup,
};

#if 0
static struct pca953x_platform_data   pca9555_data00 = {
    .gpio_base = IO_EXPAND_BASE_CHIP00,
    .setup     = pca9555_setup,
};
static struct pca953x_platform_data   pca9555_data01 = {
    .gpio_base = IO_EXPAND_BASE_CHIP01,
    .setup     = pca9555_setup,
};
static struct pca953x_platform_data   pca9555_data10 = {
    .gpio_base = IO_EXPAND_BASE_CHIP10,
    .setup     = pca9555_setup,
};
static struct pca953x_platform_data   pca9555_data11 = {
    .gpio_base = IO_EXPAND_BASE_CHIP11,
    .setup     = pca9555_setup,
};
static struct pca953x_platform_data   pca9555_data20 = {
    .gpio_base = IO_EXPAND_BASE_CHIP20,
    .setup     = pca9555_setup,
};
static struct pca953x_platform_data   pca9555_data21 = {
    .gpio_base = IO_EXPAND_BASE_CHIP21,
    .setup     = pca9555_setup,
};
static struct pca953x_platform_data   pca9555_data30 = {
    .gpio_base = IO_EXPAND_BASE_CHIP30,
    .setup     = pca9555_setup,
};
static struct pca953x_platform_data   pca9555_data31 = {
    .gpio_base = IO_EXPAND_BASE_CHIP31,
    .setup     = pca9555_setup,
};
static struct pca953x_platform_data   pca9555_data40 = {
    .gpio_base = IO_EXPAND_BASE_CHIP40,
    .setup     = pca9555_setup,
};
static struct pca953x_platform_data   pca9555_data41 = {
    .gpio_base = IO_EXPAND_BASE_CHIP41,
    .setup     = pca9555_setup,
};
static struct pca953x_platform_data   pca9555_data50 = {
    .gpio_base = IO_EXPAND_BASE_CHIP50,
    .setup     = pca9555_setup,
};
static struct pca953x_platform_data   pca9555_data51 = {
    .gpio_base = IO_EXPAND_BASE_CHIP51,
    .setup     = pca9555_setup,
};
static struct pca953x_platform_data   pca9555_data60 = {
    .gpio_base = IO_EXPAND_BASE_CHIP60,
    .setup     = pca9555_setup,
};
static struct pca953x_platform_data   pca9555_data61 = {
    .gpio_base = IO_EXPAND_BASE_CHIP61,
    .setup     = pca9555_setup,
};
#endif

static struct i2c_board_info xlp_i2c_device_info0[] __initdata = {
//        {"24c02",            0, 0x57, &at24c64_eeprom_data, 0, 0},	//VPD
        {"inv_psoc",         0, 0x66, 0, 0, 0},//psoc
        {"inv_cpld",         0, 0x55, 0, 0, 0},//cpld
        {"pca9555",          0, 0x22, &pca9555_data, 0, 0},	
        {"pca9548",          0, 0x71, &mux_data_0, 0, 0},	
};

static struct i2c_board_info xlp_i2c_device_info1[] __initdata = {
        {"inv_psoc",         0, 0x66, 0, 0, 0},//psoc
        {"inv_cpld",         0, 0x55, 0, 0, 0},//cpld
};

static struct i2c_board_info xlp_i2c_device_info2[] __initdata = {
        {"pca9548",         0, 0x72, &mux_data_0_0, 0, 0},	
//        {"pca9555",         0, 0x20, &pca9555_data00, 0, 0},	
//        {"pca9555",         0, 0x21, &pca9555_data01, 0, 0},	
};

static struct i2c_board_info xlp_i2c_device_info3[] __initdata = {
        {"pca9548",         0, 0x72, &mux_data_0_1, 0, 0},	
//        {"pca9555",         0, 0x20, &pca9555_data10, 0, 0},	
//        {"pca9555",         0, 0x21, &pca9555_data11, 0, 0},	
};

static struct i2c_board_info xlp_i2c_device_info4[] __initdata = {
        {"pca9548",         0, 0x72, &mux_data_0_2, 0, 0},	
//        {"pca9555",         0, 0x20, &pca9555_data20, 0, 0},	
//        {"pca9555",         0, 0x21, &pca9555_data21, 0, 0},	
};

static struct i2c_board_info xlp_i2c_device_info5[] __initdata = {
        {"pca9548",         0, 0x72, &mux_data_0_3, 0, 0},	
//        {"pca9555",         0, 0x20, &pca9555_data30, 0, 0},	
//        {"pca9555",         0, 0x21, &pca9555_data31, 0, 0},	
};
static struct i2c_board_info xlp_i2c_device_info6[] __initdata = {
        {"pca9548",         0, 0x72, &mux_data_0_4, 0, 0},	
//        {"pca9555",         0, 0x20, &pca9555_data40, 0, 0},	
//        {"pca9555",         0, 0x21, &pca9555_data41, 0, 0},	
};
static struct i2c_board_info xlp_i2c_device_info7[] __initdata = {
        {"pca9548",         0, 0x72, &mux_data_0_5, 0, 0},	
//        {"pca9555",         0, 0x20, &pca9555_data50, 0, 0},	
//        {"pca9555",         0, 0x21, &pca9555_data51, 0, 0},	
};
static struct i2c_board_info xlp_i2c_device_info8[] __initdata = {
        {"pca9548",         0, 0x72, &mux_data_0_6, 0, 0},	
//        {"pca9555",         0, 0x20, &pca9555_data60, 0, 0},	
//        {"pca9555",         0, 0x21, &pca9555_data61, 0, 0},	
};


static struct inv_i2c_board_info i2cdev_list[] = {
    {0, ARRAY_SIZE(xlp_i2c_device_info0),  xlp_i2c_device_info0 },  //smbus 0
//    {1, ARRAY_SIZE(xlp_i2c_device_info1),  xlp_i2c_device_info1 },  //smbus 1 or gpio11+12
    
    {2, ARRAY_SIZE(xlp_i2c_device_info2),  xlp_i2c_device_info2 },  //mux 0
    {3, ARRAY_SIZE(xlp_i2c_device_info3),  xlp_i2c_device_info3 },  //mux 1
    {4, ARRAY_SIZE(xlp_i2c_device_info4),  xlp_i2c_device_info4 },  //mux 2
    {5, ARRAY_SIZE(xlp_i2c_device_info5),  xlp_i2c_device_info5 },  //mux 3
    {6, ARRAY_SIZE(xlp_i2c_device_info6),  xlp_i2c_device_info6 },  //mux 4
    {7, ARRAY_SIZE(xlp_i2c_device_info7),  xlp_i2c_device_info7 },  //mux 5
    {8, ARRAY_SIZE(xlp_i2c_device_info8),  xlp_i2c_device_info8 },  //mux 6
};

/////////////////////////////////////////////////////////////////////////////////////////
static struct 	i2c_gpio_platform_data 	i2c_gpio_platdata = {
	.scl_pin = 8,
	.sda_pin = 9,
    
	.udelay  = 5, //5:100kHz
	.sda_is_open_drain = 0,
	.scl_is_open_drain = 0,
	.scl_is_output_only = 0
};

static struct 	platform_device 	magnolia_device_i2c_gpio = {
	.name 	= "i2c-gpio",
	.id  	= 0, // adapter number
	.dev.platform_data = &i2c_gpio_platdata,
};

#define PLAT_MAX_I2C_CLIENTS 32
static struct i2c_client *plat_i2c_client[PLAT_MAX_I2C_CLIENTS];
static int num_i2c_clients = 0;
static int plat_i2c_client_add(struct i2c_client *e)
{
    if (num_i2c_clients >= PLAT_MAX_I2C_CLIENTS)
      return -1;

    plat_i2c_client[num_i2c_clients] = e;
    num_i2c_clients++;
    return num_i2c_clients;
}

static void plat_i2c_client_remove_all(void);

static void plat_i2c_client_remove_all()
{
    int i;
    for (i = num_i2c_clients-1; i >= 0; i--)
       i2c_unregister_device(plat_i2c_client[i]);
}

static int __init plat_magnolia_init(void)
{
    struct i2c_adapter *adap = NULL;
    struct i2c_client *e = NULL;
    int ret = 0;
    int i,j;

    printk("el6661 plat_magnolia_init  \n");

#if 0  //disable for ICOS
    //use i2c-gpio    
    //register i2c gpio
    //config gpio8,9 to gpio function
    outl( inl(0x500) | (1<<8 | 1<<9), 0x500);
    
	ret = platform_device_register(&magnolia_device_i2c_gpio);
	if (ret) {
		printk(KERN_ERR "i2c-gpio: magnolia_device_i2c_gpio register fail %d\n", ret);
	}
#endif
    
    for(i=0; i<ARRAY_SIZE(i2cdev_list); i++) {
        
        adap = i2c_get_adapter( i2cdev_list[i].ch );
        if (adap == NULL) {
            printk("magnolia get channel %d adapter fail\n", i);
            continue;
            return -ENODEV;
        }
    
        i2c_put_adapter(adap);
        for(j=0; j<i2cdev_list[i].size; j++) {
            e = i2c_new_device(adap, &i2cdev_list[i].board_info[j] );

            if (plat_i2c_client_add(e)<0) {
               printk("too many i2c clients added (PLAT_MAX_I2C_CLIENTS=%d)\n", PLAT_MAX_I2C_CLIENTS);
               plat_i2c_client_remove_all();
               return -ENODEV;
            }
        }
    }

    return ret;    
}

static void __exit plat_magnolia_exit(void)
{
    plat_i2c_client_remove_all();
}

module_init(plat_magnolia_init);
module_exit(plat_magnolia_exit);
//arch_initcall(plat_magnolia_init);

MODULE_AUTHOR("Inventec");
MODULE_DESCRIPTION("Magnolia Platform devices");
MODULE_LICENSE("GPL");
