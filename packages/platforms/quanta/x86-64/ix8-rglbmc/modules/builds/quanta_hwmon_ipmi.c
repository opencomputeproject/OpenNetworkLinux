/*
*
* A hwmon driver for the Quanta switch BMC hwmon
*
*/

#include <linux/err.h>
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>
#include <linux/ipmi.h>
#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/completion.h>

#define enable_debug_msg	0
#define DEBUGUSE_SHIFT		0

#define DRVNAME		"quanta_hwmon_ipmi"
#define CLASSNAME		"quanta"
#define CLASS_SYSNAME	"sys_control"

#define tos32(val, bits)    ((val & ((1<<((bits)-1)))) ? (-((val) & (1<<((bits)-1))) | (val)) : (val))
#define BSWAP_16(x)			((((x) & 0xff00) >> 8) | (((x) & 0x00ff) << 8))
#define BSWAP_32(x)			((((x) & 0xff000000) >> 24) | (((x) & 0x00ff0000) >> 8) | (((x) & 0x0000ff00) << 8) | (((x) & 0x000000ff) << 24))
#define __TO_M(mtol)		(int16_t)(tos32((((BSWAP_16(mtol) & 0xff00) >> 8) | ((BSWAP_16(mtol) & 0xc0) << 2)), 10))
#define __TO_B(bacc)		(int32_t)(tos32((((BSWAP_32(bacc) & 0xff000000) >> 24) | ((BSWAP_32(bacc) & 0xc00000) >> 14)), 10))
#define __TO_R_EXP(bacc)	(int32_t)(tos32(((BSWAP_32(bacc) & 0xf0) >> 4), 4))
#define __TO_B_EXP(bacc)	(int32_t)(tos32((BSWAP_32(bacc) & 0xf), 4))

#define SENSOR_ATTR_MAX			16
#define SENSOR_ATTR_NAME_LENGTH	20

#define SENSOR_GET_CAP_LABEL	0x001
#define SENSOR_GET_CAP_ALARM	0x002
#define SENSOR_GET_CAP_INPUT	0x004

#define SENSOR_GET_CAP_LNC		0x008
#define SENSOR_GET_CAP_LCR		0x010
#define SENSOR_GET_CAP_LNR		0x020

#define SENSOR_GET_CAP_UNC		0x040
#define SENSOR_GET_CAP_UCR		0x080
#define SENSOR_GET_CAP_UNR		0x100

#define SENSOR_GET_CAP_MODEL	0x200
#define SENSOR_GET_CAP_SN		0x400
#define SENSOR_GET_CAP_PWM		0x800

#define SENSOR_GET_CAP_CONMODE		0x1000
#define SENSOR_GET_CAP_DIRECTION	0x2000
#define SENSOR_GET_CAP_FAN_PRESENT	0x4000
#define SENSOR_GET_CAP_PSU_PRESENT	0x8000

#define SDR_SENSOR_TYPE_TEMP	0x01
#define SDR_SENSOR_TYPE_VOLT	0x02
#define SDR_SENSOR_TYPE_CURR	0x03
#define SDR_SENSOR_TYPE_FAN		0x04
#define SDR_SENSOR_TYPE_PS		0x08
#define SDR_SENSOR_TYPE_OTHER	0x0b

#define BMC_GET_DEVICE_ID		0x01

#define IPMI_NETFN_SE			0x04
#define IPMI_NETFN_APP			0x06
#define IPMI_NETFN_STORAGE		0x0a
#define IPMI_NETFN_TSOL			0x30

#define GET_SDR_REPO_INFO		0x20
#define GET_DEVICE_SDR			0x21
#define GET_SDR_RESERVE_REPO	0x22
#define GET_SDR					0x23
#define GET_SENSOR_THRESHOLDS	0x27
#define GET_SENSOR_EVENT_ENABLE 0x29
#define GET_SENSOR_EVENT_STATUS	0x2b
#define GET_SENSOR_READING		0x2d
#define GET_PSU_READING			0x52
#define GET_FAN_INFO			0xd6
#define GET_FRU_INFO			0x11

#define IPM_DEV_DEVICE_ID_SDR_MASK		(0x80)	/* 1 = provides SDRs      */
#define IPMI_TIMEOUT			(4 * HZ)

struct quanta_hwmon_ipmi_data {
	struct platform_device	*ipmi_platform_dev;
	struct device			*ipmi_hwmon_dev;
	//struct mutex			ipmi_lock;

	int32_t					total_sensor_id;
	int32_t					total_suport_sensor;
	int32_t					total_create_sysfs;
} *data;

struct class *quanta_class = NULL;
struct quanta_sys_data {};
struct device *quanta_drv_dev = NULL;
struct quanta_sys_data *quanta_drv_data = NULL;

static struct mutex ipmi_lock;
static struct completion g_read_complete;

static ipmi_user_t ipmi_mh_user = NULL;

static int8_t g_fan_control_mode = 3;
static int32_t g_ipmi_sync_flag = 0;
static int32_t g_use_built_in = 0;

static bool g_bmc_alive_flag = true;

struct ipmi_sensor_data {
	uint8_t addr;
	uint8_t sensor_type;
	uint8_t sensor_idstring[SENSOR_ATTR_NAME_LENGTH];

	uint16_t capability;

	struct header_info {
		uint8_t header_type;
		uint8_t header_byte;
	} headerinfo;

	struct record_info {
		uint8_t record_analog;
		uint8_t record_linearization;

		int32_t record_m;
		int32_t record_b;
		int32_t record_k1;
		int32_t record_k2;
	} recordinfo;

	struct threshold_upper_info {
		uint8_t unr;
		uint8_t ucr;
		uint8_t unc;
	} upperinfo;

	struct threshold_lower_info {
		uint8_t lnr;
		uint8_t lcr;
		uint8_t lnc;
	} lowerinfo;

	struct attr_info
	{
		bool attr_exist;
		char attr_name[SENSOR_ATTR_MAX][SENSOR_ATTR_NAME_LENGTH];
		char attr_type_str[SENSOR_ATTR_NAME_LENGTH];

		struct attribute *attrs[SENSOR_ATTR_MAX + 1];
		struct attribute_group	attr_group;
		struct sensor_device_attribute sd_attrs[SENSOR_ATTR_MAX + 1];
	} attrinfo;

} *g_sensor_data;

struct ipmi_sdr_iterator {
	uint16_t reservation;
	int32_t total;
	int32_t next;
};

struct ipm_devid_rsp {
	uint8_t device_id;
	uint8_t device_revision;
	uint8_t fw_rev1;
	uint8_t fw_rev2;
	uint8_t ipmi_version;
	uint8_t adtl_device_support;
	uint8_t manufacturer_id[3];
	uint8_t product_id[2];
	uint8_t aux_fw_rev[4];
} __attribute__((packed));

struct sdr_repo_info_rs {
	uint8_t version;	/* SDR version (51h) */
	uint16_t count;		/* number of records */
	uint16_t free;		/* free space in SDR */
	uint32_t add_stamp;	/* last add timestamp */
	uint32_t erase_stamp;	/* last del timestamp */
	uint8_t op_support;	/* supported operations */
} __attribute__((packed));

struct sdr_device_info_rs {
	uint8_t count;	/* number of records */
	uint8_t flags;	/* flags */
	uint8_t popChangeInd[3];	/* free space in SDR */
} __attribute__((packed));

struct sdr_get_rs {
	uint16_t next;		/* next record id */
	uint16_t id;		/* record ID */
	uint8_t version;	/* SDR version (51h) */
#define SDR_RECORD_TYPE_FULL_SENSOR			0x01
#define SDR_RECORD_TYPE_COMPACT_SENSOR		0x02
#define SDR_RECORD_TYPE_EVENTONLY_SENSOR	0x03
#define SDR_RECORD_TYPE_ENTITY_ASSOC		0x08
#define SDR_RECORD_TYPE_DEVICE_ENTITY_ASSOC	0x09
#define SDR_RECORD_TYPE_GENERIC_DEVICE_LOCATOR	0x10
#define SDR_RECORD_TYPE_FRU_DEVICE_LOCATOR	0x11
#define SDR_RECORD_TYPE_MC_DEVICE_LOCATOR	0x12
#define SDR_RECORD_TYPE_MC_CONFIRMATION		0x13
#define SDR_RECORD_TYPE_BMC_MSG_CHANNEL_INFO	0x14
#define SDR_RECORD_TYPE_OEM			0xc0
	uint8_t type;		/* record type */
	uint8_t length;		/* remaining record bytes */
} __attribute__((packed));

