/*****************************
 Sequoia(d7264q28b) platform
******************************/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/string.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/mutex.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/ctype.h>
#include <linux/uaccess.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include "inv_pthread.h"

#define INV_PTHREAD_KERNEL_MODULE

#define SHOW_ATTR_WARNING       ("N/A")
#define SHOW_ATTR_NOTPRINT      ("Not Available")
#define SHOW_ATTR_NOTSUPPORT    ("Not Support")

#define INV_HWMID_MAX           (10)
#define INV_HWMID_INIT		(-1)

/*access userspace data to kernel space*/
#define ACC_R           (0)
#define ACC_W           (1)

#define MAX_PATH_SIZE   (64)
#define MIN_ACC_SIZE    (32)
#define MAX_ACC_SIZE    (256)

/*
 * LED definitions
 */
#define STATUS_LED_MODE_AUTO	(0)
#define STATUS_LED_MODE_DIAG	(1)
#define STATUS_LED_MODE_MANU	(2)
#define STATUS_LED_MODE_ERR	(-1)

#define STATUS_LED_GRN0		(10)	// 0 - 000: off
#define STATUS_LED_GRN1		(11)	// 1 - 001: 0.5hz
#define STATUS_LED_GRN2		(12)	// 2 - 010: 1 hz
#define STATUS_LED_GRN3		(13)	// 3 - 011: 2 hz
#define STATUS_LED_GRN7		(17)	// 7 - 111: on
#define STATUS_LED_RED0		(20)	// 0 - 000: off
#define STATUS_LED_RED1		(21)	// 1 - 001: 0.5hz
#define STATUS_LED_RED2		(22)	// 2 - 010: 1 hz
#define STATUS_LED_RED3		(23)	// 3 - 011: 2 hz
#define STATUS_LED_RED7		(27)	// 7 - 111: on
#define STATUS_LED_INVALID	(0)	// Invalid

static struct mutex pthread_mutex;

ssize_t status_led_grn(const char *freq);
ssize_t status_led_red(const char *freq);
ssize_t status_led_diag_mode_enable(void);
ssize_t status_led_diag_mode_disable(void);
int status_led_check_color(void);
int status_led_check_diag_mode(void);

#if 1
/* For timestamps in SYSFS_LOG */
#define SYSFS_LOG	printk
#else
//#define SYSFS_LOG(fmt, args...) printk(KERN_WARNING "[SYSFS] %s/%d: " fmt, __func__, __LINE__, ##args)
#define SYSFS_LOG(fmt, args...) printk(KERN_WARNING "[p_thread] " fmt, ##args)
#endif


/* inventec_class *********************************/
static struct kobject *status_kobj;
static struct kset *status_kset;

#if 0
int inventec_strtol(const char *sbufp, char **endp, unsigned int base)
{
    char *endptr;
    int value = simple_strtol(sbufp, &endptr, base);
    if (value == 0 && sbufp == endptr) {
	*endp = NULL;
	return value;
    }
    *endp = (char*)1;
    return value;
}
#endif

int inventec_singlechar_to_int(const char c)
{
    if ((c >= '0') && (c <= '9')) {
	return (c - '0');
    }
    else
    if ((c >= 'a') && (c <= 'f')) {
	return (c - 'a' + 10);
    }
    else
    if ((c >= 'A') && (c <= 'F')) {
	return (c - 'A' + 10);
    }
    return -1;
}

/* fan device *************************************/
#define FAN_STATE_NORMAL	"normal"
#define FAN_STATE_FAULTY	"faulty"
#define FAN_STATE_UNINSTALLED	"uninstalled"
#define FAN_STATE_UNKNOW	"unknown state"
#define FAN_STATE_INVALID	"Invalid state value"
#define FAN_STATE_READ_ERROR	"state read error"

#define FAN_LOG_UNINSTALLED	"removed"
#define FAN_LOG_NORMAL		"inserted"

//#define FAN_STATE_BIT_NORMAL		0
#define FAN_STATE_BIT_FAULTY		0
#define FAN_STATE_BIT_UNINSTALLED	1
#define FAN_STATE_BIT_UNKNOW		2
#define FAN_STATE_BIT_INVALID		3
#define FAN_STATE_BIT_READ_ERROR	4

static ssize_t psoc_show_fan1_input(char *buf)
{
	return psoc_show_fan_input(buf, 1);
}

