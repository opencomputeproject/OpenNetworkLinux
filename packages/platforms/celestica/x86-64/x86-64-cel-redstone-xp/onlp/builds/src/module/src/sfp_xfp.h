#ifndef INCLUDE_SFP_XFP
#define INCLUDE_SFP_XFP

/* SFP I2C parameters start here */
/* 0xA0 */
#define SFP_REVISION_ADDR             56
#define SFP_REVISION_SIZE              1
#define SFP_DIAGMODE_ADDR             92
#define SFP_DIAGMODE_SIZE              1
#define SFP_COMPLIANCE_ADDR           94
#define SFP_COMPLIANCE_SIZE            1
#define SFP_COMPLIANCE_NONE            0
#define SFP_OPTIONS_ADDR              65
#define SFP_OPTIONS_SIZE               1
#define SFP_DIAGMODE_EXTERNAL_CALIBRATION_MASK 0x10  /* bit 4 */
#define SFP_TX_FAULT_SUPPORT_MASK   (1<<3)  /* Table 3.7, Options, byte 65, bit 3 */
#define SFP_TX_FAULT_MASK           (1<<2)  /* Table 3.17, A/D Values and Status Bits, byte 110, bit 2 */
#define SFP_LOS_MASK                (1<<1)  /* Table 3.17, A/D Values and Status Bits, byte 110, bit 1 */
#define SFP_RX_POWER_PHANTOM_READ 0

/* 0xA2 */
/* Calibration constants */
#define SFP_RX_PWR4_ADDR              56
#define SFP_RX_PWR4_SIZE               4
#define SFP_RX_PWR3_ADDR              60
#define SFP_RX_PWR3_SIZE               4
#define SFP_RX_PWR2_ADDR              64
#define SFP_RX_PWR2_SIZE               4
#define SFP_RX_PWR1_ADDR              68
#define SFP_RX_PWR1_SIZE               4
#define SFP_RX_PWR0_ADDR              72
#define SFP_RX_PWR0_SIZE               4
#define SFP_TX_CURRENT_SLOPE_ADDR     76
#define SFP_TX_CURRENT_SLOPE_SIZE      2
#define SFP_TX_CURRENT_OFFSET_ADDR    78
#define SFP_TX_CURRENT_OFFSET_SIZE     2
#define SFP_TX_POWER_SLOPE_ADDR       80
#define SFP_TX_POWER_SLOPE_SIZE        2
#define SFP_TX_POWER_OFFSET_ADDR      82
#define SFP_TX_POWER_OFFSET_SIZE       2
#define SFP_T_SLOPE_ADDR              84
#define SFP_T_SLOPE_SIZE               2
#define SFP_T_OFFSET_ADDR             86
#define SFP_T_OFFSET_SIZE              2
#define SFP_V_SLOPE_ADDR              88
#define SFP_V_SLOPE_SIZE               2
#define SFP_V_OFFSET_ADDR             90
#define SFP_V_OFFSET_SIZE              2

/* 0xAC */
/* Registers (0-31) */
#define SFP_COPPER_PHY_CNTRL             0  /* Control Register */
#define SFP_COPPER_PHY_STATUS            1  /* Status Register */
#define SFP_COPPER_PHY_ID0               2  /* PHY Identifier  */
#define SFP_COPPER_PHY_ID1               3  /* PHY Identifier  */
#define SFP_COPPER_PHY_SPEC_STATUS      17  /* PHY Specific Status Register */
#define SFP_COPPER_PHY_EXT_ADDR         22  /* Extended Address for Cable Diagnostic Reg */
#define SFP_COPPER_PHY_CABLE_DIAG       28  /* GE interface Cable Diagnostic Registers */
#define SFP_COPPER_PHY_REG29            29  /* Reg. 29 */
#define SFP_COPPER_PHY_REG30            30  /* Reg. 30 */
#define SFP_COPPER_PHY_REG31            31  /* Reg. 31 */

#define SFP_COPPER_PHY_OUI              0x0141
#define SFP_COPPER_PHY_MODEL_MASK       0x03F0