struct sdr_get_rq {
	uint16_t reserve_id;	/* reservation ID */
	uint16_t id;		/* record ID */
	uint8_t offset;		/* offset into SDR */
#define GET_SDR_ENTIRE_RECORD	0xff
	uint8_t length;		/* length to read */
} __attribute__((packed));

struct entity_id {
	uint8_t	id;			/* physical entity id */
#ifdef WORDS_BIGENDIAN
	uint8_t	logical : 1;	/* physical/logical */
	uint8_t	instance : 7;	/* instance number */
#else
	uint8_t	instance : 7;	/* instance number */
	uint8_t	logical : 1;	/* physical/logical */
#endif
} __attribute__((packed));

struct sdr_record_mask {
	union {
		struct {
			uint16_t assert_event;	/* assertion event mask */
			uint16_t deassert_event;	/* de-assertion event mask */
			uint16_t read;	/* discrete reading mask */
		} discrete;
		struct {
#ifdef WORDS_BIGENDIAN
			uint16_t reserved : 1;
			uint16_t status_lnr : 1;
			uint16_t status_lcr : 1;
			uint16_t status_lnc : 1;
			uint16_t assert_unr_high : 1;
			uint16_t assert_unr_low : 1;
			uint16_t assert_ucr_high : 1;
			uint16_t assert_ucr_low : 1;
			uint16_t assert_unc_high : 1;
			uint16_t assert_unc_low : 1;
			uint16_t assert_lnr_high : 1;
			uint16_t assert_lnr_low : 1;
			uint16_t assert_lcr_high : 1;
			uint16_t assert_lcr_low : 1;
			uint16_t assert_lnc_high : 1;
			uint16_t assert_lnc_low : 1;
#else
			uint16_t assert_lnc_low : 1;
			uint16_t assert_lnc_high : 1;
			uint16_t assert_lcr_low : 1;
			uint16_t assert_lcr_high : 1;
			uint16_t assert_lnr_low : 1;
			uint16_t assert_lnr_high : 1;
			uint16_t assert_unc_low : 1;
			uint16_t assert_unc_high : 1;
			uint16_t assert_ucr_low : 1;
			uint16_t assert_ucr_high : 1;
			uint16_t assert_unr_low : 1;
			uint16_t assert_unr_high : 1;
			uint16_t status_lnc : 1;
			uint16_t status_lcr : 1;
			uint16_t status_lnr : 1;
			uint16_t reserved : 1;
#endif
#ifdef WORDS_BIGENDIAN
			uint16_t reserved_2 : 1;
			uint16_t status_unr : 1;
			uint16_t status_ucr : 1;
			uint16_t status_unc : 1;
			uint16_t deassert_unr_high : 1;
			uint16_t deassert_unr_low : 1;
			uint16_t deassert_ucr_high : 1;
			uint16_t deassert_ucr_low : 1;
			uint16_t deassert_unc_high : 1;
			uint16_t deassert_unc_low : 1;
			uint16_t deassert_lnr_high : 1;
			uint16_t deassert_lnr_low : 1;
			uint16_t deassert_lcr_high : 1;
			uint16_t deassert_lcr_low : 1;
			uint16_t deassert_lnc_high : 1;
			uint16_t deassert_lnc_low : 1;
#else
			uint16_t deassert_lnc_low : 1;
			uint16_t deassert_lnc_high : 1;
			uint16_t deassert_lcr_low : 1;
			uint16_t deassert_lcr_high : 1;
			uint16_t deassert_lnr_low : 1;
			uint16_t deassert_lnr_high : 1;
			uint16_t deassert_unc_low : 1;
			uint16_t deassert_unc_high : 1;
			uint16_t deassert_ucr_low : 1;
			uint16_t deassert_ucr_high : 1;
			uint16_t deassert_unr_low : 1;
			uint16_t deassert_unr_high : 1;
			uint16_t status_unc : 1;
			uint16_t status_ucr : 1;
			uint16_t status_unr : 1;
			uint16_t reserved_2 : 1;
#endif
			union {
				struct {
#ifdef WORDS_BIGENDIAN			/* settable threshold mask */
					uint16_t reserved : 2;
					uint16_t unr : 1;
					uint16_t ucr : 1;
					uint16_t unc : 1;
					uint16_t lnr : 1;
					uint16_t lcr : 1;
					uint16_t lnc : 1;
					/* padding lower 8 bits */
					uint16_t readable : 8;
#else
					uint16_t readable : 8;
					uint16_t lnc : 1;
					uint16_t lcr : 1;
					uint16_t lnr : 1;
					uint16_t unc : 1;
					uint16_t ucr : 1;
					uint16_t unr : 1;
					uint16_t reserved : 2;
#endif
				} set;
				struct {
#ifdef WORDS_BIGENDIAN			/* readable threshold mask */
					/* padding upper 8 bits */
					uint16_t settable : 8;
					uint16_t reserved : 2;
					uint16_t unr : 1;
					uint16_t ucr : 1;
					uint16_t unc : 1;
					uint16_t lnr : 1;
					uint16_t lcr : 1;
					uint16_t lnc : 1;
#else
					uint16_t lnc : 1;
					uint16_t lcr : 1;
					uint16_t lnr : 1;
					uint16_t unc : 1;
					uint16_t ucr : 1;
					uint16_t unr : 1;
					uint16_t reserved : 2;
					uint16_t settable : 8;
#endif
				} read;
			};
		} threshold;
	} type;
} __attribute__((packed));

struct sdr_record_full_sensor {
	struct {
		uint8_t owner_id;
#ifdef WORDS_BIGENDIAN
		uint8_t channel : 4;	/* channel number */
		uint8_t __reserved : 2;
		uint8_t lun : 2;	/* sensor owner lun */
#else
		uint8_t lun : 2;	/* sensor owner lun */
		uint8_t __reserved : 2;
		uint8_t channel : 4;	/* channel number */
#endif
		uint8_t sensor_num;	/* unique sensor number */
	} keys;

	struct entity_id entity;

	struct {
		struct {
#ifdef WORDS_BIGENDIAN
			uint8_t __reserved : 1;
			uint8_t scanning : 1;
			uint8_t events : 1;
			uint8_t thresholds : 1;
			uint8_t hysteresis : 1;
			uint8_t type : 1;
			uint8_t event_gen : 1;
			uint8_t sensor_scan : 1;
#else
			uint8_t sensor_scan : 1;
			uint8_t event_gen : 1;
			uint8_t type : 1;
			uint8_t hysteresis : 1;
			uint8_t thresholds : 1;
			uint8_t events : 1;
			uint8_t scanning : 1;
			uint8_t __reserved : 1;
#endif
		} init;
		struct {
#ifdef WORDS_BIGENDIAN
			uint8_t ignore : 1;
			uint8_t rearm : 1;
			uint8_t hysteresis : 2;
			uint8_t threshold : 2;
			uint8_t event_msg : 2;
#else
			uint8_t event_msg : 2;
			uint8_t threshold : 2;
			uint8_t hysteresis : 2;
			uint8_t rearm : 1;
			uint8_t ignore : 1;
#endif
		} capabilities;
		uint8_t type;
	} sensor;

	uint8_t event_type;	/* event/reading type code */

	struct sdr_record_mask mask;