static ssize_t psoc_show_fan2_input(char *buf)
{
	return psoc_show_fan_input(buf, 2);
}

static ssize_t psoc_show_fan3_input(char *buf)
{
	return psoc_show_fan_input(buf, 3);
}

static ssize_t psoc_show_fan4_input(char *buf)
{
	return psoc_show_fan_input(buf, 4);
}

static ssize_t psoc_show_fan5_input(char *buf)
{
	return psoc_show_fan_input(buf, 5);
}

static ssize_t psoc_show_fan6_input(char *buf)
{
	return psoc_show_fan_input(buf, 6);
}

static ssize_t psoc_show_fan7_input(char *buf)
{
	return psoc_show_fan_input(buf, 7);
}

static ssize_t psoc_show_fan8_input(char *buf)
{
	return psoc_show_fan_input(buf, 8);
}

static struct fans_tbl_s {
        char *fan_name;
        ssize_t (*fan_front)(char *);
        ssize_t (*fan_rear)(char *);
        unsigned int fan_state;
} fans_tbl[] = {
        {"fan1",	psoc_show_fan1_input,
			psoc_show_fan2_input,	0},
        {"fan2",	psoc_show_fan3_input,
			psoc_show_fan4_input,	0},
        {"fan3",	psoc_show_fan5_input,
			psoc_show_fan6_input,	0},
        {"fan4",	psoc_show_fan7_input,
			psoc_show_fan8_input,	0},
};
#define FAN_TBL_TOTAL	( sizeof(fans_tbl)/ sizeof(const struct fans_tbl_s) )

#define FAN_STATE_CHECK(i,b)	(fans_tbl[i].fan_state & (1<<b))
#define FAN_STATE_SET(i,b)	(fans_tbl[i].fan_state |= (1<<b))
#define FAN_STATE_CLEAR(i,b)	(fans_tbl[i].fan_state &= ~(1<<b))
#define FAN_STATE_INIT(i)	(fans_tbl[i].fan_state = 0)

static int
fans_faulty_log(int fan_id)
{
    char buf[MAX_ACC_SIZE];
    int pwm;

    memset(&buf[0], 0, MAX_ACC_SIZE);
    if (fans_tbl[fan_id].fan_front(buf) < 0) {
	if(!FAN_STATE_CHECK(fan_id, FAN_STATE_BIT_READ_ERROR)) {
	    FAN_STATE_SET(fan_id, FAN_STATE_BIT_READ_ERROR);
	    SYSFS_LOG("[p_thread] %s: front %s\n",fans_tbl[fan_id].fan_name,FAN_STATE_READ_ERROR);
	}
	return 1;
    }
    pwm = simple_strtol(buf, NULL, 10);
    if (pwm <= 0) {
	if(!FAN_STATE_CHECK(fan_id, FAN_STATE_BIT_FAULTY)) {
	    FAN_STATE_SET(fan_id, FAN_STATE_BIT_FAULTY);
	    //SYSFS_LOG("[p_thread] %s: %s\n",fans_tbl[fan_id].fan_name,FAN_STATE_FAULTY);
	}
	return 1;
    }

    memset(&buf[0], 0, MAX_ACC_SIZE);
    if (fans_tbl[fan_id].fan_rear(buf) < 0) {
	if(!FAN_STATE_CHECK(fan_id, FAN_STATE_BIT_READ_ERROR)) {
	    FAN_STATE_SET(fan_id, FAN_STATE_BIT_READ_ERROR);
	    SYSFS_LOG("[p_thread] %s: rear %s\n",fans_tbl[fan_id].fan_name,FAN_STATE_READ_ERROR);
	}
	return 1;
    }
    pwm = simple_strtol(buf, NULL, 10);
    if (pwm <= 0) {
	if(!FAN_STATE_CHECK(fan_id, FAN_STATE_BIT_FAULTY)) {
	    FAN_STATE_SET(fan_id, FAN_STATE_BIT_FAULTY);
	    //SYSFS_LOG("[p_thread] %s: %s\n",fans_tbl[fan_id].fan_name,FAN_STATE_FAULTY);
	}
	return 1;
    }

    if(fans_tbl[fan_id].fan_state != 0) {
	fans_tbl[fan_id].fan_state = 0;
	SYSFS_LOG("[p_thread] %s: %s\n",fans_tbl[fan_id].fan_name,FAN_LOG_NORMAL);
    }
    return 0;
}