#define SFP_IF_ERROR_RETURN(op)  do { int __rc__; if ((__rc__ = (op)) < 0) return(L7_FAILURE); } while(0) 

/*********************************************************************
* In addition to the hardware specs, please see Application Note AN-2030
* available from FINISAR.
*********************************************************************/
#define SFP_XFP_FINISAR_OUI     0x009065
#define SFP_XFP_TYPE_ADDR              0
#define SFP_XFP_TYPE_SFP            0x03
#define SFP_XFP_TYPE_SFP_PLUS       0x03
#define SFP_XFP_TYPE_XFP            0x06
#define SFP_XFP_TEMPERATURE_ADDR      96
#define SFP_XFP_TEMPERATURE_SIZE       2
#define SFP_XFP_VOLTAGE_ADDR          98
#define SFP_XFP_VOLTAGE_SIZE           2
#define SFP_XFP_CURRENT_ADDR         100
#define SFP_XFP_CURRENT_SIZE           2
#define SFP_XFP_TX_POWER_ADDR        102
#define SFP_XFP_TX_POWER_SIZE          2
#define SFP_XFP_RX_POWER_ADDR        104
#define SFP_XFP_RX_POWER_SIZE          2
#define SFP_XFP_LOS_ADDR             110
#define SFP_XFP_LOS_SIZE               1

#define SFP_VENDOR_NAME_ADDR          20
#define SFP_VENDOR_OUI_ADDR           37
#define SFP_VENDOR_OUI_SIZE            3
#define SFP_VENDOR_PN_ADDR            40
#define SFP_VENDOR_REV_ADDR           56 
#define SFP_VENDOR_SN_ADDR            68
#define SFP_VENDOR_DATE_ADDR          84
#define SFP_VENDOR_DATE_SIZE           8
#define SFP_NOMINAL_SIG_RATE_ADDR     12
#define SFP_NOMINAL_SIG_RATE_SIZE      1
#define SFP_LINK_LENGTH_50UM_ADDR     16
#define SFP_LINK_LENGTH_50UM_SIZE      1
#define SFP_LINK_LENGTH_62_5UM_ADDR   17
#define SFP_LINK_LENGTH_62_5UM_SIZE    1
#define SFP_WAVELENGTH_ADDR           60
#define SFP_WAVELENGTH_SIZE            2
#define SFP_DAC_LENGTH_ADDR           18
#define SFP_DAC_LENGTH_SIZE            1


#define QSFP_ADDRESS_DELTA            128
#define QSFP_TEMPERATURE_ADDR          22
#define QSFP_VOLTAGE_ADDR              26
#define QSFP_TX_FAULT_ADDR              4
#define QSFP_LOS_ADDR                   3
#define QSFP_RX_POWER_ADDR             34
#define QSFP_TX_POWER_ADDR             42
#define QSFP_VENDOR_NAME_ADDR         148
#define QSFP_VENDOR_NAME_SIZE          16
#define QSFP_VENDOR_PN_ADDR           168
#define QSFP_VENDOR_PN_SIZE            16
#define QSFP_VENDOR_SN_ADDR           196
#define QSFP_VENDOR_SN_SIZE            16
#define QSFP_VENDOR_REV_ADDR          184 
#define QSFP_VENDOR_REV_SIZE            2
#define QSFP_NOMINAL_SIG_RATE_ADDR    140
#define QSFP_NOMINAL_SIG_RATE_SIZE      1
#define QSFP_LINK_LENGTH_50UM_ADDR    144
#define QSFP_LINK_LENGTH_50UM_SIZE      1
#define QSFP_LINK_LENGTH_62_5UM_ADDR  145
#define QSFP_LINK_LENGTH_62_5UM_SIZE    1
#define QSFP_WAVELENGTH_ADDR          186
#define QSFP_WAVELENGTH_SIZE            2
#define QSFP_DAC_LENGTH_ADDR          146
#define QSFP_DAC_LENGTH_SIZE            1
#define QSFP_ETHERNET_COMPLIANCE_ADDR 131
#define QSFP_ETHERNET_COMPLIANCE_SIZE   1