	struct {
#ifdef WORDS_BIGENDIAN
		uint8_t analog : 2;
		uint8_t rate : 3;
		uint8_t modifier : 2;
		uint8_t pct : 1;
#else
		uint8_t pct : 1;
		uint8_t modifier : 2;
		uint8_t rate : 3;
		uint8_t analog : 2;
#endif
		struct {
			uint8_t base;
			uint8_t modifier;
		} type;
	} unit;

#define SDR_SENSOR_L_LINEAR     0x00
#define SDR_SENSOR_L_LN         0x01
#define SDR_SENSOR_L_LOG10      0x02
#define SDR_SENSOR_L_LOG2       0x03
#define SDR_SENSOR_L_E          0x04
#define SDR_SENSOR_L_EXP10      0x05
#define SDR_SENSOR_L_EXP2       0x06
#define SDR_SENSOR_L_1_X        0x07
#define SDR_SENSOR_L_SQR        0x08
#define SDR_SENSOR_L_CUBE       0x09
#define SDR_SENSOR_L_SQRT       0x0a
#define SDR_SENSOR_L_CUBERT     0x0b
#define SDR_SENSOR_L_NONLINEAR  0x70

	uint8_t linearization;	/* 70h=non linear, 71h-7Fh=non linear, OEM */
	uint16_t mtol;		/* M, tolerance */
	uint32_t bacc;		/* accuracy, B, Bexp, Rexp */

	struct {
#ifdef WORDS_BIGENDIAN
		uint8_t __reserved : 5;
		uint8_t normal_min : 1;	/* normal min field specified */
		uint8_t normal_max : 1;	/* normal max field specified */
		uint8_t nominal_read : 1;	/* nominal reading field specified */
#else
		uint8_t nominal_read : 1;	/* nominal reading field specified */
		uint8_t normal_max : 1;	/* normal max field specified */
		uint8_t normal_min : 1;	/* normal min field specified */
		uint8_t __reserved : 5;
#endif
	} analog_flag;

	uint8_t nominal_read;	/* nominal reading, raw value */
	uint8_t normal_max;	/* normal maximum, raw value */
	uint8_t normal_min;	/* normal minimum, raw value */
	uint8_t sensor_max;	/* sensor maximum, raw value */
	uint8_t sensor_min;	/* sensor minimum, raw value */

	struct {
		struct {
			uint8_t non_recover;
			uint8_t critical;
			uint8_t non_critical;
		} upper;
		struct {
			uint8_t non_recover;
			uint8_t critical;
			uint8_t non_critical;
		} lower;
		struct {
			uint8_t positive;
			uint8_t negative;
		} hysteresis;
	} threshold;
	uint8_t __reserved[2];
	uint8_t oem;		/* reserved for OEM use */
	uint8_t id_code;	/* sensor ID string type/length code */
	uint8_t id_string[16];	/* sensor ID string bytes, only if id_code != 0 */
} __attribute__((packed));

int32_t pow_convert(int32_t *a, int32_t b)
{
	/* function input parameter (a * 10 ^ b) */
	int32_t i = 0, r = 1, temp_b = 0;

	temp_b = (b > 0) ? b : -b;

	for (i = 0; i < temp_b; i++) r = r * 10;

	if (b > 0)
	{
		*a = (*a) * r;
		r = 1;
	}
	/* function return parameter calc_result = *a, decimal_point = r */
	return r;
}

void simple_atoi(const char *buf, int8_t *output_val)
{
	while (*buf >= '0' && *buf <= '9')
	{
		*output_val = *output_val * 10 + *buf - '0';
		buf++;
	}
}

int32_t ipmi_send_system_cmd(struct kernel_ipmi_msg msg, void *user_msg_data)
{
	int32_t rv = 0;
	int32_t msgid = 0;
	int32_t escape_time = 0;

	struct ipmi_addr addr;
	struct ipmi_system_interface_addr bmc_addr = {
	addr_type:
		IPMI_SYSTEM_INTERFACE_ADDR_TYPE,
			channel :
			IPMI_BMC_CHANNEL,
	};

	if (!g_bmc_alive_flag) return -1;

	bmc_addr.lun = 0;
	msgid = ++g_ipmi_sync_flag;
	memcpy(&addr, &bmc_addr, sizeof(bmc_addr));

#if enable_debug_msg
	{
		int32_t i = 0;
		printk("%s: type = 0x%x, channel = 0x%x, msgid = %d, netfn = 0x%x, cmd = 0x%x, data_len %d\n", __func__, addr.addr_type, addr.channel, msgid, msg.netfn, msg.cmd, msg.data_len);
		for (i = 0; i < msg.data_len; i++)
		{
			printk("%02x ", msg.data[i]);
		}
		printk("\n");
	}
#endif

	ipmi_request_settime(ipmi_mh_user, &addr, msgid, &msg, user_msg_data, 0, -1, 0);
	escape_time = wait_for_completion_timeout(&g_read_complete, IPMI_TIMEOUT);

	if (escape_time == 0)
	{
		g_bmc_alive_flag = false;

		printk("BMC not response (%d)\n", escape_time);
		rv = -1;
	}
	else {
#if enable_debug_msg
		printk("BMC response!!!! (%d)\n", escape_time);
#endif
		rv = 0;
	}

	return rv;

}

static void msg_handler(struct ipmi_recv_msg *msg, void *handler_data)
{
	int32_t data_len = 0;

	data_len = msg->msg.data_len - 1;

	if (data_len > 0)
	{
		memcpy(msg->user_msg_data, msg->msg.data + 1, data_len);
	}
	else
	{
		if (msg->user_msg_data != NULL) memset(msg->user_msg_data, 0xff, 1);
	}

	if (g_ipmi_sync_flag == msg->msgid)
	{
		g_ipmi_sync_flag = 0;
		ipmi_free_recv_msg(msg);

		if (g_bmc_alive_flag)	complete(&g_read_complete);
		else					g_bmc_alive_flag = true;
	}
	else
	{
		printk("unexpect flow (%d, %ld)\n", g_ipmi_sync_flag, msg->msgid);
	}
}
static struct ipmi_user_hndl ipmi_hndlrs = { .ipmi_recv_hndl = msg_handler, };

int32_t ipmi_sdr_get_reservation(uint16_t * reserve_id)
{
	int32_t rv = 0;
	struct kernel_ipmi_msg msg;

	/* obtain reservation ID */
	memset(&msg, 0, sizeof(msg));

	if (g_use_built_in == 0)	msg.netfn = IPMI_NETFN_STORAGE;
	else						msg.netfn = IPMI_NETFN_SE;

	msg.cmd = GET_SDR_RESERVE_REPO;
	rv = ipmi_send_system_cmd(msg, reserve_id);
	if (rv) printk("BMC down at (%d)!!\n", __LINE__);

#if enable_debug_msg
	printk("SDR reservation ID %04x\n", *reserve_id);
#endif

	return rv;
}

int32_t ipmi_sdr_start(struct ipmi_sdr_iterator *itr)
{
	int32_t rv = 0;
	struct kernel_ipmi_msg msg;
	struct ipm_devid_rsp devid;

	/* check SDRR capability */
	memset(&msg, 0, sizeof(msg));
	msg.netfn = IPMI_NETFN_APP;
	msg.cmd = BMC_GET_DEVICE_ID;
	msg.data_len = 0;

	rv = ipmi_send_system_cmd(msg, &devid);
	if (rv)
	{
		printk("BMC down at (%d)!!\n", __LINE__);
		return rv;
	}

	if (devid.device_revision & IPM_DEV_DEVICE_ID_SDR_MASK)
	{
		if ((devid.adtl_device_support & 0x02) == 0)
		{
			if ((devid.adtl_device_support & 0x01))
			{
				printk("Using Device SDRs\n");
				g_use_built_in = 1;
			}
			else
			{
				printk("Error obtaining SDR info\n");
			}
		}
	}

	memset(&msg, 0, sizeof(msg));
	if (g_use_built_in == 0)
	{
		struct sdr_repo_info_rs sdr_info;
		/* get sdr repository info */
		msg.netfn = IPMI_NETFN_STORAGE;
		msg.cmd = GET_SDR_REPO_INFO;
		rv = ipmi_send_system_cmd(msg, &sdr_info);
		itr->total = sdr_info.count;
	}
	else
	{
		struct sdr_device_info_rs sdr_info;
		/* get device sdr info */
		msg.netfn = IPMI_NETFN_SE;
		msg.cmd = GET_SDR_REPO_INFO;
		rv = ipmi_send_system_cmd(msg, &sdr_info);
		itr->total = sdr_info.count;
	}

	if (rv)
	{
		printk("BMC down at (%d)!!\n", __LINE__);
		return rv;
	}

	itr->next = 0;

#if enable_debug_msg
	printk("SDR version: 0x%x\n", sdr_info.version);
	printk("SDR free space: %d\n", sdr_info.free);
	printk("SDR records   : %d\n", sdr_info.count);
#endif

	rv = ipmi_sdr_get_reservation(&(itr->reservation));

	return rv;
}