/* INV drivers mapping */
static int
fans_control_log(char *buf, int fan_id)
{
    unsigned int statebit;

    if (buf[0] != '0' || (buf[1] != 'x' && buf[1] != 'X')) {
	SYSFS_LOG("[p_thread] %s/%d: %s %s\n",__func__,__LINE__,FAN_STATE_INVALID, buf);
	return 1;
    }

    if (buf[2] != buf[3]) {
	SYSFS_LOG("[p_thread] %s/%d: %s %s\n",__func__,__LINE__,FAN_STATE_INVALID, buf);
	return 1;
    }

    if ((statebit = inventec_singlechar_to_int(buf[2])) == -1) {
	SYSFS_LOG("[p_thread] Error value read from fan %d\n", fan_id);
	return 1;
    }

    if (statebit & 1<<fan_id) {
	if(!FAN_STATE_CHECK(fan_id, FAN_STATE_BIT_UNINSTALLED)) {
	    FAN_STATE_SET(fan_id, FAN_STATE_BIT_UNINSTALLED);
	    SYSFS_LOG("[p_thread] %s: %s\n",fans_tbl[fan_id].fan_name,FAN_LOG_UNINSTALLED);
	}
	return fans_faulty_log(fan_id);
    }

    return 0;
}

int fans_control(void)
{
    char buf[MAX_ACC_SIZE];
    int i, err_fans = 0;

    if (psoc_show_fan_state(buf) < 0) {
        SYSFS_LOG("[p_thread] read fan_gpi failed\n");
        return FAN_TBL_TOTAL+1;
    }

    for (i = 0; i < FAN_TBL_TOTAL; i++) {
        if(fans_control_log(buf, i) == 1) {
            err_fans++;
        }
    }

    if (!err_fans) {
	return 0;
    }

    status_led_red("3");  //1Hz
    if (0 < err_fans && err_fans < FAN_TBL_TOTAL) {
	printk(KERN_ERR "[p_thread] %d fans failed. [%s]\n", err_fans, buf);
    }
    else
    if (err_fans == FAN_TBL_TOTAL) {
        printk(KERN_ERR "[p_thread] All fans failed. [%s]\n", buf);
    }
    return err_fans;
}

/* End of faninfo_device */

/* psu device *************************************/
static unsigned int psu_voltin = 0;
#define PSU_VOLTIN_ACDC	(70000)

/*
 * normal/unpower/uninstall/fault are PSU states output from driver level
 * checkpsu/error are defined by sysfs
 */
#define PSU_STATE_VAL_NORMAL	(0)
#define PSU_STATE_VAL_UNPOWER	(2)
#define PSU_STATE_VAL_FAULT	(4)
#define PSU_STATE_VAL_UNINSTALL	(7)
#define PSU_STATE_VAL_CHECKPSU	(8)
#define PSU_STATE_VAL_ERROR	(9)

#define PSU_STATE_NORMAL	("0 : normal")
#define PSU_STATE_UNPOWER	("2 : unpowered")
#define PSU_STATE_FAULT		("4 : fault")
#define PSU_STATE_UNINSTALL	("7 : not installed")
#define PSU_STATE_CHECKPSU	("8 : check psu")
#define PSU_STATE_ERROR		("9 : state error")

#define PSU_STATE_LEN_NORMAL	(strlen(PSU_STATE_NORMAL))
#define PSU_STATE_LEN_UNPOWER	(strlen(PSU_STATE_UNPOWER))
#define PSU_STATE_LEN_FAULT	(strlen(PSU_STATE_FAULT))
#define PSU_STATE_LEN_UNINSTALL	(strlen(PSU_STATE_UNINSTALL))
#define PSU_STATE_LEN_CHECKPSU	(strlen(PSU_STATE_CHECKPSU))

typedef struct {
	char *inv_dev_attrp;
} psu_dev_t;

typedef struct {
	const char		*psu_name;
	int			psu_major;
        dev_t			psu_devt;
	struct device		*psu_dev_p;
	psu_dev_t		*psu_dev_namep;
	int			psu_dev_total;
	char			*psu_inv_pathp;
	void			*psu_tracking;
	char			*psu_currentin;
	char			*psu_currentout;
	char			*psu_powerin;
	char			*psu_powerout;
	char			*psu_voltin;
	char			*psu_voltout;
} psu_dev_group_t;

