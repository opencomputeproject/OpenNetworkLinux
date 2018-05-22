/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *        Copyright 2014, 2015 Big Switch Networks, Inc.
 *
 * Licensed under the Eclipse Public License, Version 1.0 (the
 * "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 *
 *        http://www.eclipse.org/legal/epl-v10.html
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific
 * language governing permissions and limitations under the
 * License.
 *
 * </bsn.cl>
 ************************************************************
 *
 * SFF 8472 Definitions.
 *
 ***********************************************************/
#ifndef __SFF_8472_H__
#define __SFF_8472_H__

#include <sff/sff_config.h>
#include <stdint.h>

/******************************
 *
 * All (most of) the gory details from table 3.5 of the SFF spec
 *
 ******************************/

/* identifier byte 0x00 */

#define SFF8472_IDENT_UNKNOWN   0x00
#define SFF8472_IDENT_GBIC      0x01
#define SFF8472_IDENT_BASEBOARD 0x02
#define SFF8472_IDENT_SFP       0x03
#define SFF8472_IDENT_XBI       0x04
#define SFF8472_IDENT_XENPAK    0x05
#define SFF8472_IDENT_XFP       0x06
#define SFF8472_IDENT_XFF       0x07
#define SFF8472_IDENT_XFPE      0x08
#define SFF8472_IDENT_XPAK      0x09
#define SFF8472_IDENT_X2        0x0A
#define SFF8472_IDENT_DWDM_SFP  0x0B
#define SFF8472_IDENT_QSFP      0x0C

#define SFF8472_MODULE_SFP(idprom)              \
    ((idprom[0] == SFF8472_IDENT_SFP)           \
     || (idprom[0] == SFF8472_IDENT_DWDM_SFP))

/* connector byte 0x02 */

#define SFF8472_CONN_UNKNOWN     0x00
#define SFF8472_CONN_SC          0x01
#define SFF8472_CONN_FC1_CU      0x02
#define SFF8472_CONN_FC2_CU      0x03
#define SFF8472_CONN_BNC         0x04
#define SFF8472_CONN_FC_COAX     0x05
#define SFF8472_CONN_FJ          0x06
#define SFF8472_CONN_LC          0x07
#define SFF8472_CONN_MT_RJ       0x08
#define SFF8472_CONN_MU          0x09
#define SFF8472_CONN_SG          0x0A
#define SFF8472_CONN_SI_PIGTAIL  0x0B
#define SFF8472_CONN_MPO         0x0C
#define SFF8472_CONN_HSSDC_II    0x20
#define SFF8472_CONN_CU_PIGTAIL  0x21
#define SFF8472_CONN_RJ45        0x22
#define SFF8472_CONN_NOSEP       0x23

/* module compliance codes (SFP type) */

#define SFF8472_CC3_INF_1X_CU_PASSIVE     0x01
#define SFF8472_CC3_INF_1X_CU_ACTIVE      0x02
#define SFF8472_CC3_INF_1X_LX             0x04
#define SFF8472_CC3_INF_1X_SX             0x08
#define SFF8472_CC3_XGE_BASE_SR           0x10
#define SFF8472_CC3_XGE_BASE_LR           0x20
#define SFF8472_CC3_XGE_BASE_LRM          0x40
#define SFF8472_CC3_XGE_BASE_ER           0x80

#define SFF8472_CC4_OC48_SHORT            0x01
#define SFF8472_CC4_OC48_INTERMEDIATE     0x02
#define SFF8472_CC4_OC48_LONG             0x04
#define SFF8472_CC4_SONET                 0x18
#define SFF8472_CC4_OC192_SHORT           0x20
#define SFF8472_CC4_ESCON_SMF             0x40
#define SFF8472_CC4_ESCON_MMF             0x80

#define SFF8472_CC5_OC3_SHORT             0x01
#define SFF8472_CC5_OC3_SMM_INTERMEDIATE  0x02
#define SFF8472_CC5_OC3_SMM_LONG          0x04
#define SFF8472_CC5_UNALLOCATED1          0x08
#define SFF8472_CC5_OC12_SHORT            0x10
#define SFF8472_CC5_OC12_SMM_INTERMEDIATE 0x20
#define SFF8472_CC5_OC12_SMM_LONG         0x40
#define SFF8472_CC5_UNALLOCATED2          0x80

#define SFF8472_CC6_GBE_BASE_SX           0x01
#define SFF8472_CC6_GBE_BASE_LX           0x02
#define SFF8472_CC6_GBE_BASE_CX           0x04
#define SFF8472_CC6_GBE_BASE_T            0x08
#define SFF8472_CC6_CBE_BASE_LX           0x10
#define SFF8472_CC6_CBE_BASE_FX           0x20
#define SFF8472_CC6_BASE_BX10             0x40
#define SFF8472_CC6_BASE_PX               0x80

#define SFF8472_CC7_FC_TECH_EL            0x01
#define SFF8472_CC7_FC_TECH_LC            0x02
#define SFF8472_CC7_FC_TECH_SA            0x04
#define SFF8472_CC7_FC_MEDIUM             0x08
#define SFF8472_CC7_FC_LONG               0x10
#define SFF8472_CC7_FC_INTERMEDIATE       0x20
#define SFF8472_CC7_FC_SHORT              0x40
#define SFF8472_CC7_FC_VERY_LONG          0x80

#define SFF8472_CC8_UNALLOCATED1          0x03
#define SFF8472_CC8_SFP_PASSIVE           0x04
#define SFF8472_CC8_SFP_ACTIVE            0x08
#define SFF8472_CC8_FC_TECH_LL            0x10
#define SFF8472_CC8_FC_TECH_SL            0x20
#define SFF8472_CC8_FC_TECH_SN            0x40
#define SFF8472_CC8_FC_TECH_EL            0x80

#define SFF8472_CC9_FC_MEDIA_SM           0x01
#define SFF8472_CC9_UNALLOCATED1          0x02
#define SFF8472_CC9_FC_MEDIA_M5           0x04
#define SFF8472_CC9_FC_MEDIA_M6           0x08
#define SFF8472_CC9_FC_MEDIA_TV           0x10
#define SFF8472_CC9_FC_MEDIA_MI           0x20
#define SFF8472_CC9_FC_MEDIA_TP           0x40
#define SFF8472_CC9_FC_MEDIA_TW           0x80

#define SFF8472_CC10_FC_SPEED_100         0x01
#define SFF8472_CC10_UNALLOCATED1         0x02
#define SFF8472_CC10_FC_SPEED_200         0x04
#define SFF8472_CC10_FC_SPEED_3200        0x08
#define SFF8472_CC10_FC_SPEED_400         0x10
#define SFF8472_CC10_FC_SPEED_1600        0x20
#define SFF8472_CC10_FC_SPEED_800         0x40
#define SFF8472_CC10_FC_SPEED_1200        0x80

#define SFF8472_CC36_XGE_UNALLOCATED      0x01
#define SFF8472_CC36_UNALLOCATED1         0xF7
#define SFF8472_CC36_100G_25G_SR          0x02
#define SFF8472_CC36_100G_25G_LR          0x03
#define SFF8472_CC36_100G_25G_AOC_1       0x01
#define SFF8472_CC36_100G_25G_AOC_2       0x18

#define SFF8471_CC60_FC_PI_4_LIMITING     0x08
#define SFF8471_CC60_SFF8431_LIMITING     0x04
#define SFF8471_CC60_FC_PI_4              0x02
#define SFF8471_CC60_SFF8431              0x01

/* diagnostic monitoring type 0x92 */
#define SFF8472_DOM_IMPL                  0x40
#define SFF8472_DOM_EXTCAL                0x10
#define SFF8472_DOM_RXPWRTYPE             0x08

#define SFF8472_DOM_SUPPORTED(idprom)          \
    ((idprom[92] & SFF8472_DOM_IMPL)? 1:0)
#define SFF8472_DOM_USE_EXTCAL(idprom)         \
    ((idprom[92] & SFF8472_DOM_EXTCAL)? 1:0)
#define SFF8472_DOM_GET_RXPWR_TYPE(idprom)     \
    ((idprom[92] & SFF8472_DOM_RXPWRTYPE)? 1:0)

/* SFF8472 A2 registers */

/* 32-bit little-endian calibration constants */
#define SFF8472_CAL_RXPWR4                 56
#define SFF8472_CAL_RXPWR3                 60
#define SFF8472_CAL_RXPWR2                 64
#define SFF8472_CAL_RXPWR1                 68
#define SFF8472_CAL_RXPWR0                 72

/* 16-bit little endian calibration constants */
#define SFF8472_CAL_TXI_SLP                76
#define SFF8472_CAL_TXI_OFF                78
#define SFF8472_CAL_TXPWR_SLP              80
#define SFF8472_CAL_TXPWR_OFF              82
#define SFF8472_CAL_T_SLP                  84
#define SFF8472_CAL_T_OFF                  86
#define SFF8472_CAL_V_SLP                  88
#define SFF8472_CAL_V_OFF                  90

#define SFF8472_RX_PWR(dom)                \
    (dom[104] << 8 | dom[104 + 1])
#define SFF8472_BIAS_CUR(dom)              \
    (dom[100] << 8 | dom[100 + 1])
#define SFF8472_SFP_VOLT(dom)              \
    (dom[98] << 8 | dom[98 + 1])
#define SFF8472_TX_PWR(dom)                \
    (dom[102] << 8 | dom[102 + 1])
#define SFF8472_SFP_TEMP(dom)              \
    (dom[96] << 8 | dom[96 + 1])

#define SFF8472_MEDIA_XGE_SR(idprom)            \
    ((idprom[3] & SFF8472_CC3_XGE_BASE_SR) != 0)
#define SFF8472_MEDIA_XGE_LR(idprom)            \
    ((idprom[3] & SFF8472_CC3_XGE_BASE_LR) != 0)
#define SFF8472_MEDIA_XGE_LRM(idprom)           \
    ((idprom[3] & SFF8472_CC3_XGE_BASE_LRM) != 0)
#define SFF8472_MEDIA_XGE_ER(idprom)            \
    ((idprom[3] & SFF8472_CC3_XGE_BASE_ER) != 0)

/*
 * some CR cables identify as infiniband copper
 * some CR cables identify as FC twinax
 * some CR cables identify their electrical compliance
 * using bytes 60,61
 * some CR cables identify as FC electrical intra-
 * or inter-enclosure (bytes 7, 8)
 */

static inline int
_sff8472_inf_1x(const uint8_t* idprom)
{
    if ((idprom[3] & SFF8472_CC3_INF_1X_CU_PASSIVE) != 0) return 1;
    if ((idprom[3] & SFF8472_CC3_INF_1X_CU_ACTIVE) != 0) return 1;
    if ((idprom[3] & SFF8472_CC3_INF_1X_LX) != 0) return 1;
    if ((idprom[3] & SFF8472_CC3_INF_1X_SX) != 0) return 1;
    return 0;
}

static inline int
_sff8472_inf_1x_cu_active(const uint8_t* idprom)
{
    if ((idprom[3] & SFF8472_CC3_INF_1X_CU_PASSIVE) != 0) return 0;
    if ((idprom[3] & SFF8472_CC3_INF_1X_LX) != 0) return 0;
    if ((idprom[3] & SFF8472_CC3_INF_1X_SX) != 0) return 0;

    if ((idprom[3] & SFF8472_CC3_INF_1X_CU_ACTIVE) != 0) return 1;

    return 0;
}

static inline int
_sff8472_inf_1x_cu_passive(const uint8_t* idprom)
{
    if ((idprom[3] & SFF8472_CC3_INF_1X_CU_ACTIVE) != 0) return 0;
    if ((idprom[3] & SFF8472_CC3_INF_1X_LX) != 0) return 0;
    if ((idprom[3] & SFF8472_CC3_INF_1X_SX) != 0) return 0;

    if ((idprom[3] & SFF8472_CC3_INF_1X_CU_PASSIVE) != 0) return 1;

    return 0;
}

static inline int
_sff8472_fc_media(const uint8_t* idprom)
{
    if ((idprom[9] & SFF8472_CC9_FC_MEDIA_SM) != 0) return 1;
    if ((idprom[9] & SFF8472_CC9_FC_MEDIA_M5) != 0) return 1;
    if ((idprom[9] & SFF8472_CC9_FC_MEDIA_M6) != 0) return 1;
    if ((idprom[9] & SFF8472_CC9_FC_MEDIA_TV) != 0) return 1;
    if ((idprom[9] & SFF8472_CC9_FC_MEDIA_MI) != 0) return 1;
    if ((idprom[9] & SFF8472_CC9_FC_MEDIA_TP) != 0) return 1;
    if ((idprom[9] & SFF8472_CC9_FC_MEDIA_TW) != 0) return 1;
    return 0;
}

static inline int
_sff8472_fc_media_tw(const uint8_t* idprom)
{
    if ((idprom[9] & SFF8472_CC9_FC_MEDIA_SM) != 0) return 0;
    if ((idprom[9] & SFF8472_CC9_FC_MEDIA_M5) != 0) return 0;
    if ((idprom[9] & SFF8472_CC9_FC_MEDIA_M6) != 0) return 0;
    if ((idprom[9] & SFF8472_CC9_FC_MEDIA_TV) != 0) return 0;
    if ((idprom[9] & SFF8472_CC9_FC_MEDIA_MI) != 0) return 0;
    if ((idprom[9] & SFF8472_CC9_FC_MEDIA_TP) != 0) return 0;

    if ((idprom[9] & SFF8472_CC9_FC_MEDIA_TW) != 0) return 1;

    return 0;
}

static inline int
_sff8472_fc_media_sm(const uint8_t* idprom)
{
    if ((idprom[9] & SFF8472_CC9_FC_MEDIA_M5) != 0) return 0;
    if ((idprom[9] & SFF8472_CC9_FC_MEDIA_M6) != 0) return 0;
    if ((idprom[9] & SFF8472_CC9_FC_MEDIA_TV) != 0) return 0;
    if ((idprom[9] & SFF8472_CC9_FC_MEDIA_MI) != 0) return 0;
    if ((idprom[9] & SFF8472_CC9_FC_MEDIA_TP) != 0) return 0;
    if ((idprom[9] & SFF8472_CC9_FC_MEDIA_TW) != 0) return 0;

    if ((idprom[9] & SFF8472_CC9_FC_MEDIA_SM) != 0) return 1;
    return 0;
}

static inline int
_sff8472_fc_media_mm(const uint8_t* idprom)
{
    if ((idprom[9] & SFF8472_CC9_FC_MEDIA_SM) != 0) return 0;
    if ((idprom[9] & SFF8472_CC9_FC_MEDIA_TV) != 0) return 0;
    if ((idprom[9] & SFF8472_CC9_FC_MEDIA_MI) != 0) return 0;
    if ((idprom[9] & SFF8472_CC9_FC_MEDIA_TP) != 0) return 0;
    if ((idprom[9] & SFF8472_CC9_FC_MEDIA_TW) != 0) return 0;

    if (((idprom[9] & SFF8472_CC9_FC_MEDIA_M5) != 0)
        && ((idprom[9] & SFF8472_CC9_FC_MEDIA_M6) == 0)) return 1;
    if (((idprom[9] & SFF8472_CC9_FC_MEDIA_M5) == 0)
        && ((idprom[9] & SFF8472_CC9_FC_MEDIA_M6) != 0)) return 1;

    return 0;
}

static inline int
_sff8472_tech_fc(const uint8_t* idprom)
{
    if ((idprom[7] & SFF8472_CC7_FC_TECH_EL) != 0) return 1;
    if ((idprom[7] & SFF8472_CC7_FC_TECH_LC) != 0) return 1;
    if ((idprom[7] & SFF8472_CC7_FC_TECH_SA) != 0) return 1;
    if ((idprom[8] & SFF8472_CC8_FC_TECH_EL) != 0) return 1;
    if ((idprom[8] & SFF8472_CC8_FC_TECH_LL) != 0) return 1;
    if ((idprom[8] & SFF8472_CC8_FC_TECH_SL) != 0) return 1;
    if ((idprom[8] & SFF8472_CC8_FC_TECH_SN) != 0) return 1;
    return 0;
}

static inline int
_sff8472_tech_fc_el(const uint8_t* idprom)
{
    /* non-EL types */
    if ((idprom[7] & SFF8472_CC7_FC_TECH_LC) != 0) return 0;
    if ((idprom[7] & SFF8472_CC7_FC_TECH_SA) != 0) return 0;
    if ((idprom[8] & SFF8472_CC8_FC_TECH_LL) != 0) return 0;
    if ((idprom[8] & SFF8472_CC8_FC_TECH_SL) != 0) return 0;
    if ((idprom[8] & SFF8472_CC8_FC_TECH_SN) != 0) return 0;

    if (((idprom[7] & SFF8472_CC7_FC_TECH_EL) != 0)
        || ((idprom[8] & SFF8472_CC8_FC_TECH_EL) != 0))
        return 1;

    return 0;
}

static inline int
_sff8472_tech_fc_ll(const uint8_t* idprom)
{
    /* non-LL types */
    if ((idprom[7] & SFF8472_CC7_FC_TECH_EL) != 0) return 0;
    if ((idprom[7] & SFF8472_CC7_FC_TECH_LC) != 0) return 0;
    if ((idprom[7] & SFF8472_CC7_FC_TECH_SA) != 0) return 0;
    if ((idprom[8] & SFF8472_CC8_FC_TECH_SL) != 0) return 0;
    if ((idprom[8] & SFF8472_CC8_FC_TECH_SN) != 0) return 0;
    if ((idprom[8] & SFF8472_CC8_FC_TECH_EL) != 0) return 0;

    if (((idprom[8] & SFF8472_CC8_FC_TECH_LL) != 0))
        return 1;

    return 0;
}

/* do not specify an FC speed code unless you are actually FC */
static inline int
_sff8472_fc_speed_ok(const uint8_t* idprom)
{
    if (_sff8472_tech_fc(idprom)
        && (idprom[10] != 0))
        return 1;
    if (!_sff8472_tech_fc(idprom)
        && (idprom[10] == 0))
        return 1;
    return 0;
}

/*
 * for dual ethernet/fc modules, we can infer the ethernet
 * speed from the max FC speed
 * (like, if the module mi-identifies itself as 10G and 1G
 */
static inline int
_sff8472_fc_speed_1g(const uint8_t *idprom)
{
     /* not FC, cannot infer speed */
     if (!_sff8472_tech_fc(idprom))
          return 0;

     /* 100MByte and 200MByte should be sufficient to identify as 1G */
     if (((idprom[10] & SFF8472_CC10_FC_SPEED_100) != 0)
         || ((idprom[10] & SFF8472_CC10_FC_SPEED_200) != 0))
          return 1;

     return 0;
}

static inline int
_sff8472_fc_speed_10g(const uint8_t *idprom)
{
     /* not FC, cannot infer speed */
     if (!_sff8472_tech_fc(idprom))
          return 0;

     /* check for any speed close to 10G */
     if (((idprom[10] & SFF8472_CC10_FC_SPEED_800) != 0)
         || ((idprom[10] & SFF8472_CC10_FC_SPEED_1200) != 0)
         || ((idprom[10] & SFF8472_CC10_FC_SPEED_1600) != 0)
         || ((idprom[10] & SFF8472_CC10_FC_SPEED_3200) != 0))
          return 1;

     return 0;
}

/*
 * XXX roth
 * some CR cables do not list an SFP+ active/passive bit,
 * but register as active or passive via infiniband instead
 */

static inline int
_sff8472_sfp_plus_passive(const uint8_t* idprom)
{
    if ((idprom[8] & SFF8472_CC8_SFP_PASSIVE) != 0) return 1;

    /* also allow pre-standard cables identifying as infiniband */
    if (_sff8472_inf_1x_cu_passive(idprom)) return 1;

    return 0;
}

static inline int
_sff8472_sfp_plus_active(const uint8_t* idprom)
{
    if ((idprom[8] & SFF8472_CC8_SFP_ACTIVE) != 0) return 1;

    /* also allow pre-standard cables identifying as infiniband */
    if (_sff8472_inf_1x_cu_active(idprom)) return 1;

    return 0;
}

/*
 * XXX roth
 * some pre-standard CR cables report a "wavelength"
 * in byte 60, 61, pretending to be an optical module
 */

#define _SFF8472_WAVELENGTH(idprom)             \
    ((idprom[60] << 8) | idprom[61])

#define _SFF8472_COMPLIANCE_UNSPEC(idprom) \
    ((idprom[60] == 0)                     \
     && (idprom[61] == 0))

static inline int
_sff8472_compliance_passive_fc(const uint8_t* idprom)
{
    if (idprom[61] != 0) return 0;

    int byte = idprom[60];

    if ((byte & SFF8471_CC60_FC_PI_4) == 0) return 0;

    /* no other wavelength-ish bits */
    byte &= ~SFF8471_CC60_FC_PI_4;
    byte &= ~SFF8471_CC60_SFF8431;
    if (byte != 0) return 0;

    return 1;
}

static inline int
_sff8472_compliance_passive_sff(const uint8_t* idprom)
{
    if (idprom[61] != 0) return 0;

    int byte = idprom[60];

    if ((byte & SFF8471_CC60_SFF8431) == 0) return 0;

    /* no other wavelength-ish bits */
    byte &= ~SFF8471_CC60_FC_PI_4;
    byte &= ~SFF8471_CC60_SFF8431;
    if (byte != 0) return 0;

    return 1;
}

static inline int
_sff8472_compliance_active_fc(const uint8_t* idprom)
{
    if (idprom[61] != 0) return 0;

    int byte = idprom[60];

    if (((byte & SFF8471_CC60_FC_PI_4) == 0)
        && ((byte & SFF8471_CC60_FC_PI_4_LIMITING) == 0))
        return 0;

    /* no other wavelength-ish bits */
    byte &= ~SFF8471_CC60_FC_PI_4;
    byte &= ~SFF8471_CC60_FC_PI_4_LIMITING;
    byte &= ~SFF8471_CC60_SFF8431;
    byte &= ~SFF8471_CC60_SFF8431_LIMITING;
    if (byte != 0) return 0;

    if (idprom[61] != 0) return 0;

    return 1;
}

static inline int
_sff8472_compliance_active_sff(const uint8_t* idprom)
{
    int byte = idprom[60];

    if (((byte & SFF8471_CC60_SFF8431) == 0)
        && ((byte & SFF8471_CC60_SFF8431_LIMITING) == 0))
        return 0;

    /* no other wavelength-ish bits */
    byte &= ~SFF8471_CC60_FC_PI_4;
    byte &= ~SFF8471_CC60_FC_PI_4_LIMITING;
    byte &= ~SFF8471_CC60_SFF8431;
    byte &= ~SFF8471_CC60_SFF8431_LIMITING;
    if (byte != 0) return 0;

    if (idprom[61] != 0) return 0;

    return 1;
}

/*
 * Cisco pre-standard CR cables
 */
static inline int
_sff8472_hack_cr(const uint8_t* idprom)
{
    /* vendor is 'OEM' */
    if (SFF_STRNCMP((char*)idprom+20, "OEM ", 4) != 0) return 0;

    /* model reads like 'SFP-H10GB-CU...' */
    if (SFF_STRNCMP((char*)idprom+40, "SFP-H10GB-CU", 12) != 0) return 0;

    /* S/N reads like 'CSC...' */
    if (SFF_STRNCMP((char*)idprom+68, "CSC", 3) != 0) return 0;

    /*
     * congratulations, you have an expensive Cisco
     * pre-standard CR cable.
     */
    return 1;
}

#define SFF8472_MEDIA_GBE_SX(idprom)            \
  ((idprom[6] & SFF8472_CC6_GBE_BASE_SX) != 0)
#define SFF8472_MEDIA_GBE_LX(idprom)            \
  ((idprom[6] & SFF8472_CC6_GBE_BASE_LX) != 0)
#define SFF8472_MEDIA_GBE_CX(idprom)            \
  ((idprom[6] & SFF8472_CC6_GBE_BASE_CX) != 0)
#define SFF8472_MEDIA_GBE_T(idprom)             \
  ((idprom[6] & SFF8472_CC6_GBE_BASE_T) != 0)

#define SFF8472_MEDIA_CBE_LX(idprom)            \
  ((idprom[6] & SFF8472_CC6_CBE_BASE_LX) != 0)
#define SFF8472_MEDIA_CBE_FX(idprom)            \
  ((idprom[6] & SFF8472_CC6_CBE_BASE_FX) != 0)

/*
 * XXX roth
 * ZR Finisar optics don't list any ethernet
 * or FC compliance, but *do* identify as having a high bit rate
 */

static inline int
_sff8472_bitrate_xge(const uint8_t *idprom)
{
    if (idprom[12] == 0)
        return 0;
    if (idprom[12] == 0xFF)
        return 0;
    long long br = (long long) idprom[12] * 100*1000000;
    if (br > 10LL*1000*1000000)
        return 1;
    return 0;
}

static inline int
_sff8472_bitrate_gbe(const uint8_t *idprom)
{
    if (idprom[12] == 0)
        return 0;
    if (idprom[12] == 0xFF)
        return 0;
    long long br = (long long) idprom[12] * 100*1000000;
    if (br > 1LL*1000*1000000 && br < 5LL*1000*1000000)
        return 1;
    return 0;
}

static inline int
_sff8472_length_sm(const uint8_t *idprom)
{
    if ((idprom[14] == 0) && (idprom[15] == 0)) return 0;
    if (idprom[14] != 0)
        return idprom[14] * 1000;
    return idprom[15] * 100;
}

static inline int
_sff8472_length_om2(const uint8_t *idprom)
{
    return idprom[16] * 10;
}

static inline int
_sff8472_length_om1(const uint8_t *idprom)
{
    return idprom[17] * 10;
}

static inline int
_sff8472_length_om4(const uint8_t *idprom)
{
    return idprom[18] * 10;
}

static inline int
_sff8472_length_cu(const uint8_t *idprom)
{
    return idprom[18];
}

static inline int
_sff8472_length_om3(const uint8_t *idprom)
{
    return idprom[19] * 10;
}

/* grab-bag to detect pre-standard CR media */
static inline int
_sff8472_media_cr_passive(const uint8_t* idprom)
{
    int maybe = 0;

    if (_sff8472_inf_1x_cu_passive(idprom))
        maybe = 1;
    else if (_sff8472_inf_1x(idprom))
        return 0;

    if (_sff8472_tech_fc_el(idprom))
        maybe = 1;
    else if (_sff8472_tech_fc(idprom))
        return 0;

    if (idprom[4] != 0) return 0;
    if (idprom[5] != 0) return 0;
    /*
     * XXX roth -- PAN-934 -- Tyco CR cable advertises 1000BASE-CX,
     * presumably because it is a 2-wire cable
     */
    if ((idprom[6] != SFF8472_CC6_GBE_BASE_CX)
        && (idprom[6] != 0))
        return 0;

    if (_sff8472_sfp_plus_passive(idprom))
        maybe = 1;

    if (_sff8472_fc_media_tw(idprom))
        maybe = 1;
    else if (_sff8472_fc_media(idprom))
        return 0;

    if (!_sff8472_fc_speed_ok(idprom))
        return 0;

    /*
     * XXX roth -- PAN-934 -- once we relax the checks here
     * for 1000BASE-CX, we need to make sure we don't accept any
     * 1G cables... make sure the FC speed is specified,
     * and is high enough.
     */
    if ((idprom[6] == SFF8472_CC6_GBE_BASE_CX)
        && !_sff8472_tech_fc(idprom)
        && !_sff8472_bitrate_xge(idprom))
        return 0;
    if ((idprom[6] == SFF8472_CC6_GBE_BASE_CX)
        && _sff8472_tech_fc(idprom)
        && !_sff8472_fc_speed_10g(idprom))
        return 0;

    /*
     * PAN-1233 -- cable identifies as 1000BASE-CX with a high BR
     */
    if ((idprom[6] == SFF8472_CC6_GBE_BASE_CX)
        && (_sff8472_length_cu(idprom) > 0)
        && (_sff8472_bitrate_xge(idprom)))
        maybe = 1;

    if (_sff8472_hack_cr(idprom))
        maybe = 1;

    if (maybe) {
        if (!_sff8472_compliance_passive_fc(idprom)
            && !_sff8472_compliance_passive_sff(idprom)
            && (_SFF8472_WAVELENGTH(idprom) != 850)
            && !_SFF8472_COMPLIANCE_UNSPEC(idprom)
            && !_sff8472_hack_cr(idprom))
            return 0;
    }

    return maybe;
}

static inline int
_sff8472_media_cr_active(const uint8_t* idprom)
{
    int maybe = 0;

    if (_sff8472_inf_1x_cu_active(idprom))
        maybe = 1;
    else if (_sff8472_inf_1x(idprom))
        return 0;

    if (_sff8472_tech_fc_el(idprom))
        maybe = 1;
    else if (_sff8472_tech_fc(idprom))
        return 0;

    if (idprom[4] != 0) return 0;
    if (idprom[5] != 0) return 0;
    /*
     * XXX roth -- PAN-1111 -- 3M CR cable advertises 1000BASE-CX,
     * presumably because it is a 2-wire cable
     */
    if ((idprom[6] != SFF8472_CC6_GBE_BASE_CX)
        && (idprom[6] != 0))
        return 0;

    if (_sff8472_sfp_plus_active(idprom))
        maybe = 1;

    if (_sff8472_fc_media_tw(idprom))
        maybe = 1;
    else if (_sff8472_fc_media(idprom))
        return 0;

    if (!_sff8472_fc_speed_ok(idprom))
        return 0;

    /*
     * XXX roth -- PAN-1111 -- yet another CR cable
     * that advertises as 1000BASE-CX.
     */
    if ((idprom[6] == SFF8472_CC6_GBE_BASE_CX)
        && !_sff8472_tech_fc(idprom))
        return 0;
    if ((idprom[6] == SFF8472_CC6_GBE_BASE_CX)
        && _sff8472_tech_fc(idprom)
        && !_sff8472_fc_speed_10g(idprom))
        return 0;

    if (maybe) {
        if (!_sff8472_compliance_active_fc(idprom)
            && !_sff8472_compliance_active_sff(idprom)
            && !_SFF8472_COMPLIANCE_UNSPEC(idprom))
            return 0;
    }

    return maybe;
}

static inline int
_sff8472_media_zr(const uint8_t *idprom)
{
    if (!SFF8472_MODULE_SFP(idprom)) return 0;

    /* should be LC connector */
    if ((idprom[2] & SFF8472_CONN_LC) == 0) return 0;

    /* do not advertise any other infiniband or ethernet features */
    if (_sff8472_inf_1x(idprom)) return 0;
    if (SFF8472_MEDIA_XGE_SR(idprom)) return 0;
    if (SFF8472_MEDIA_XGE_LR(idprom)) return 0;
    if (SFF8472_MEDIA_XGE_LRM(idprom)) return 0;
    if (SFF8472_MEDIA_XGE_ER(idprom)) return 0;

    /* JSM-01ZWBA1 from JDSU does not have FC field(idprom[7~9] programmed,
     * so check other fields first */
    /* at least 10GbE */
    if (!_sff8472_bitrate_xge(idprom)) return 0;

    /* JSM-01ZWBA1 from JDSU is 80km */
    if (idprom[14] == 80)
        return 1;

    /* advertise media and tech as per FC */
    if (_sff8472_tech_fc_ll(idprom) == 0) return 0;
    if (_sff8472_fc_media_sm(idprom) == 0) return 0;
    /* at least 40km of single-mode */
    if ((idprom[14] > 40) && (idprom[15] == 0xff))
        return 1;

    return 0;

}

static inline int
_sff8472_media_srlite(const uint8_t *idprom)
{
    if (!SFF8472_MODULE_SFP(idprom)) return 0;

    /* should be LC connector */
    if ((idprom[2] & SFF8472_CONN_LC) == 0) return 0;

    /* do not advertise any other infiniband or ethernet features */
    if (_sff8472_inf_1x(idprom)) return 0;
    if (SFF8472_MEDIA_XGE_SR(idprom)) return 0;
    if (SFF8472_MEDIA_XGE_LR(idprom)) return 0;
    if (SFF8472_MEDIA_XGE_LRM(idprom)) return 0;
    if (SFF8472_MEDIA_XGE_ER(idprom)) return 0;

    /* do not advertise FC media */
    if (_sff8472_fc_media(idprom)) return 0;
    if (_sff8472_tech_fc(idprom)) return 0;

    /* at least 10GbE */
    if (!_sff8472_bitrate_xge(idprom)) return 0;

    /* no single-mode fiber length */
    if (idprom[14] != 0) return 0;
    if (idprom[15] != 0) return 0;

    /* some non-zero multi-mode fiber length */
    if (_sff8472_length_sm(idprom)) return 0;
    if (_sff8472_length_om1(idprom)) return 1;
    if (_sff8472_length_om2(idprom)) return 1;
    if (_sff8472_length_om3(idprom)) return 1;
    if (_sff8472_length_om4(idprom)) return 1;
    return 0;

}

/*
 * some modules (e.g. Finisar FTRJ8519P1BNL-B1, 1G Ethernet /
 * 2G fiber) mis-identify as supporting 10G *and* 1G;
 * in this case we do not want to enable 10G mode;
 * we can verify this by looking at the FC speed field
 */

static inline int
_sff8472_media_gbe_sx_fc_hack(const uint8_t *idprom)
{
    if (!SFF8472_MODULE_SFP(idprom)) return 0;

    /* module should report as SX */
    if (!SFF8472_MEDIA_GBE_SX(idprom)) return 0;

    /* module erroneously reports as 10G */
    if (!SFF8472_MEDIA_XGE_SR(idprom)
        || !SFF8472_MEDIA_XGE_LR(idprom)
        || !SFF8472_MEDIA_XGE_LRM(idprom)
        || !SFF8472_MEDIA_XGE_ER(idprom)) return 0;

    /* module reports as 1G FC, but not 10G */
    if (_sff8472_fc_speed_1g(idprom)
        && !_sff8472_fc_speed_10g(idprom))
        return 1;
    else
        return 0;
}

static inline int
_sff8472_media_gbe_lx_fc_hack(const uint8_t *idprom)
{
    if (!SFF8472_MODULE_SFP(idprom)) return 0;

    /* module should report as LX */
    if (!SFF8472_MEDIA_GBE_LX(idprom)) return 0;

    /* module erroneously reports as 10G */
    if (!SFF8472_MEDIA_XGE_SR(idprom)
        || !SFF8472_MEDIA_XGE_LR(idprom)
        || !SFF8472_MEDIA_XGE_LRM(idprom)
        || !SFF8472_MEDIA_XGE_ER(idprom)) return 0;

    /* module reports as 1G FC, but not 10G */
    if (_sff8472_fc_speed_1g(idprom)
        && !_sff8472_fc_speed_10g(idprom))
        return 1;
    else
        return 0;
}

/*
 * Try to identify active SR cables, like breakouts
 */
static inline int
_sff8472_sfp_10g_aoc(const uint8_t *idprom)
{
    /* module should be sfp */
    if (!SFF8472_MODULE_SFP(idprom)) return 0;

    /* should report any normal specs */
    if (_sff8472_fc_media(idprom)) return 0;
    if (_sff8472_tech_fc(idprom)) return 0;

    if (SFF8472_MEDIA_XGE_SR(idprom)) return 0;
    if (SFF8472_MEDIA_XGE_LR(idprom)) return 0;
    if (SFF8472_MEDIA_XGE_LRM(idprom)) return 0;
    if (SFF8472_MEDIA_XGE_ER(idprom)) return 0;

    if (SFF8472_MEDIA_GBE_SX(idprom)) return 0;
    if (SFF8472_MEDIA_GBE_LX(idprom)) return 0;
    if (SFF8472_MEDIA_GBE_CX(idprom)) return 0;
    if (SFF8472_MEDIA_GBE_T(idprom)) return 0;
    if (SFF8472_MEDIA_CBE_LX(idprom)) return 0;
    if (SFF8472_MEDIA_CBE_FX(idprom)) return 0;

    /* should be active */
    if (!_sff8472_sfp_plus_active(idprom)) return 0;

    /* should not report passive cable compliance */
    if (_sff8472_compliance_passive_fc(idprom)) return 0;
    if (_sff8472_compliance_passive_sff(idprom)) return 0;

    /* should report active FC or SFF */
    if (!_sff8472_compliance_active_fc(idprom)
        && !_sff8472_compliance_active_sff(idprom))
        return 0;

    /* nominal BR should be 10g */
    if (!_sff8472_bitrate_xge(idprom)) return 0;

    /* should be a short-ish cable */
    if (_sff8472_length_sm(idprom)) return 0;
    if (_sff8472_length_om1(idprom)) return 0;
    if (_sff8472_length_om2(idprom)) return 0;
    if (_sff8472_length_om3(idprom)) return 0;

    if (!_sff8472_length_cu(idprom)) return 0;
    if (_sff8472_length_cu(idprom) > 30) return 0;

    /* congratulations, probably an active optical cable */
    return 1;
}

/*
 * Infer cable length for fixed-length (AOC) optical cables
 */
static inline int
_sff8472_sfp_10g_aoc_length(const uint8_t *idprom)
{
    /* module should be sfp */
    if (!SFF8472_MODULE_SFP(idprom)) return -1;

    /* does not report a fiber length, but does report a cable length */
    if (_sff8472_length_sm(idprom) > 0) return -1;
    if (_sff8472_length_om1(idprom) > 0) return -1;
    if (_sff8472_length_om2(idprom) > 0) return -1;
    if (_sff8472_length_om3(idprom) > 0) return -1;
    if (_sff8472_length_cu(idprom) > 0)
        return _sff8472_length_cu(idprom);

    return -1;
}

/*
 * SFP28
 */
static inline int
_sff8472_media_sfp28_cr(const uint8_t* idprom)
{
    /* module should be sfp */
    if (!SFF8472_MODULE_SFP(idprom)) return 0;

    if ((idprom[3] & SFF8472_CC3_INF_1X_CU_PASSIVE) == 0) return 0;
    if (idprom[12] == 0xFF) return 1;

    return 0;
}

static inline int
_sff8472_media_sfp28_sr(const uint8_t* idprom)
{
    /* module should be sfp */
    if (!SFF8472_MODULE_SFP(idprom)) return 0;

    if (idprom[12] != 0xFF) return 0;
    if (idprom[36] == SFF8472_CC36_100G_25G_SR) return 1;

    return 0;
}

static inline int
_sff8472_media_sfp28_lr(const uint8_t* idprom)
{
    /* module should be sfp */
    if (!SFF8472_MODULE_SFP(idprom)) return 0;

    if (idprom[12] != 0xFF) return 0;
    if (idprom[36] == SFF8472_CC36_100G_25G_LR) return 1;

    return 0;
}

static inline int
_sff8472_media_sfp28_aoc(const uint8_t* idprom)
{
    /* module should be sfp */
    if (!SFF8472_MODULE_SFP(idprom)) return 0;

    if (idprom[12] != 0xFF) return 0;
    if ((idprom[36] == SFF8472_CC36_100G_25G_AOC_1) ||
         (idprom[36] == SFF8472_CC36_100G_25G_AOC_2)) {
        return 1;
    }

    return 0;
}
#endif