static struct sdr_get_rs* ipmi_sdr_get_header(struct ipmi_sdr_iterator *itr)
{
	int32_t rv = 0;
	struct sdr_get_rq sdr_rq;
	static struct sdr_get_rs sdr_rs;
	struct kernel_ipmi_msg msg;

	memset(&sdr_rq, 0, sizeof(sdr_rq));
	memset(&sdr_rs, 0, sizeof(sdr_rs));

	sdr_rq.reserve_id = itr->reservation;
	sdr_rq.id = itr->next;
	sdr_rq.offset = 0;
	sdr_rq.length = 5;	/* only get the header */

	memset(&msg, 0, sizeof(msg));
	if (g_use_built_in == 0)
	{
		msg.netfn = IPMI_NETFN_STORAGE;
		msg.cmd = GET_SDR;
	}
	else
	{
		msg.netfn = IPMI_NETFN_SE;
		msg.cmd = GET_DEVICE_SDR;
	}

	msg.data = (uint8_t *)&sdr_rq;
	msg.data_len = sizeof(sdr_rq);
	rv = ipmi_send_system_cmd(msg, &sdr_rs);
	if ((rv) || (sdr_rs.length == 0))
	{
		printk("SDR record id 0x%04x: invalid length %d", itr->next, sdr_rs.length);
		return NULL;
	}

	if (sdr_rs.id != itr->next)
	{
		printk("SDR record id mismatch: 0x%04x\n", sdr_rs.id);
		sdr_rs.id = itr->next;
	}
#if enable_debug_msg
	printk("\nSDR record ID   : 0x%04x", itr->next);
	printk("SDR record type : 0x%02x\n", sdr_rs.type);
	printk("SDR record next : 0x%04x\n", sdr_rs.next);
	printk("SDR record bytes: %d\n", sdr_rs.length);
#endif

	return &sdr_rs;
}

int32_t ipmi_sdr_get_record(struct sdr_get_rs * header, struct ipmi_sdr_iterator * itr, uint8_t *ret_data)
{
	int32_t rv = 0, len = 0;
	uint8_t buff[128] = "";

	struct sdr_get_rq sdr_rq;
	struct kernel_ipmi_msg msg;

	len = header->length;
	if (len > 0)
	{
		memset(&sdr_rq, 0, sizeof(sdr_rq));
		sdr_rq.reserve_id = itr->reservation;
		sdr_rq.id = header->id;
		sdr_rq.offset = 5;
		sdr_rq.length = len;

		memset(&msg, 0, sizeof(msg));
		if (g_use_built_in == 0)
		{
			msg.netfn = IPMI_NETFN_STORAGE;
			msg.cmd = GET_SDR;
		}
		else
		{
			msg.netfn = IPMI_NETFN_SE;
			msg.cmd = GET_DEVICE_SDR;
		}

		msg.data = (uint8_t *)&sdr_rq;
		msg.data_len = sizeof(sdr_rq);
		rv = ipmi_send_system_cmd(msg, ret_data);
		if (rv)
		{
			printk("BMC down at (%d)!!\n", __LINE__);
			return rv;
		}

		memset(buff, 0, sizeof(buff));
		memcpy(buff, ret_data + 2, sdr_rq.length);
		memcpy(ret_data, buff, sdr_rq.length + 2);
	}

	return rv;
}

void ipmi_sdr_set_sensor_threshold(uint8_t idx, struct sdr_record_full_sensor *sensor)
{

	/*refer to Table 35-, Get Sensor Event Enable*/
	/*
	// change detect threshold method, keep it for record detail format
	// in this version function input is 
		"void ipmi_sdr_set_sensor_threshold(uint8_t idx, uint8_t *rec)"
		#define offset_threshold_enable	9
		#define offset_threshold_data	31
	if (rec[offset_threshold_enable + 1] & 0x08)	g_sensor_data[idx].upperinfo.unr_high = 1;
	if (rec[offset_threshold_enable + 1] & 0x04)	g_sensor_data[idx].upperinfo.unr_low = 1;
	if (rec[offset_threshold_enable + 1] & 0x02)	g_sensor_data[idx].upperinfo.ucr_high = 1;
	if (rec[offset_threshold_enable + 1] & 0x01)	g_sensor_data[idx].upperinfo.ucr_low = 1;
	if (rec[offset_threshold_enable] & 0x80)		g_sensor_data[idx].upperinfo.unc_high = 1;
	if (rec[offset_threshold_enable] & 0x40)		g_sensor_data[idx].upperinfo.unc_low = 1;

	if (rec[offset_threshold_enable] & 0x20)		g_sensor_data[idx].lowerinfo.lnr_high = 1;
	if (rec[offset_threshold_enable] & 0x10)		g_sensor_data[idx].lowerinfo.lnr_low = 1;
	if (rec[offset_threshold_enable] & 0x08)		g_sensor_data[idx].lowerinfo.lcr_high = 1;
	if (rec[offset_threshold_enable] & 0x04)		g_sensor_data[idx].lowerinfo.lcr_low = 1;
	if (rec[offset_threshold_enable] & 0x02)		g_sensor_data[idx].lowerinfo.lnc_high = 1;
	if (rec[offset_threshold_enable] & 0x01)		g_sensor_data[idx].lowerinfo.lnc_low = 1;
	//*/

	/* lower threshold info */
	if (sensor->mask.type.threshold.read.lnc) g_sensor_data[idx].capability |= SENSOR_GET_CAP_LNC;
	if (sensor->mask.type.threshold.read.lcr) g_sensor_data[idx].capability |= SENSOR_GET_CAP_LCR;
	if (sensor->mask.type.threshold.read.lnr) g_sensor_data[idx].capability |= SENSOR_GET_CAP_LNR;
	g_sensor_data[idx].lowerinfo.lnc = sensor->threshold.lower.non_critical;
	g_sensor_data[idx].lowerinfo.lcr = sensor->threshold.lower.critical;
	g_sensor_data[idx].lowerinfo.lnr = sensor->threshold.lower.non_recover;

	/* upper threshold info */
	if (sensor->mask.type.threshold.read.unc) g_sensor_data[idx].capability |= SENSOR_GET_CAP_UNC;
	if (sensor->mask.type.threshold.read.ucr) g_sensor_data[idx].capability |= SENSOR_GET_CAP_UCR;
	if (sensor->mask.type.threshold.read.unr) g_sensor_data[idx].capability |= SENSOR_GET_CAP_UNR;
	g_sensor_data[idx].upperinfo.unc = sensor->threshold.upper.non_critical;
	g_sensor_data[idx].upperinfo.ucr = sensor->threshold.upper.critical;
	g_sensor_data[idx].upperinfo.unr = sensor->threshold.upper.non_recover;
}