static psu_dev_t psu_dev_name[] = {
	{ "state" },	// Using cpld
	{ "psu_voltin" },
};
#define PSU_DEV_NAME_TOTAL	( sizeof(psu_dev_name) / sizeof(const psu_dev_t) )

static psu_dev_group_t psu_dev_group[] = {
	{
	  .psu_name = "psu1",
	  .psu_dev_namep = &psu_dev_name[0],
	  .psu_dev_total = sizeof(psu_dev_name) / sizeof(const psu_dev_t),
	},
	{
	  .psu_name = "psu2",
	  .psu_dev_namep = &psu_dev_name[0],
	  .psu_dev_total = sizeof(psu_dev_name) / sizeof(const psu_dev_t),
	},
};
#define PSU_DEV_GROUP_TOTAL	( sizeof(psu_dev_group)/ sizeof(const psu_dev_group_t) )

static char psu_state[4][MIN_ACC_SIZE];

static ssize_t psoc_show_psu0_state(char *buf)
{
        return psoc_show_psu_state(buf, 0);
}

static ssize_t psoc_show_psu1_state(char *buf)
{
        return psoc_show_psu_state(buf, 1);
}

static ssize_t psoc_show_psu1_vin(char *buf)
{
        return psoc_show_psu_vin(buf, 1);
}

static ssize_t psoc_show_psu2_vin(char *buf)
{
        return psoc_show_psu_vin(buf, 2);
}

static struct psu_wire_tbl_s {
        char *psu_attr;
        char *psu_name;
	ssize_t (*psu_wire)(char *);
	char *psu_state;
} psu_wire_tbl[] = {
        { "state",		"psu1",	psoc_show_psu0_state,	psu_state[0] },	// Using cpld
        { "state",		"psu2",	psoc_show_psu1_state,	psu_state[1] },
	{ "psu_voltin",		"psu1",	psoc_show_psu1_vin,	psu_state[2] },
	{ "psu_voltin",		"psu2",	psoc_show_psu2_vin,	psu_state[3] },
};
#define PSU_WIRE_TBL_TOTAL   ( sizeof(psu_wire_tbl)/ sizeof(const struct psu_wire_tbl_s) )

static ssize_t
(*psu_get_show_function(const char *psu_attrp, const char *psu_namep, char **psu_statepp)) (char *buf)
{
    int i;

    for (i = 0; i < PSU_WIRE_TBL_TOTAL; i++) {
	if (strncmp(psu_wire_tbl[i].psu_attr, psu_attrp, strlen(psu_attrp)) == 0 &&
	    strncmp(psu_wire_tbl[i].psu_name, psu_namep, strlen(psu_namep)) == 0) {
	    if (psu_statepp) {
		*psu_statepp = psu_wire_tbl[i].psu_state;
	    }
	    return psu_wire_tbl[i].psu_wire;
	}
    }
    return NULL;
}

int psu_check_state_normal(char *statep)
{
    if (strstr(statep, "normal")) {
	return 1;
    }
    return 0;
}

#define PSU_ATTR_VOLTIN		("psu_voltin")
#define PSU_ATTR_VOLTIN_LEN	(10)

/* Get PSU voltin for determon AC(110v) or DC(48v) */
void psu_get_voltin(void)
{
    char volt[MIN_ACC_SIZE];
    psu_dev_t *devnamep;
    unsigned int voltin;
    ssize_t (*invwirep)(char *buf) = NULL;
    int i, j;

    for (i = 0; i < PSU_DEV_GROUP_TOTAL; i++) {
	//psu_dev_group[i].psu_name;
	devnamep = psu_dev_group[i].psu_dev_namep;
	for (j = 0; j < psu_dev_group[i].psu_dev_total; j++, devnamep++) {
	    if (strncmp(devnamep->inv_dev_attrp, PSU_ATTR_VOLTIN, PSU_ATTR_VOLTIN_LEN) == 0) {
		invwirep = psu_get_show_function(PSU_ATTR_VOLTIN, psu_dev_group[i].psu_name, NULL);
		if (invwirep == NULL) {
		    printk(KERN_DEBUG "[p_thread] Invalid psuname: %s\n", psu_dev_group[i].psu_name);
		    continue;
		}
		if (invwirep(volt) <= 0) {
		    printk(KERN_DEBUG "[p_thread] Read %s failed\n", psu_dev_group[i].psu_name);
		    continue;
		}
		else {
		    voltin = simple_strtol(&volt[0], NULL, 10);
		    printk(KERN_DEBUG "[p_thread] Read %s %s = %u\n",psu_dev_group[i].psu_name,volt,voltin);
		    if (voltin > psu_voltin) {
			psu_voltin = voltin;
		    }
		}
	    }
	}
    }

    SYSFS_LOG("[p_thread] PSU voltin = %u\n", psu_voltin);
}