/*
 *  SFF Committee
 *  SFF-8436 Specification
 *  QSFP+ 10 Gbs 4X PLUGGABLE TRANSCEIVER
 *  Rev 4.5 January 7, 2013
 *
 *  Table 33, Address 131 Bits
 */
#define QSFP_ETHERNET_COMPLIANCE_RESERVED	(1 << 7)
#define QSFP_ETHERNET_COMPLIANCE_10GBASE_LRM	(1 << 6)
#define QSFP_ETHERNET_COMPLIANCE_10GBASE_LR	(1 << 5)
#define QSFP_ETHERNET_COMPLIANCE_10GBASE_SR	(1 << 4)
#define QSFP_ETHERNET_COMPLIANCE_40GBASE_CR4	(1 << 3)
#define QSFP_ETHERNET_COMPLIANCE_40GBASE_SR4	(1 << 2)
#define QSFP_ETHERNET_COMPLIANCE_40GBASE_LR4	(1 << 1)
#define QSFP_ETHERNET_COMPLIANCE_40GACTIVE	(1 << 0)
/*
SFF Committee
INF-8077i
10 Gigabit Small Form Factor Pluggable Module
Revision 4.5 August 31, 2005
*/

#define XFP_AUX1_ADDR                106
#define XFP_AUX2_ADDR                108
#define XFP_AUX_CONFIG               222
#define XFP_AUX_SIZE                   2
#define XFP_PAGE_SELECT              127
/* from table 59 of INF-8077i */
#define XFP_AUX_INPUT_NONE             0
#define XFP_AUX_INPUT_BIAS_VOLTAGE     1
#define XFP_AUX_INPUT_TEC_CURRENT      3
#define XFP_AUX_INPUT_LASER_TEMP       4
#define XFP_AUX_INPUT_LASER_WAVE       5
#define XFP_AUX_INPUT_5_VOLTAGE        6
#define XFP_AUX_INPUT_3_3_VOLTAGE      7
#define XFP_AUX_INPUT_1_8_VOLTAGE      8
#define XFP_AUX_INPUT_MASK           0xF
#define XFP_FAULT_ADDR               111
#define XFP_FAULT_SIZE                 1
#define XFP_TX_FAULT_MASK         (1<<6)  /* Table 42 General Control/Status Bits, byte 111, bit 6 */

#define XFP_VENDOR_NAME_ADDR         148
#define XFP_VENDOR_NAME_SIZE          15
#define XFP_VENDOR_PN_ADDR           168
#define XFP_VENDOR_PN_SIZE            16
#define XFP_VENDOR_SN_ADDR           196
#define XFP_VENDOR_SN_SIZE            16
#define XFP_VENDOR_DATE_ADDR         212
#define XFP_VENDOR_DATE_SIZE           8
#define XFP_VENDOR_OUI_ADDR          165
#define XFP_VENDOR_OUI_SIZE            3
#define XFP_VENDOR_REV_ADDR          184 
#define XFP_VENDOR_REV_SIZE            2

/* Validation of the modules in the hpcIsValidXxx() routine       */
/* Controls wheter the hpcIsValidXxx() actually tries to read the */
/* xFP signature from the pluggable module.                       */
#define     SFP_VALIDATE_MODULE           0
#define     XFP_VALIDATE_MODULE           1
#define     SFP_SFPPLUS_VALIDATE_MODULE   1


#define SFF8472_ID_ADDR                   0
#define SFF8472_EXT_ID_ADDR               1
#define SFF8472_10G_COMPLIANCE_ADDR       3
#define SFF8472_ETHERNET_COMPLIANCE_ADDR  6
#define SFF8472_SFPPLUS_CABLE_TECH_ADDR   8
#define SFF8472_BIT_RATE_ADDR             12


#define SFF8472_DIAG_MON_TYPE_ADDR        92
#define SFF8472_DIAG_MON_TYPE_SIZE         1
#define SFF8472_COMPLIANCE_ADDR           94
#define SFF8472_COMPLIANCE_SIZE            1

#define SFF8436_COMPLIANCE_ADDR          131
#define SFF8436_COMPLIANCE_SIZE            1