void ipmi_sdr_set_sensor_factor(uint8_t idx, struct sdr_record_full_sensor *sensor)
{
	char *loc = NULL;

	g_sensor_data[idx].sensor_type = sensor->sensor.type;
	sprintf(g_sensor_data[idx].sensor_idstring, "%s", sensor->id_string);

	g_sensor_data[idx].recordinfo.record_m = __TO_M(sensor->mtol);
	g_sensor_data[idx].recordinfo.record_b = __TO_B(sensor->bacc);
	g_sensor_data[idx].recordinfo.record_k1 = __TO_B_EXP(sensor->bacc);
	g_sensor_data[idx].recordinfo.record_k2 = __TO_R_EXP(sensor->bacc);

	g_sensor_data[idx].recordinfo.record_analog = sensor->unit.analog;
	g_sensor_data[idx].recordinfo.record_linearization = sensor->linearization;

	memset(g_sensor_data[idx].attrinfo.attr_type_str, 0x00, SENSOR_ATTR_NAME_LENGTH);

	switch (g_sensor_data[idx].sensor_type)
	{
	case SDR_SENSOR_TYPE_TEMP:
		sprintf(g_sensor_data[idx].attrinfo.attr_type_str, "temp");
		break;
	case SDR_SENSOR_TYPE_VOLT:
		sprintf(g_sensor_data[idx].attrinfo.attr_type_str, "in");
		break;
	case SDR_SENSOR_TYPE_FAN:
		g_sensor_data[idx].capability |= SENSOR_GET_CAP_PWM;
		g_sensor_data[idx].capability |= SENSOR_GET_CAP_CONMODE;
		g_sensor_data[idx].capability |= SENSOR_GET_CAP_DIRECTION;
		sprintf(g_sensor_data[idx].attrinfo.attr_type_str, "fan");
		break;
	case SDR_SENSOR_TYPE_PS:
		loc = strstr(g_sensor_data[idx].sensor_idstring, "POWER");
		if (loc) {
			if ((strncmp(g_sensor_data[idx].sensor_idstring + 11, "OUT", 3)) == 0)
			{
				g_sensor_data[idx].capability |= SENSOR_GET_CAP_MODEL;
				g_sensor_data[idx].capability |= SENSOR_GET_CAP_SN;
				g_sensor_data[idx].capability |= SENSOR_GET_CAP_PSU_PRESENT;
			}
			sprintf(g_sensor_data[idx].attrinfo.attr_type_str, "power");
		}

		loc = strstr(g_sensor_data[idx].sensor_idstring, "VOLTAGE");
		if (loc) sprintf(g_sensor_data[idx].attrinfo.attr_type_str, "in");

		loc = strstr(g_sensor_data[idx].sensor_idstring, "CURRENT");
		if (loc) sprintf(g_sensor_data[idx].attrinfo.attr_type_str, "curr");

		break;
	case SDR_SENSOR_TYPE_CURR:
		sprintf(g_sensor_data[idx].attrinfo.attr_type_str, "curr");
		break;
	case SDR_SENSOR_TYPE_OTHER:
		sprintf(g_sensor_data[idx].attrinfo.attr_type_str, "other");
		break;
	default:
		printk("not support sensor type !! [%d]\n", g_sensor_data[idx].sensor_type);
		break;
	}

	if ((strncmp(g_sensor_data[idx].sensor_idstring, "Fan", 3)) == 0)
	{
		g_sensor_data[idx].capability |= SENSOR_GET_CAP_FAN_PRESENT;
	}

#if enable_debug_msg
	{
		printk("\n********************\n");

		printk("m[%d], b[%d], k1[%d], k2[%d]\n", g_sensor_data[idx].recordinfo.record_m, g_sensor_data[idx].recordinfo.record_b
			, g_sensor_data[idx].recordinfo.record_k1, g_sensor_data[idx].recordinfo.record_k2);

		printk("sensor [%s] type[%d], analog[%d], linearization[%d]\n", g_sensor_data[idx].sensor_idstring, g_sensor_data[idx].sensor_type
			, g_sensor_data[idx].recordinfo.record_analog, g_sensor_data[idx].recordinfo.record_linearization);

		printk("\n********************\n");
	}
#endif

}

void sdr_convert_sensor_reading(uint8_t idx, uint8_t val, uint32_t *ret1, uint32_t *ret2)
{
	int32_t m = g_sensor_data[idx].recordinfo.record_m;
	int32_t b = g_sensor_data[idx].recordinfo.record_b;
	int32_t k1 = g_sensor_data[idx].recordinfo.record_k1;
	int32_t k2 = g_sensor_data[idx].recordinfo.record_k2;
	int32_t result = 0;

	int32_t point_b = 0, point_k2 = 0, point_result = 0;

	point_b = pow_convert(&b, k1);

	switch (g_sensor_data[idx].recordinfo.record_analog)
	{
	case 0:
		result = m * val * point_b + b;
		break;
	case 1:
		if (val & 0x80) val++;
	case 2:
		result = (m * (int16_t)val) * point_b + b;
		break;
	default:
		return;
	}

	point_k2 = pow_convert(&result, k2);

	point_result = point_b * point_k2;

	if (g_sensor_data[idx].sensor_type != SDR_SENSOR_TYPE_FAN)
	{
		result = result * 1000; //shift for lm-sensors
	}

	*ret1 = result / point_result;
	*ret2 = result % point_result;
}

uint8_t ipmi_check_psu_present(uint8_t psu_slot)
{
	uint8_t slot_mask = 0x0;

	uint8_t returnData[128] = { 0 };

	struct kernel_ipmi_msg msg;

	memset(&msg, 0, sizeof(msg));
	mutex_lock(&ipmi_lock);

	msg.data = kmalloc(4, GFP_KERNEL);
	memset(msg.data, 0, 4);

	msg.netfn = IPMI_NETFN_APP;
	msg.cmd = GET_PSU_READING;
	msg.data[0] = 0x1b;
	msg.data[1] = 0x4c;
	msg.data[2] = 0x01;
	msg.data[3] = 0x00;
	msg.data_len = 4;

	ipmi_send_system_cmd(msg, returnData);
	kfree(msg.data);
	mutex_unlock(&ipmi_lock);

	if (psu_slot == 1)	slot_mask = 0x01;
	else				slot_mask = 0x10;

	if (returnData[0] & slot_mask)	return 0; //no detect
	else							return 1; //detected!!
}

void ipmi_get_psu_info(uint8_t idx, uint8_t cmd, uint8_t *retbuf)
{
	uint8_t psu_slot = 0;

	uint8_t returnData[128] = { 0 };

	if (strstr(g_sensor_data[idx].sensor_idstring, "PSU1"))	psu_slot = 1;
	else													psu_slot = 2;

	if (ipmi_check_psu_present(psu_slot))
	{
		struct kernel_ipmi_msg msg;

		memset(&msg, 0, sizeof(msg));
		mutex_lock(&ipmi_lock);

		msg.data = kmalloc(4, GFP_KERNEL);
		memset(msg.data, 0, 4);

		msg.netfn = IPMI_NETFN_APP;
		msg.cmd = GET_PSU_READING;
		msg.data[0] = 0x0f;
		msg.data[1] = (psu_slot == 1) ? 0xb0 : 0xb2;
		msg.data[2] = 0x80;
		msg.data[3] = cmd;
		msg.data_len = 4;

		ipmi_send_system_cmd(msg, returnData);
		kfree(msg.data);
		mutex_unlock(&ipmi_lock);

		snprintf(retbuf, returnData[0] + 1, "%s\n", returnData + 1);
	}
	else
	{
		//printk("Error ! cannot detect PSU%d\n", psu_slot);
	}
}

void ipmi_fan_control(uint8_t cmd_data1, uint8_t cmd_data2, uint8_t *retbuf)
{
	uint8_t returnData[10] = { 0 };

	struct kernel_ipmi_msg msg;

	memset(&msg, 0, sizeof(msg));
	mutex_lock(&ipmi_lock);

	msg.data = kmalloc(2, GFP_KERNEL);
	memset(msg.data, 0, 2);

	msg.netfn = IPMI_NETFN_TSOL;
	msg.cmd = GET_FAN_INFO;
	msg.data[0] = cmd_data1;
	msg.data[1] = cmd_data2;
	msg.data_len = 2;

	ipmi_send_system_cmd(msg, returnData);
	kfree(msg.data);
	mutex_unlock(&ipmi_lock);

	sprintf(retbuf, "%s", returnData);
}

static ssize_t show_label(struct device *dev, struct device_attribute *devattr, char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
	return sprintf(buf, "%s\n", g_sensor_data[attr->index + DEBUGUSE_SHIFT].sensor_idstring);
}

static ssize_t show_crit_alarm(struct device *dev, struct device_attribute *devattr, char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
	return sprintf(buf, "%d\n", attr->index);
}