#define PSU_ATTR_STATE		("state")
#define PSU_ATTR_STATE_LEN	(5)

/* psus_control() by inv_thread */
int psus_control(int log_only)
{
    char state[MIN_ACC_SIZE];
    psu_dev_t *devnamep = NULL;
    ssize_t (*invwirep)(char *buf) = NULL;
    char *psu_statep = NULL;
    int i, j, flag = 0;

    for (i = 0; i < PSU_DEV_GROUP_TOTAL; i++) {
	devnamep = psu_dev_group[i].psu_dev_namep;
	for (j = 0; j < psu_dev_group[i].psu_dev_total; j++, devnamep++) {
	    if (strncmp(devnamep->inv_dev_attrp, PSU_ATTR_STATE, PSU_ATTR_STATE_LEN) == 0) {
		invwirep = psu_get_show_function(PSU_ATTR_STATE, psu_dev_group[i].psu_name, &psu_statep);
		if (invwirep == NULL) {
		    printk(KERN_DEBUG "[p_thread] Invalid psuname: %s\n", psu_dev_group[i].psu_name);
		    continue;
		}
		if (invwirep(state) <= 0) {
		    printk(KERN_DEBUG "[p_thread] Read %s failed\n", psu_dev_group[i].psu_name);
		    if (strncmp(psu_statep, PSU_STATE_ERROR, strlen(PSU_STATE_ERROR)) != 0) {
			strcpy(psu_statep, PSU_STATE_ERROR);
			SYSFS_LOG("[p_thread] %s: %s\n",psu_dev_group[i].psu_name,PSU_STATE_ERROR);
		    }
		    flag = 1;
		}
		else
		if (strstr(state, "normal")) {
		    if (strncmp(psu_statep, state, strlen(state)) != 0) {
			strcpy(psu_statep, state);
			SYSFS_LOG("[p_thread] %s: %s\n",psu_dev_group[i].psu_name,state);
		    }
		}
		else
		if (psu_voltin > PSU_VOLTIN_ACDC) {	/* AC PSUS */
		    if (strncmp(psu_statep, state, strlen(state)) != 0) {
			strcpy(psu_statep, state);
			SYSFS_LOG("[p_thread] %s: %s\n",psu_dev_group[i].psu_name,state);
		    }
		    flag = 1;
		}
		else {					/* DC PSUS */
		    if (strncmp(psu_statep, PSU_STATE_CHECKPSU, PSU_STATE_LEN_CHECKPSU) != 0) {
			strcpy(psu_statep, PSU_STATE_CHECKPSU);
			SYSFS_LOG("[p_thread] %s: %s\n",psu_dev_group[i].psu_name,PSU_STATE_CHECKPSU);
		    }
		    flag = 1;
		}
	    }
	}
    }

    if (log_only) {
	return 0;
    }

    //SYSFS_LOG("[p_thread] RYU: %s: flag = %d\n",psu_wire_tbl[i].psu_name,flag);
    if (flag == 1) {
	status_led_grn("2");
	return 1;
    }
    return 0;
}

/* End of psuinfo_device */

/* led device *************************************/