#define SFF8436_40G_ACTIVE               (1 << 0)
#define SFF8436_40G_BASE_LR4             (1 << 1)
#define SFF8436_40G_BASE_SR4             (1 << 2)
#define SFF8436_40G_BASE_CR4             (1 << 3)
#define SFF8436_10G_BASE_SR              (1 << 4)
#define SFF8436_10G_BASE_LR              (1 << 5)
#define SFF8436_10G_BASE_LRM             (1 << 6)


#define SFF8472_COMPLIANCE_NONE          (0x0)
#define SFF8472_DIGITAL_DIAG_IMPLEMENTED (0x1 << 6)

#define SFF8472_SFP_ID       (0x03)
#define SFF8472_SFP_EXT_ID   (0x04)
#define SFF8472_XFP_ID       (0x06)
#define SFF8436_QSFP_PLUS_ID (0x0d)

#define SFF8472_10G_BASE_ER  (1<<7)
#define SFF8472_10G_BASE_LRM (1<<6)
#define SFF8472_10G_BASE_LR  (1<<5)
#define SFF8472_10G_BASE_SR  (1<<4)

#define SFF8472_10G_BASE     (SFF8472_10G_BASE_ER  | \
                              SFF8472_10G_BASE_LRM | \
                              SFF8472_10G_BASE_LR  | \
                              SFF8472_10G_BASE_SR )

#define SFF8472_10G_SPEED    (0x64)

#define SFF8472_100M_BASE_FX  (1 << 5)
#define SFF8472_100M_BASE_LX  (1 << 4)
#define SFF8472_1G_BASE_T    (1<<3)
#define SFF8472_1G_BASE_CX   (1<<2)
#define SFF8472_1G_BASE_LX   (1<<1)
#define SFF8472_1G_BASE_SX      (1)

#define SFF8472_1G_BASE      (SFF8472_1G_BASE_T  | \
                              SFF8472_1G_BASE_CX | \
                              SFF8472_1G_BASE_LX | \
                              SFF8472_1G_BASE_SX )

#define SFF8472_100M_BASE     (SFF8472_100M_BASE_FX | SFF8472_100M_BASE_LX)

#define SFF8431_COMPLIANCE            (1<<0)
#define SFF8431_LIMITED_COMPLIANCE    (1<<2)
#define SFF8431_APPENDIX_E_COMPLIANCE (1<<0)

#define SFPPLUS_ACTIVE_CABLE    (1<<3)
#define SFPPLUS_PASSIVE_CABLE   (1<<2)

#define SFPPLUS_CABLE_TECH      (SFPPLUS_ACTIVE_CABLE  | \
                                 SFPPLUS_PASSIVE_CABLE )


#define TRANSCEIVER_NOT_PRESENT  (0)
#define TRANSCEIVER_TYPE_100M    (1)
#define TRANSCEIVER_TYPE_1G      (2)
#define TRANSCEIVER_TYPE_10G     (3)
#define TRANSCEIVER_TYPE_QSFP    (4)
#define TRANSCEIVER_TYPE_XFP     (5)
#define TRANSCEIVER_TYPE_INVALID (6)

#if L7_FEAT_SFP_QUALIFICATION
extern L7_RC_t hpcSfpOpticQualify(L7_uint32 slot, L7_uint32 port);
#define HPC_SFP_OPTICS_QUALIFY(slot,port) hpcSfpOpticQualify(slot,port)
#else
#define HPC_SFP_OPTICS_QUALIFY(slot,port)   L7_NOT_SUPPORTED
#endif

#define HPC_SFP_CUST_WAVELENGTH_READ(slot, port, wavelength)  L7_NOT_SUPPORTED
#define HPC_SFP_CUST_DATARATE_READ(slot, port, dataRate)      L7_NOT_SUPPORTED
#define HPC_SFP_CUST_OPTIC_MEDIA_TYPE_GET(slot, port, opticType)   L7_NOT_SUPPORTED

#ifdef INCLUDE_SFP_XFP_OVERRIDES
#include "sfp_xfp_overrides.h"
#endif

#endif /* INCLUDE_SFP_XFP */