static ssize_t show_input(struct device *dev, struct device_attribute *devattr, char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
	uint32_t val1 = 0, val2 = 0;

	uint8_t returnData[4] = { 0 };

	struct kernel_ipmi_msg msg;

	memset(&msg, 0, sizeof(msg));
	mutex_lock(&ipmi_lock);

	msg.data = kmalloc(1, GFP_KERNEL);
	memset(msg.data, 0, 1);

	msg.netfn = IPMI_NETFN_SE;
	msg.cmd = GET_SENSOR_READING;
	msg.data[0] = g_sensor_data[attr->index + DEBUGUSE_SHIFT].addr;
	msg.data_len = 1;

	ipmi_send_system_cmd(msg, returnData);
	kfree(msg.data);
	mutex_unlock(&ipmi_lock);

	sdr_convert_sensor_reading(attr->index + DEBUGUSE_SHIFT, returnData[0], &val1, &val2);

	return sprintf(buf, "%u.%u\n", val1, val2);
}

static ssize_t show_lnr(struct device *dev, struct device_attribute *devattr, char *buf)
{
	uint32_t val1 = 0, val2 = 0;
	struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
	sdr_convert_sensor_reading(attr->index + DEBUGUSE_SHIFT, g_sensor_data[attr->index + DEBUGUSE_SHIFT].lowerinfo.lnr, &val1, &val2);
	return sprintf(buf, "%u.%u\n", val1, val2);
}

static ssize_t show_lcr(struct device *dev, struct device_attribute *devattr, char *buf)
{
	uint32_t val1 = 0, val2 = 0;
	struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
	sdr_convert_sensor_reading(attr->index + DEBUGUSE_SHIFT, g_sensor_data[attr->index + DEBUGUSE_SHIFT].lowerinfo.lcr, &val1, &val2);
	return sprintf(buf, "%u.%u\n", val1, val2);
}

static ssize_t show_lnc(struct device *dev, struct device_attribute *devattr, char *buf)
{
	uint32_t val1 = 0, val2 = 0;
	struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
	sdr_convert_sensor_reading(attr->index + DEBUGUSE_SHIFT, g_sensor_data[attr->index + DEBUGUSE_SHIFT].lowerinfo.lnc, &val1, &val2);
	return sprintf(buf, "%u.%u\n", val1, val2);
}

static ssize_t show_unr(struct device *dev, struct device_attribute *devattr, char *buf)
{
	uint32_t val1 = 0, val2 = 0;
	struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
	sdr_convert_sensor_reading(attr->index + DEBUGUSE_SHIFT, g_sensor_data[attr->index + DEBUGUSE_SHIFT].upperinfo.unr, &val1, &val2);
	return sprintf(buf, "%u.%u\n", val1, val2);
}

static ssize_t show_ucr(struct device *dev, struct device_attribute *devattr, char *buf)
{
	uint32_t val1 = 0, val2 = 0;
	struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
	sdr_convert_sensor_reading(attr->index + DEBUGUSE_SHIFT, g_sensor_data[attr->index + DEBUGUSE_SHIFT].upperinfo.ucr, &val1, &val2);
	return sprintf(buf, "%u.%u\n", val1, val2);
}

static ssize_t show_unc(struct device *dev, struct device_attribute *devattr, char *buf)
{
	uint32_t val1 = 0, val2 = 0;
	struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
	sdr_convert_sensor_reading(attr->index + DEBUGUSE_SHIFT, g_sensor_data[attr->index + DEBUGUSE_SHIFT].upperinfo.unc, &val1, &val2);
	return sprintf(buf, "%u.%u\n", val1, val2);
}

static ssize_t show_model(struct device *dev, struct device_attribute *devattr, char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
	uint8_t returnData[128] = { 0 };
	ipmi_get_psu_info(attr->index + DEBUGUSE_SHIFT, 0x9a, returnData);
	return sprintf(buf, "%s\n", returnData);
}

static ssize_t show_sn(struct device *dev, struct device_attribute *devattr, char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
	uint8_t returnData[128] = { 0 };
	ipmi_get_psu_info(attr->index + DEBUGUSE_SHIFT, 0x9e, returnData);
	return sprintf(buf, "%s\n", returnData);
}

static ssize_t show_pwm(struct device *dev, struct device_attribute *devattr, char *buf)
{
	uint8_t returnData[1] = { 0 };
	ipmi_fan_control(0x00, 0x00, returnData);
	return sprintf(buf, "%d\n", returnData[0]);
}

static ssize_t store_pwm(struct device *dev, struct device_attribute *devattr, const char *buf, size_t count)
{
	uint8_t store_input = 0;
	uint8_t returnData[10] = { 0 };
	simple_atoi(buf, &store_input);
	if (g_fan_control_mode == 1) ipmi_fan_control(0x01, store_input, returnData);

	return count;
}

static ssize_t show_controlmode(struct device *dev, struct device_attribute *devattr, char *buf)
{
	return sprintf(buf, "%d\n", g_fan_control_mode);
}

static ssize_t store_controlmode(struct device *dev, struct device_attribute *devattr, const char *buf, size_t count)
{
	uint8_t store_input = 0;
	uint8_t returnData[10] = { 0 };
	simple_atoi(buf, &store_input);
	g_fan_control_mode = store_input;
	if (g_fan_control_mode == 3) ipmi_fan_control(0x7f, 0xff, returnData);

	return count;
}

static ssize_t show_direction(struct device *dev, struct device_attribute *devattr, char *buf)
{
	uint8_t returnData[10] = { 0 };

	struct kernel_ipmi_msg msg;

	memset(&msg, 0, sizeof(msg));
	mutex_lock(&ipmi_lock);

	msg.data = kmalloc(4, GFP_KERNEL);
	memset(msg.data, 0, 4);

	msg.netfn = IPMI_NETFN_STORAGE;
	msg.cmd = GET_FRU_INFO;
	msg.data[0] = 0x0;
	msg.data[1] = 0x19;
	msg.data[2] = 0x0;
	msg.data[3] = 0x1;
	msg.data_len = 4;

	ipmi_send_system_cmd(msg, returnData);
	kfree(msg.data);
	mutex_unlock(&ipmi_lock);

	return sprintf(buf, "%c\n", returnData[1]);
}

static ssize_t show_fanpresent(struct device *dev, struct device_attribute *devattr, char *buf)
{
	int32_t fan_idx = 0, fan_present = 0;

	uint8_t returnData[10] = { 0 };

	struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
	struct kernel_ipmi_msg msg;

	fan_idx = (g_sensor_data[attr->index].sensor_idstring[8] - '0') - 1;

	memset(&msg, 0, sizeof(msg));
	mutex_lock(&ipmi_lock);

	msg.data = kmalloc(4, GFP_KERNEL);
	memset(msg.data, 0, 4);

	msg.netfn = 0x36;
	msg.cmd = 0xB9;
	msg.data[0] = 0x4C;
	msg.data[1] = 0x1C;
	msg.data[2] = 0x00;
	msg.data[3] = 0x02;
	msg.data_len = 4;

	ipmi_send_system_cmd(msg, returnData);
	kfree(msg.data);
	mutex_unlock(&ipmi_lock);

	fan_present = ((returnData[0] >> fan_idx) & 0x1) ? 0 : 1;

	return sprintf(buf, "%d\n", fan_present);
}

static ssize_t show_psupresent(struct device *dev, struct device_attribute *devattr, char *buf)
{
	int32_t psu_idx = 0;

	struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);

	psu_idx = g_sensor_data[attr->index].sensor_idstring[3] - '0';

	return sprintf(buf, "%d\n", ipmi_check_psu_present(psu_idx));
}

static ssize_t(*const attr_show_func_ptr[SENSOR_ATTR_MAX]) (struct device *dev, struct device_attribute *devattr, char *buf) =
{
	show_label, show_crit_alarm, show_input
	, show_lnc, show_lcr, show_lnr
	, show_unc, show_ucr, show_unr
	, show_model, show_sn, show_pwm
	, show_controlmode, show_direction, show_fanpresent
	, show_psupresent
};

static ssize_t(*const attr_store_func_ptr[SENSOR_ATTR_MAX]) (struct device *dev, struct device_attribute *devattr, const char *buf, size_t count) =
{
	NULL, NULL, NULL
	, NULL, NULL, NULL
	, NULL, NULL, NULL
	, NULL, NULL, store_pwm
	, store_controlmode, NULL, NULL
	, NULL
};