/* return 0/off 1/green 2/red */
int
status_led_check_color(void)
{
    char tmpbuf[MIN_ACC_SIZE];
    int ret = STATUS_LED_INVALID;

    if (cpld_show_led(&tmpbuf[0], CPLD_DEV_LED_GRN_INDEX) > 0) {
	if (tmpbuf[0] == '0') {
	    ret = STATUS_LED_GRN0;
	}
	if (tmpbuf[0] == '1') {
	    ret = STATUS_LED_GRN1;
	}
	if (tmpbuf[0] == '2') {
	    ret = STATUS_LED_GRN2;
	}
	if (tmpbuf[0] == '3') {
	    ret = STATUS_LED_GRN3;
	}
	if (tmpbuf[0] == '7') {
	    ret = STATUS_LED_GRN7;
	}
        return ret;
    }

    if (cpld_show_led(&tmpbuf[0], CPLD_DEV_LED_RED_INDEX) > 0) {
	if (tmpbuf[0] == '0') {
	    ret = STATUS_LED_RED0;
	}
	if (tmpbuf[0] == '1') {
	    ret = STATUS_LED_RED1;
	}
	if (tmpbuf[0] == '2') {
	    ret = STATUS_LED_RED2;
	}
	if (tmpbuf[0] == '3') {
	    ret = STATUS_LED_RED3;
	}
	if (tmpbuf[0] == '7') {
	    ret = STATUS_LED_RED7;
	}
	return ret;
    }
    return ret;
}

/*
 * Store attr Section
 */
ssize_t status_led_diag_mode_enable(void)
{
    char tmp[MIN_ACC_SIZE];
    ssize_t ret;

    ret = psoc_show_diag(&tmp[0]);
    if (ret <= 0) {
        return ret;
    }

    if (tmp[0] == '0') {
	ret = psoc_set_diag("1", 1);
        if (ret < 0) {
            return ret;
        }

	ret = cpld_set_ctl("1", 1);
        if (ret < 0) {
            return ret;
        }
    }

    return ret;
}

ssize_t status_led_diag_mode_disable(void)
{
    char tmp[MIN_ACC_SIZE];
    ssize_t ret;

    ret = psoc_show_diag(&tmp[0]);
    if (ret <= 0) {
        return ret;
    }

    if (tmp[0] == '1') {
	ret = psoc_set_diag("0", 1);
        if (ret < 0) {
            return 1;
        }

	ret = cpld_set_ctl("1", 1);
        if (ret < 0) {
            return 1;
        }
    }
    return 1;
}

ssize_t
status_led_red(const char *freq)
{
    char buf[MIN_ACC_SIZE];
    ssize_t ret;

    ret = cpld_show_led(buf, CPLD_DEV_LED_RED_INDEX);
    if (ret < 0 || buf[0] == freq[0]) {
        return ret;
    }

    ret = cpld_set_led("0", 1, CPLD_DEV_LED_GRN_INDEX);
    if (ret < 0) {
        return ret;
    }
    ret = cpld_set_led(freq, strlen(freq), CPLD_DEV_LED_RED_INDEX);
    if (ret < 0) {
        return ret;
    }
    if ((ret = status_led_diag_mode_enable()) <= 0) {
        return ret;
    }
    ssleep(1);
    if ((ret = status_led_diag_mode_disable()) <= 0) {
        return ret;
    }
    return ret;
}

ssize_t
status_led_grn(const char *freq)
{
    char buf[MIN_ACC_SIZE];
    ssize_t ret;

    ret = cpld_show_led(buf, CPLD_DEV_LED_GRN_INDEX);
    if (ret < 0 || buf[0] == freq[0]) {
	return ret;
    }

    ret = cpld_set_led("0", 1, CPLD_DEV_LED_RED_INDEX);
    if (ret < 0) {
        return ret;
    }
    ssleep(1);
    ret = cpld_set_led(freq, strlen(freq), CPLD_DEV_LED_GRN_INDEX);
    if (ret < 0) {
        return ret;
    }
    if ((ret = status_led_diag_mode_enable()) <= 0) {
        return ret;
    }
    ssleep(1);
    if ((ret = status_led_diag_mode_disable()) <= 0) {
        return ret;
    }
    return ret;
}

int status_led_check_diag_mode(void)
{
    char tmp[MIN_ACC_SIZE];
    ssize_t ret;

    ret = psoc_show_diag(&tmp[0]);
    if (ret <= 0) {
        return ret;
    }

    if (tmp[0] == '1') {
	return STATUS_LED_MODE_DIAG;
    }

    if (tmp[0] == '0') {
	return STATUS_LED_MODE_AUTO;
    }

    return -1;
}

/* End of ledinfo_device */

/**************************************************/
/* From system_device */
static int inv_pthread_control = 1;

