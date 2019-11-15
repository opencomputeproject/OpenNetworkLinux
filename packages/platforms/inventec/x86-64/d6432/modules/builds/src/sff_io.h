//port info
#ifndef __SFF_IO_H
#define __SFF_IO_H


#define TEST_CODE
#define RETRY_COUNT (5)
#define RETRY_DELAY_MS (100)

#define IO_RESET_DEFAULT (1)
#define IO_LPMODE_DEFAULT (1)
#define IO_MODESEL_DEFAULT (0)
#define IO_TXDISABLE_DEFAULT (1)

typedef enum
{
   IO_PRESENT = 0,
   IO_RX_LOS,
   IO_TX_FAULT,
   IO_INTL,
   IO_RESET,
   IO_LPMODE,
   IO_MODE_SEL,
   IO_TXDISABLE,
   IO_IDX_NUM,

}io_config_table_idx;

typedef enum
{
    PCA9555_TYPE,
    PCA9554_TYPE,
    PCA95XX_TYPE_NUM,

}pca95xx_type_t;

struct ioexp_data_t {
    u8 ch;
    u8 addr;
    int port_min;
    int bit_min;
    int bit_max;  /*reprsent the num of ports*/
};
struct ioexp_config_t {
    u8 ch;
    u8 addr;
    u16 val;
};

struct input_change_table_t {
    int *table;
    int size;
};
struct pca95xx_type_t {
    u8 ch;
    u8 addr;
    int type;
};

struct ioexp_func_t {
    struct ioexp_data_t *map;
    int *ioPort_2_port;
    int *port_2_ioPort;
    //struct ioexp_port_2_port_t *io_p_2_p;
};
struct ioexp_func_map_t {
    int idx;
    struct ioexp_func_t *func;
    //struct ioexp_port_2_port_t *io_p_2_p;
};
struct cpld_io_info_t {
    int ch;
    u8  addr;
    u8  io_int_status_reg;
    u8  io_int_enable_reg;
    int int_gpio;
};
/*exported functions*/
int sff_io_prsL_get(int port, u8 *prsL);
int sff_io_prsL_all_get(unsigned long *bitmap);
int sff_io_reset_set(int port, u8 reset);
int sff_io_reset_get(int port, u8 *reset);
int sff_io_lpmode_set(int port, u8 value);
int sff_io_lpmode_get(int port, u8 *value);
int sff_io_tx_disable_set(int port, u8 value);
int sff_io_tx_disable_get(int port, u8 *value);
int sff_io_mode_sel_set(int port, u8 value);
int sff_io_mode_sel_get(int port, u8 *value);
int sff_io_intL_get(int port, u8 *value);
int sff_io_rx_los_get(int port, u8 *value);
int sff_io_tx_fault_get(int port, u8 *value);

int sff_io_lpmode_set(int port, u8 lpmode);
int sff_io_reset_set(int port, u8 reset);
int sff_io_mux_reset_all(int value);
int sff_io_init(void);
void sff_io_deinit(void);
int ioexp_input_handler(void);
bool ioexp_is_channel_ready(void);
int ioexp_health_monitor(void);
bool ioexp_is_i2c_ready(void);
int sff_io_mux_reset_get(void);
bool ioexp_scan_ready(void);
#endif /*__SFF_IO_H*/