static const char *const sensor_attrnames[SENSOR_ATTR_MAX] =
{
	"%s%d_label", "%s%d_crit_alarm", "%s%d_input"
	, "%s%d_lncrit", "%s%d_lcrit", "%s%d_min"
	, "%s%d_ncrit", "%s%d_crit", "%s%d_max"
	, "%s%d_model", "%s%d_sn", "%s%d_pwm"
	, "%s%d_controlmode", "%s%d_direction", "%s%d_present"
	, "%s%d_present"
};

static int32_t create_sensor_attrs(int32_t attr_no)
{
	int32_t i = 0, j = 0;

	struct attr_info *attrdata = &g_sensor_data[attr_no].attrinfo;

#if enable_debug_msg
	printk("##### %s:%d attr_no %d\n", __FUNCTION__, __LINE__, attr_no);
#endif

	for (i = 0; i < SENSOR_ATTR_MAX; i++)
	{
		if ((g_sensor_data[attr_no].capability >> i) & 0x01)
		{
			snprintf(attrdata->attr_name[j], SENSOR_ATTR_NAME_LENGTH, sensor_attrnames[i], attrdata->attr_type_str, attr_no - DEBUGUSE_SHIFT);

			sysfs_attr_init(&attrdata->sd_attrs[j].dev_attr.attr);
			attrdata->sd_attrs[j].dev_attr.attr.name = attrdata->attr_name[j];
			attrdata->sd_attrs[j].dev_attr.show = attr_show_func_ptr[i];
			attrdata->sd_attrs[j].dev_attr.store = attr_store_func_ptr[i];

			attrdata->sd_attrs[j].dev_attr.attr.mode = S_IRUGO;
			if (attrdata->sd_attrs[j].dev_attr.store)	attrdata->sd_attrs[j].dev_attr.attr.mode |= S_IWUSR;

			attrdata->sd_attrs[j].index = attr_no - DEBUGUSE_SHIFT;
			attrdata->attrs[j] = &attrdata->sd_attrs[j].dev_attr.attr;
			j++;

			data->total_create_sysfs++;
		}
	}

	attrdata->attrs[j] = NULL;
	attrdata->attr_group.attrs = attrdata->attrs;

	return sysfs_create_group(&data->ipmi_hwmon_dev->kobj, &attrdata->attr_group);
}

static int32_t remove_sensor_attrs(void)
{
	int32_t i = 0;

	for (i = 0; i < data->total_sensor_id; i++)
	{
		if (g_sensor_data[i].attrinfo.attr_exist)
		{
			sysfs_remove_group(&data->ipmi_hwmon_dev->kobj, &g_sensor_data[i].attrinfo.attr_group);
		}
	}
	return 0;
}

int32_t ipmi_init_sdr_sensors_data(void)
{
	int32_t sdr_idx = 0;
	int32_t err = 0;

	struct ipmi_sdr_iterator *itr = NULL;
	struct sdr_get_rs *header = NULL;

	uint8_t *rec = NULL;

	mutex_lock(&ipmi_lock);

	itr = kmalloc(sizeof(struct ipmi_sdr_iterator), GFP_KERNEL);
	if (itr == NULL)
	{
		printk("%s(%d): kmalloc failure.\n", __func__, __LINE__);
		goto itr_malloc_fail;
	}

	err = ipmi_sdr_start(itr);
	if (err)
	{
		printk("%s(%d): ipmi_sdr_start fail.\n", __func__, __LINE__);
		goto ipmi_sdr_start_fail;
	}

	data->total_sensor_id = itr->total;
	rec = kmalloc(GET_SDR_ENTIRE_RECORD, GFP_KERNEL);
	if (rec == NULL)
	{
		printk("%s(%d): kmalloc failure\n", __func__, __LINE__);
		goto rec_malloc_fail;
	}

	g_sensor_data = kmalloc(itr->total * sizeof(struct ipmi_sensor_data), GFP_KERNEL);
	if (g_sensor_data == NULL)
	{
		printk("%s(%d): malloc failure", __func__, __LINE__);
		goto g_sensor_data_malloc_fail;
	}

	memset(g_sensor_data, 0x0, itr->total * sizeof(struct ipmi_sensor_data));

	for (sdr_idx = 0; sdr_idx < itr->total; sdr_idx++)
	{
		header = ipmi_sdr_get_header(itr);
		if (header == NULL) continue;

		itr->next = header->next;

		memset(rec, 0, GET_SDR_ENTIRE_RECORD);

		err = ipmi_sdr_get_record(header, itr, rec);
		if ((err) || (rec[0] == 0xFF))
		{
			printk("\nipmi: Get SDR error,so break this request\n");
			goto ipmi_sdr_get_record_fail;
		}

		switch (header->type)
		{
		case SDR_RECORD_TYPE_FULL_SENSOR:
			/* prepare (threshold, factor)data whilie init, for reduce reading step and improve operate speed */
			g_sensor_data[sdr_idx].addr = rec[2];
			g_sensor_data[sdr_idx].capability = SENSOR_GET_CAP_LABEL /*| SENSOR_GET_CAP_ALARM */| SENSOR_GET_CAP_INPUT;
			g_sensor_data[sdr_idx].headerinfo.header_type = header->type;
			g_sensor_data[sdr_idx].headerinfo.header_byte = header->length;

			ipmi_sdr_set_sensor_threshold(sdr_idx, (struct sdr_record_full_sensor*) rec);
			ipmi_sdr_set_sensor_factor(sdr_idx, (struct sdr_record_full_sensor*) rec);

			if (sdr_idx >= DEBUGUSE_SHIFT)
			{
				err = create_sensor_attrs(sdr_idx);
				if (err)
				{
					printk("[err : %d]sysfs_create_group fail in [%d] %s\n", err, sdr_idx, g_sensor_data[sdr_idx].sensor_idstring);
					return err;
				}
				g_sensor_data[sdr_idx].attrinfo.attr_exist = 1;
			}

			data->total_suport_sensor++;

			break;
		case SDR_RECORD_TYPE_COMPACT_SENSOR: /* not supporrt now */
		case SDR_RECORD_TYPE_EVENTONLY_SENSOR: /* not supporrt now */
		case SDR_RECORD_TYPE_MC_DEVICE_LOCATOR: /* not supporrt now */
		default:
			g_sensor_data[sdr_idx].attrinfo.attr_exist = 0;
#if enable_debug_msg
			printk("ID[%d] : not support type [%d]\n", sdr_idx, header->type);
#endif
			break;
		}
	}

	printk("quanta_hwmon_ipmi : detected [%d] sensor, create [%d] sysfs\n", data->total_suport_sensor, data->total_create_sysfs);

ipmi_sdr_get_record_fail:
g_sensor_data_malloc_fail:
	if (rec) {
		kfree(rec);
		rec = NULL;
	}

rec_malloc_fail:
ipmi_sdr_start_fail:
	if (itr) {
		kfree(itr);
		itr = NULL;
	}

itr_malloc_fail:
	mutex_unlock(&ipmi_lock);

	return err;
}

static ssize_t set_i2c_monitor_enable(struct device *dev, struct device_attribute *devattr, const char *buf, size_t count)
{
	int32_t store_input = 0;
	uint8_t returnData[10] = { 0 };

	struct kernel_ipmi_msg msg;

	simple_atoi(buf, &store_input);
	if ((store_input != 0) && (store_input != 1)) return -1;

	memset(&msg, 0, sizeof(msg));
	mutex_lock(&ipmi_lock);

	msg.data = kmalloc(5, GFP_KERNEL);
	memset(msg.data, 0, 5);

	msg.netfn = 0x06;
	msg.cmd = 0x52;
	msg.data[0] = 0x11;
	msg.data[1] = 0x40;
	msg.data[2] = 0x00;
	msg.data[3] = 0x03;
	msg.data[4] = (store_input)? 0xff : 0xf7;
	msg.data_len = 5;

	ipmi_send_system_cmd(msg, returnData);

	/* set p13 i2c_monitor_en direction output */
	msg.data[3] = 0x07;
	msg.data[4] = 0xf7;
	ipmi_send_system_cmd(msg, returnData);

	/* always set i2c_watchdog_en in system cpld */
	msg.data[0] = 0x1b;
	msg.data[1] = 0x80;
	msg.data[2] = 0x00;
	msg.data[3] = 0x04; /* i2c_watchdog_en */
	msg.data[4] = 0x01; /* enable */

	ipmi_send_system_cmd(msg, returnData);

	kfree(msg.data);
	mutex_unlock(&ipmi_lock);

	return count;
}