int thread_control(void)
{
    return inv_pthread_control;
}

void thread_control_set(int val)
{
    inv_pthread_control = val;
}
/* End system_device */

#define THREAD_SLEEP_MINS	(3)
#define THREAD_DELAY_MINS	(THREAD_SLEEP_MINS + THREAD_SLEEP_MINS + 1)

static struct task_struct *thread_st;
static int thread_data;

// Function executed by kernel thread
static int thread_fn(void *unused)
{
    mutex_init(&pthread_mutex);

    ssleep(THREAD_DELAY_MINS);

    /* Default status init */
    mutex_lock(&pthread_mutex);
    status_led_grn("7");
    mutex_unlock(&pthread_mutex);

    psu_get_voltin();

    /* Delay for guarantee HW ready */
    ssleep(THREAD_DELAY_MINS);

    while (1)
    {
	mutex_unlock(&pthread_mutex);
	ssleep(THREAD_SLEEP_MINS);

	if (thread_control() == 0) {
	    printk(KERN_INFO "[p_thread] %s/%d: Thread Stop by inv_pthread control\n",__func__,__LINE__);
	    break;
	}

	mutex_lock(&pthread_mutex);
	if (status_led_check_diag_mode() == STATUS_LED_MODE_MANU) {
            /* status led in change color/freq mode, higher priority. Ignore fans sttaus */
            continue;
	}

#if 0
	switch_temp_update();
#endif

	if (fans_control() > 0) {
	    psus_control(1);
	    continue;
	}
	else
	if (psus_control(0) > 0) {
	    continue;
	}

	if (status_led_check_color() != STATUS_LED_GRN7) {      /* status led red, change it to green */
	    status_led_grn("7");
	}
    }

    do_exit(0);
    printk(KERN_INFO "[p_thread] %s/%d: Thread Stopped\n",__func__,__LINE__);
    return 0;
}


static ssize_t s_show(struct kobject *kobj, struct attribute *attr, char *buf)
{
    int fan_absence;
    size_t count = 0;

    fan_absence = fans_control();
    count += sprintf(&buf[count], "%d\n", fan_absence);
    return count;
}

static ssize_t s_store(struct kobject *kobj, struct attribute *attr, const char *buf, size_t count)
{
    return count;
}

static struct attribute status_att = {
    .name = "fan_absence",
    .mode = 0777,
};

static const struct sysfs_ops status_ops = {
    .show = s_show,
    .store = s_store,
};

static struct kobj_type status_ktype = {
    .sysfs_ops = &status_ops,
};


static int __init inv_pthread_init(void)
{
    int retval;

    status_kobj = kzalloc(sizeof(*status_kobj), GFP_KERNEL);
    if(!status_kobj)
        return PTR_ERR(status_kobj);

    status_kset = kset_create_and_add("platform_status", NULL, kernel_kobj);
    if(!status_kset)
        return -1;

    status_kobj->kset = status_kset;

    retval = kobject_init_and_add(status_kobj, &status_ktype, NULL, "fan");
    if(retval)
        return retval;

    retval = sysfs_create_file(status_kobj, &status_att);

    thread_control_set(1);

    printk(KERN_INFO "[p_thread] %s/%d: Creating Thread\n",__func__,__LINE__);
    //Create the kernel thread with name 'inv_pthread'
    thread_st = kthread_run(thread_fn, (void*)&thread_data, "inv_pthread");
    if (thread_st)
        printk(KERN_INFO "[p_thread] inv_pthread Created successfully\n");
    else
        printk(KERN_ERR "[p_thread] inv_pthread creation failed\n");

    return retval;
}

static void __exit inv_pthread_exit(void)
{
    thread_control_set(0);
    /* Delay for guarantee thread exit */
    ssleep(THREAD_DELAY_MINS);

    sysfs_remove_file(status_kobj, &status_att);
    kset_unregister(status_kset);
    kobject_del(status_kobj);

    printk(KERN_INFO "[p_thread] inv_pthread cleaning Up\n");
}

module_init(inv_pthread_init);
module_exit(inv_pthread_exit);

MODULE_AUTHOR("Robert <yu.robertxk@inventec.com>");
MODULE_DESCRIPTION("Inventec Platform Management Thread");
MODULE_VERSION("version 1.1");
MODULE_LICENSE("GPL");