int32_t get_i2c_monitor_ipmi(void)
{
	uint8_t returnData[10] = { 0 };
	struct kernel_ipmi_msg msg;

	memset(&msg, 0, sizeof(msg));
	mutex_lock(&ipmi_lock);

	msg.data = kmalloc(4, GFP_KERNEL);
	memset(msg.data, 0, 4);

	msg.netfn = 0x06;
	msg.cmd = 0x52;
	msg.data[0] = 0x11;
	msg.data[1] = 0x40;
	msg.data[2] = 0x01;
	msg.data[3] = 0x01;
	msg.data_len = 4;
	ipmi_send_system_cmd(msg, returnData);

	kfree(msg.data);
	mutex_unlock(&ipmi_lock);

	return returnData[0];
}

static ssize_t get_i2c_monitor_enable(struct device *dev, struct device_attribute *devattr, char *buf)
{
	int rv = 0;
	rv = get_i2c_monitor_ipmi();
	return sprintf(buf, "%d\n", (rv & 0x08)? 1 : 0);
}

static DEVICE_ATTR(i2c_monitor_enable, S_IRUGO | S_IWUSR, get_i2c_monitor_enable, set_i2c_monitor_enable);

int32_t rw_i2c_monitor_interval(int32_t input_interval)
{
	uint8_t returnData[10] = { 0 };
	struct kernel_ipmi_msg msg;

	memset(&msg, 0, sizeof(msg));
	mutex_lock(&ipmi_lock);

	msg.data = kmalloc(5, GFP_KERNEL);
	memset(msg.data, 0, 5);

	msg.netfn = 0x06;
	msg.cmd = 0x52;
	msg.data[0] = 0x1b;
	msg.data[1] = 0x80;
	msg.data[2] = (input_interval)? 0x00 : 0x01;
	msg.data[3] = 0x05;
	msg.data[4] = input_interval;
	msg.data_len = (input_interval)? 5 : 4;;
	ipmi_send_system_cmd(msg, returnData);

	kfree(msg.data);
	mutex_unlock(&ipmi_lock);

	return returnData[0];
}

static ssize_t set_i2c_monitor_interval(struct device *dev, struct device_attribute *devattr, const char *buf, size_t count)
{
	int32_t store_input = 0;
	simple_atoi(buf, &store_input);

	if ((store_input < 0) || (store_input > 0xff)) return -1;
	rw_i2c_monitor_interval(store_input);

	return count;
}
static ssize_t get_i2c_monitor_interval(struct device *dev, struct device_attribute *devattr, char *buf)
{
	return sprintf(buf, "%d\n", rw_i2c_monitor_interval(0));
}

static DEVICE_ATTR(i2c_monitor_interval, S_IRUGO | S_IWUSR, get_i2c_monitor_interval, set_i2c_monitor_interval);

static ssize_t get_i2c_monitor_alert(struct device *dev, struct device_attribute *devattr, char *buf)
{
	int rv = 0;
	rv = get_i2c_monitor_ipmi();
	return sprintf(buf, "%d\n", (rv & 0x10)? 0 : 1);
}

static DEVICE_ATTR(i2c_monitor_alert, S_IRUGO, get_i2c_monitor_alert, NULL);

static const struct attribute *sys_attrs[] = {
	&dev_attr_i2c_monitor_enable.attr,
	&dev_attr_i2c_monitor_interval.attr,
	&dev_attr_i2c_monitor_alert.attr,
	NULL,
};
static const struct attribute_group sys_attr_group = {
	.attrs = (struct attribute **) sys_attrs,
};


int32_t create_sys_control_attr(void)
{
	int32_t rv = 0;

	quanta_class = class_create(THIS_MODULE, CLASSNAME);
	if (IS_ERR(quanta_class)) {
		rv = PTR_ERR(quanta_class);
		printk("error : class_create failed!\n");
		goto err_cls_create;
	}

	quanta_drv_data = kzalloc(sizeof(struct quanta_sys_data), GFP_KERNEL);
	if (quanta_drv_data == NULL)
	{
		rv = -ENOMEM;
		goto err_alloc_mem;
	}

	quanta_drv_dev = device_create(quanta_class, NULL, MKDEV(0, 0), quanta_drv_data, CLASS_SYSNAME);
	if (IS_ERR(quanta_drv_dev)) {
		rv = PTR_ERR(quanta_drv_dev);
		printk("error : quanta_drv_dev device_create failed!\n");
		goto err_dev_create;
	}

	dev_set_drvdata(quanta_drv_dev, quanta_drv_data);

	rv = sysfs_create_group(&quanta_drv_dev->kobj, &sys_attr_group);
	if (rv) {
		printk("error : sysfs_create_group failed!\n");
		goto err_sysfs_create;
	}

	printk("init sys attr done\n");

	return rv;

err_sysfs_create:
	device_unregister(quanta_drv_dev);

err_dev_create:
	if (quanta_drv_data) kfree(quanta_drv_data);

err_alloc_mem:
	class_destroy(quanta_class);

err_cls_create:

	return rv;
}


static int32_t __init quanta_hwmon_ipmi_init(void)
{
	int32_t err = 0;

	init_completion(&g_read_complete);

	data = kzalloc(sizeof(struct quanta_hwmon_ipmi_data), GFP_KERNEL);
	if (NULL == data)
	{
		printk("alloc data fail\n");
		goto alloc_err;
	}

	data->ipmi_platform_dev = platform_device_register_simple(DRVNAME, -1, NULL, 0);
	err = IS_ERR(data->ipmi_platform_dev);
	if (err) {
		printk("platform device register fail (err : %d)\n", err);
		goto device_reg_err;
	}

	data->ipmi_hwmon_dev = hwmon_device_register_with_groups(NULL, DRVNAME, NULL, NULL);
	err = IS_ERR(data->ipmi_hwmon_dev);
	if (err)
	{
		printk("hwmon register fail\n");
		goto hwmon_register_err;
	}

	err = ipmi_create_user(0, &ipmi_hndlrs, NULL, &ipmi_mh_user);
	if (err)
	{
		printk("warning: create user fail, watchdog broken (err : %d)\n", err);
		goto ipmi_create_err;
	}

	mutex_init(&ipmi_lock);
	err = ipmi_init_sdr_sensors_data();
	if (err)
	{
		printk("init sensor data fail (err : %d)\n", err);
		goto init_sensor_err;
	}

	err = create_sys_control_attr();
	if (err)
	{
		printk("init sys data fail (err : %d)\n", err);
		goto init_sensor_err;
	}

	return 0;

init_sensor_err:
	kfree(g_sensor_data);
ipmi_create_err:
	hwmon_device_unregister(data->ipmi_hwmon_dev);
hwmon_register_err:
	platform_device_unregister(data->ipmi_platform_dev);
device_reg_err:
	kfree(data);
alloc_err:
	return err;
}

static int32_t __exit quanta_hwmon_ipmi_exit(void)
{
	remove_sensor_attrs();
	hwmon_device_unregister(data->ipmi_hwmon_dev);
	platform_device_unregister(data->ipmi_platform_dev);

	mutex_lock(&ipmi_lock);
	ipmi_destroy_user(ipmi_mh_user);
	mutex_unlock(&ipmi_lock);

	kfree(g_sensor_data);
	kfree(data);

	device_unregister(quanta_drv_dev);
	if (quanta_drv_data) kfree(quanta_drv_data);
	class_destroy(quanta_class);

	return 0;
}

module_init(quanta_hwmon_ipmi_init);
module_exit(quanta_hwmon_ipmi_exit);

MODULE_AUTHOR("Charlie Hsu");
MODULE_VERSION("1.1");
MODULE_DESCRIPTION("Quanta BMC hardware monitor driver");
MODULE_LICENSE("GPL");