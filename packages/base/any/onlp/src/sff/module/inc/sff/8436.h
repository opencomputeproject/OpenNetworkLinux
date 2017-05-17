/**********************************************************************
 *
 * 8436.h
 *
 * idprom defintions for QSFP+ modules.
 *
 **********************************************************************/

#ifndef __SFF_8436_H__
#define __SFF_8436_H__

#include <sff/sff_config.h>
#include "sff/8472.h"

/* identifier, bytes 128 page 0 (also byte 0) */

#define SFF8436_IDENT_UNKNOWN            SFF8472_IDENT_UNKNOWN
#define SFF8436_IDENT_GBIC               SFF8472_IDENT_GBIC
#define SFF8436_IDENT_BASEBOARD          SFF8472_IDENT_BASEBOARD
#define SFF8436_IDENT_SFP                SFF8472_IDENT_SFP
#define SFF8436_IDENT_XBI                SFF8472_IDENT_XBI
#define SFF8436_IDENT_XENPAK             SFF8472_IDENT_XENPAK
#define SFF8436_IDENT_XFP                SFF8472_IDENT_XFP
#define SFF8436_IDENT_XFF                SFF8472_IDENT_XFF
#define SFF8436_IDENT_XFPE               SFF8472_IDENT_XFPE
#define SFF8436_IDENT_XPAK               SFF8472_IDENT_XPAK
#define SFF8436_IDENT_X2                 SFF8472_IDENT_X2
#define SFF8436_IDENT_DWDM_SFP           SFF8472_IDENT_DWDM_SFP
#define SFF8436_IDENT_QSFP               SFF8472_IDENT_QSFP

/* additional values from this spec */
#define SFF8436_IDENT_QSFP_PLUS          0x0D

#define SFF8436_MODULE_QSFP_PLUS(idprom)        \
  ((idprom[0] == SFF8436_IDENT_QSFP)            \
   || (idprom[0] == SFF8436_IDENT_QSFP_PLUS))

#define SFF8436_MODULE_QSFP_PLUS_V2(idprom)     \
  ((idprom[128] == SFF8436_IDENT_QSFP)          \
   || (idprom[128] == SFF8436_IDENT_QSFP_PLUS))

#define SFF8436_RX1_PWR(idprom)                \
    (idprom[34] << 8 | idprom[34 + 1])
#define SFF8436_RX2_PWR(idprom)                \
    (idprom[36] << 8 | idprom[36 + 1])
#define SFF8436_RX3_PWR(idprom)                \
    (idprom[38] << 8 | idprom[38 + 1])
#define SFF8436_RX4_PWR(idprom)                \
    (idprom[40] << 8 | idprom[40 + 1])

#define SFF8436_TX1_BIAS(idprom)              \
    (idprom[42] << 8 | idprom[42 + 1])
#define SFF8436_TX2_BIAS(idprom)              \
    (idprom[44] << 8 | idprom[44 + 1])
#define SFF8436_TX3_BIAS(idprom)              \
    (idprom[46] << 8 | idprom[46 + 1])
#define SFF8436_TX4_BIAS(idprom)              \
    (idprom[48] << 8 | idprom[48 + 1])

#define SFF8436_SFP_VOLT(idprom)              \
    (idprom[26] << 8 | idprom[26 + 1])
#define SFF8436_SFP_TEMP(idprom)              \
    (idprom[22] << 8 | idprom[22 + 1])

/* connector value, byte 130 page 0 */

#define SFF8436_CONN_UNKNOWN             SFF8472_CONN_UNKNOWN
#define SFF8436_CONN_SC                  SFF8472_CONN_SC
#define SFF8436_CONN_FC1_CU              SFF8472_CONN_FC1_CU
#define SFF8436_CONN_FC2_CU              SFF8472_CONN_FC2_CU
#define SFF8436_CONN_BNC                 SFF8472_CONN_BNC
#define SFF8436_CONN_FC_COAX             SFF8472_CONN_FC_COAX
#define SFF8436_CONN_FJ                  SFF8472_CONN_FJ
#define SFF8436_CONN_LC                  SFF8472_CONN_LC
#define SFF8436_CONN_MT_RJ               SFF8472_CONN_MT_RJ
#define SFF8436_CONN_MU                  SFF8472_CONN_MU
#define SFF8436_CONN_SG                  SFF8472_CONN_SG
#define SFF8436_CONN_SI_PIGTAIL          SFF8472_CONN_SI_PIGTAIL
#define SFF8436_CONN_MPO                 SFF8472_CONN_MPO
#define SFF8436_CONN_HSSDC_II            SFF8472_CONN_HSSDC_II
#define SFF8436_CONN_CU_PIGTAIL          SFF8472_CONN_CU_PIGTAIL
#define SFF8436_CONN_RJ45                SFF8472_CONN_RJ45

/* addtional values from this spec */
#define SFF8436_CONN_NONE                0x23

/* Amphenol QSFP, identified as "QSFP" (not "QSFP+") and copper
   pigtail */
#define SFF8436_MEDIA_40GE_CR(idprom)                    \
  (idprom[130] == SFF8436_CONN_CU_PIGTAIL)

/* QSFP+ compliance codes, bytes 131-138 */
#define SFF8436_CC131_XGE_BASE_LRM       SFF8472_CC3_XGE_BASE_LRM
#define SFF8436_CC131_XGE_BASE_LR        SFF8472_CC3_XGE_BASE_LR
#define SFF8436_CC131_XGE_BASE_SR        SFF8472_CC3_XGE_BASE_SR
#define SFF8436_CC131_40GE_BASE_CR4      0x08
#define SFF8436_CC131_40GE_BASE_SR4      0x04
#define SFF8436_CC131_40GE_BASE_LR4      0x02
#define SFF8436_CC131_40GE_ACTIVE        0x01

#define SFF8436_MEDIA_XGE_LRM(idprom)                   \
  ((idprom[131] & SFF8436_CC131_XGE_BASE_LRM) != 0)
#define SFF8436_MEDIA_XGE_LR(idprom)                    \
  ((idprom[131] & SFF8436_CC131_XGE_BASE_LR) != 0)
#define SFF8436_MEDIA_XGE_SR(idprom)                    \
  ((idprom[131] & SFF8436_CC131_XGE_BASE_SR) != 0)

#define SFF8436_MEDIA_40GE_CR4(idprom)                  \
  ((idprom[131] & SFF8436_CC131_40GE_BASE_CR4) != 0)
#define SFF8436_MEDIA_40GE_SR4(idprom)                  \
  ((idprom[131] & SFF8436_CC131_40GE_BASE_SR4) != 0)
#define SFF8436_MEDIA_40GE_LR4(idprom)                  \
  ((idprom[131] & SFF8436_CC131_40GE_BASE_LR4) != 0)
#define SFF8436_MEDIA_40GE_ACTIVE(idprom)               \
  ((idprom[131] & SFF8436_CC131_40GE_ACTIVE) != 0)

#define SFF8436_MEDIA_NONE(idprom)              \
    (idprom[131] == 0)

#define SFF8436_CC132_40G_OTN            0x08
#define SFF8436_CC132_OC48_LONG          SFF8472_CC4_OC48_LONG
#define SFF8436_CC132_OC48_INTERMETIATE  SFF8472_CC4_OC48_INTERMEDIATE
#define SFF8436_CC132_OC48_SHORT         SFF8472_CC4_OC48_SHORT

#define SFF8436_CC133_SAS_6G             0x20
#define SFF8436_CC133_SAS_3G             0x10

#define SFF8436_CC134_GBE_BASE_T         SFF8472_CC6_GBE_BASE_T
#define SFF8436_CC134_GBE_BASE_CX        SFF8472_CC6_GBE_BASE_CX
#define SFF8436_CC134_GBE_BASE_LX        SFF8472_CC6_GBE_BASE_LX
#define SFF8436_CC134_GBE_BASE_SX        SFF8472_CC6_GBE_BASE_SX

#define SFF8436_MEDIA_GBE_T(idprom)                        \
  ((idprom[134] & SFF8436_CC134_GBE_BASE_T) != 0)
#define SFF8436_MEDIA_GBE_CX(idprom)                       \
  ((idprom[134] & SFF8436_CC134_GBE_BASE_CX) != 0)
#define SFF8436_MEDIA_GBE_LX(idprom)                       \
  ((idprom[134] & SFF8436_CC134_GBE_BASE_LX) != 0)
#define SFF8436_MEDIA_GBE_SX(idprom)                       \
  ((idprom[134] & SFF8436_CC134_GBE_BASE_SX) != 0)

#define SFF8436_CC135_FC_VERY_LONG       SFF8472_CC7_FC_VERY_LONG
#define SFF8436_CC135_FC_SHORT           SFF8472_CC7_FC_SHORT
#define SFF8436_CC135_FC_INTERMEDIATE    SFF8472_CC7_FC_INTERMEDIATE
#define SFF8436_CC135_FC_LONG            SFF8472_CC7_FC_LONG
#define SFF8436_CC135_FC_MEDIUM          SFF8472_CC7_FC_MEDIUM
#define SFF8436_CC135_FC_TECH_LC         SFF8472_CC7_FC_TECH_LC
#define SFF8436_CC135_FC_TECH_EL         SFF8472_CC7_FC_TECH_EL
#define SFF8436_CC136_FC_TECH_EL         SFF8472_CC8_FC_TECH_EL
#define SFF8436_CC136_FC_TECH_SN         SFF8472_CC8_FC_TECH_SN
#define SFF8436_CC136_FC_TECH_SL         SFF8472_CC8_FC_TECH_SL
#define SFF8436_CC136_FC_TECH_LL         SFF8472_CC8_FC_TECH_LL

#define SFF8436_TECH_FC_FIBER_LONG(idprom)                   \
  (((idprom[135] & SFF8436_CC135_FC_TECH_LC) != 0)           \
   || ((idprom[136] & SFF8436_CC136_FC_TECH_LL) != 0))

#define SFF8436_TECH_FC_FIBER_SHORT(idprom)                   \
  (((idprom[136] & SFF8436_CC136_FC_TECH_SN) != 0)            \
   || ((idprom[136] & SFF8436_CC136_FC_TECH_SL) != 0))

#define SFF8436_TECH_FC_FIBER(idprom)                   \
    (SFF8436_TECH_FC_FIBER_SHORT(idprom)                \
     || SFF8436_TECH_FC_FIBER_LONG(idprom))

#define SFF8436_TECH_FC_COPPER(idprom)                  \
  (((idprom[135] & SFF8436_CC135_FC_TECH_EL) != 0)      \
   || ((idprom[136] & SFF8436_CC136_FC_TECH_EL) != 0))

#define SFF8436_CC137_FC_MEDIA_TW        SFF8472_CC9_FC_MEDIA_TW
#define SFF8436_CC137_FC_MEDIA_TP        SFF8472_CC9_FC_MEDIA_TP
#define SFF8436_CC137_FC_MEDIA_MI        SFF8472_CC9_FC_MEDIA_MI
#define SFF8436_CC137_FC_MEDIA_TV        SFF8472_CC9_FC_MEDIA_TV
#define SFF8436_CC137_FC_MEDIA_M6        SFF8472_CC9_FC_MEDIA_M6
#define SFF8436_CC137_FC_MEDIA_M5        SFF8472_CC9_FC_MEDIA_M5
#define SFF8436_CC137_FC_MEDIA_OM3       0x02
#define SFF8436_CC137_FC_MEDIA_SM        SFF8472_CC9_FC_MEDIA_SM

#define SFF8436_MEDIA_FC_FIBER_SM(idprom)               \
  ((idprom[137] & SFF8436_CC137_FC_MEDIA_SM) != 0)

#define SFF8436_MEDIA_FC_FIBER_MM(idprom)                      \
    (((idprom[137] & SFF8436_CC137_FC_MEDIA_OM3) != 0)         \
     || ((idprom[137] & SFF8436_CC137_FC_MEDIA_M5) != 0)       \
     || ((idprom[137] & SFF8436_CC137_FC_MEDIA_M6) != 0))

#define SFF8436_MEDIA_FC_FIBER(idprom)                   \
    (SFF8436_MEDIA_FC_FIBER_SM(idprom)                   \
     || SFF8436_MEDIA_FC_FIBER_MM(idprom))

#define SFF8436_MEDIA_FC_COPPER_TP(idprom)                      \
    (((idprom[137] & SFF8436_CC137_FC_MEDIA_TP) != 0)           \
     || ((idprom[137] & SFF8436_CC137_FC_MEDIA_TW) != 0))

#define SFF8436_MEDIA_FC_COPPER_COAX(idprom)                  \
    (((idprom[137] & SFF8436_CC137_FC_MEDIA_TV) != 0)         \
     || ((idprom[137] & SFF8436_CC137_FC_MEDIA_MI) != 0))

#define SFF8436_MEDIA_FC_COPPER(idprom)                  \
  (SFF8436_MEDIA_FC_COPPER_TP(idprom)                    \
  || SFF8436_MEDIA_FC_COPPER_COAX(idprom)

#define SFF8436_CC138_FC_SPEED_100       SFF8472_CC10_FC_SPEED_100
#define SFF8436_CC138_FC_SPEED_200       SFF8472_CC10_FC_SPEED_200
#define SFF8436_CC138_FC_SPEED_400       SFF8472_CC10_FC_SPEED_400
#define SFF8436_CC138_FC_SPEED_1600      SFF8472_CC10_FC_SPEED_1600
#define SFF8436_CC138_FC_SPEED_800       SFF8472_CC10_FC_SPEED_800
#define SFF8436_CC138_FC_SPEED_1200      SFF8472_CC10_FC_SPEED_1200

#define SFF8436_CC139_8B10B              0x01
#define SFF8436_CC139_4B5B               0x02
#define SFF8436_CC139_NRZ                0x03
#define SFF8436_CC139_SONET              0x04
#define SFF8436_CC139_64B66B             0x05
#define SFF8436_CC139_MANCHESTER         0x06

#define SFF8436_CC164_INF_SDR            0x01
#define SFF8436_CC164_INF_DDR            0x02
#define SFF8436_CC164_INF_QDR            0x04
#define SFF8436_CC164_INF_FDR            0x08
#define SFF8436_CC164_INF_EDR            0x10

#define SFF8436_RX_PWR_TYPE_MASK         0x08
#define SFF8436_DOM_GET_RXPWR_TYPE(idprom)               \
        (idprom[220] & SFF8436_RX_PWR_TYPE_MASK)

/* SFF8436 registers */
#define SFF8436_CONTROL_TX_DISABLE       86
/* alternate ways to identify pre-standard 40G cables */
static inline int
_sff8436_qsfp_40g_pre(const uint8_t* idprom)
{
    /* skip if not qsfp+ */
    if (!SFF8436_MODULE_QSFP_PLUS_V2(idprom))
        return 0;

    /* if any of the other standard bits are set, abort */
    if (SFF8436_MEDIA_40GE_CR4(idprom))
        return 0;
    if (SFF8436_MEDIA_40GE_SR4(idprom))
        return 0;
    if (SFF8436_MEDIA_40GE_LR4(idprom))
        return 0;
    if (SFF8436_MEDIA_40GE_ACTIVE(idprom))
        return 0;

    /* since it is QSFP/QSFP+, let us assume 4 lanes */

    /* QDR infiniband == 8G per lane, 32G total, probably 40G Ethernet
       compliant */
    if ((idprom[164] & SFF8436_CC164_INF_QDR) != 0)
        return 1;

    /* nominal BR > 100 * 100M (10G), likely 40G Ethernet compliant */
    if (idprom[140] >= 64)
        return 1;

    return 0;
}

static inline int
_sff8436_qsfp_40g_lm4(const uint8_t* idprom)
{
    if(!SFF8436_MODULE_QSFP_PLUS_V2(idprom)) {
        return 0;
    }

    if (idprom[130] != SFF8436_CONN_LC) return 0;
    if (!SFF8436_MEDIA_NONE(idprom)) return 0;

    if ((idprom[142] != 1) && idprom[143] != 70) {
        return 0;
    }
    return 1;
}

static inline int
_sff8436_qsfp_40g_sm4(const uint8_t* idprom)
{
    if(!SFF8436_MODULE_QSFP_PLUS_V2(idprom)) {
        return 0;
    }

    if (!SFF8436_MEDIA_NONE(idprom)) return 0;
    /* 850nm tx technology */
    if (idprom[147] & 0xF0) return 0;
    /* length is 200m(OM3) or 250m(OM4) */
    if ((idprom[143] != 100) && (idprom[146] != 125)) {
        return 0;
    }
    return 1;
}

static inline int
_sff8436_bitrate(const uint8_t *idprom)
{
    if (idprom[12] == 0)
        return 0;
    if (idprom[12] == 0xFF)
        return 0;
    long long br = (long long) idprom[12] * 100*1000000;
    if ((br > 1000*1000000LL) && (br < 5*1000*1000000LL))
        return 1;
    return 0;
}

static inline int
_sff8436_length_sm(const uint8_t *idprom)
{
    return idprom[142] * 1000;
}

static inline int
_sff8436_length_om3(const uint8_t *idprom)
{
    return idprom[143] * 2;
}

static inline int
_sff8436_length_om2(const uint8_t *idprom)
{
    return idprom[144];
}

static inline int
_sff8436_length_om1(const uint8_t *idprom)
{
    return idprom[145];
}

static inline int
_sff8436_length_copper_active(const uint8_t *idprom)
{
    return idprom[146];
}

static inline int
_sff8436_wavelength(const uint8_t *idprom)
{
    return ((idprom[186] << 8) | idprom[187]) / 20;
}

/*
 * try to detect QSFP modules (outside of the 8436 spec)
 */
static inline int
_sff8436_qsfp_40g_sr2_bidi_pre(const uint8_t *idprom)
{
    /* module should be qsfp */
    if (!SFF8436_MODULE_QSFP_PLUS(idprom)
        && !SFF8436_MODULE_QSFP_PLUS_V2(idprom))
        return 0;

    /* module should have LC connector */
    if (idprom[130] != SFF8436_CONN_LC) return 0;

    /* reject any unrelated compliance codes */
    if (SFF8436_MEDIA_XGE_LRM(idprom)) return 0;
    if (SFF8436_MEDIA_XGE_LR(idprom)) return 0;
    if (SFF8436_MEDIA_XGE_SR(idprom)) return 0;
    if (SFF8436_MEDIA_40GE_CR4(idprom)) return 0;

    /* do *not* report SR4 compliance */
    if (SFF8436_MEDIA_40GE_SR4(idprom)) return 0;

    if (SFF8436_MEDIA_40GE_LR4(idprom)) return 0;
    if (SFF8436_MEDIA_40GE_ACTIVE(idprom)) return 0;

    /* make sure it's MM fiber */
    if (_sff8436_wavelength(idprom) != 850) return 0;

    /* make sure it reports a MM cable length */
    if (_sff8436_length_sm(idprom) > 0) return 0;
    if (!_sff8436_length_om1(idprom)
        && !_sff8436_length_om2(idprom)
        && !_sff8436_length_om3(idprom))
        return 0;

    /*
     * report a BR greater than 10G...
     * for a two-fiber bidi cable we report 20G, but that is per fiber
     */
    long long br = (long long) idprom[140] * 100 * 1000000;
    if ((br >= 20LL*1000*1000000) && (br < 40LL*1000*1000000)) return 1;
    return 0;
    
}

/*
 * non-standard Cisco/Finisar QSFP MM (AOC?)
 */
static inline int
_sff8436_qsfp_40g_sr4_aoc_pre(const uint8_t *idprom)
{
    /* module should be qsfp */
    if (!SFF8436_MODULE_QSFP_PLUS(idprom)
        && !SFF8436_MODULE_QSFP_PLUS_V2(idprom))
        return 0;

    /* no separable connector -- likely active */
    if (idprom[130] != SFF8436_CONN_NONE) return 0;

    /* reject any unrelated compliance codes */
    if (SFF8436_MEDIA_XGE_LRM(idprom)) return 0;
    if (SFF8436_MEDIA_XGE_LR(idprom)) return 0;
    if (SFF8436_MEDIA_XGE_SR(idprom)) return 0;
    if (SFF8436_MEDIA_40GE_CR4(idprom)) return 0;

    /* do *not* report SR4 compliance */
    if (SFF8436_MEDIA_40GE_SR4(idprom)) return 0;

    if (SFF8436_MEDIA_40GE_LR4(idprom)) return 0;
    if (SFF8436_MEDIA_40GE_ACTIVE(idprom)) return 0;

    /* make sure it's MM fiber */
    if (_sff8436_wavelength(idprom) != 850) return 0;

    /* make sure it reports a MM cable length */
    if (_sff8436_length_sm(idprom) > 0) return 0;
    if (!_sff8436_length_om1(idprom)
        && !_sff8436_length_om2(idprom)
        && !_sff8436_length_om3(idprom))
        return 0;

    /*
     * report a BR roughly 10G (4 strands)
     */
    long long br = (long long) idprom[140] * 100 * 1000000;
    if ((br >= 10LL*1000*1000000) && (br < 15LL*1000*1000000)) return 1;
    return 0;
    
}

/*
 * 40G AOC breakout cable, e.g. Finisar
 */
static inline int
_sff8436_qsfp_40g_aoc_breakout(const uint8_t *idprom)
{
    /* module should be qsfp */
    if (!SFF8436_MODULE_QSFP_PLUS(idprom)
        && !SFF8436_MODULE_QSFP_PLUS_V2(idprom))
        return 0;

    /* no separable connector -- likely active */
    if (idprom[130] != SFF8436_CONN_NONE) return 0;

    /* no media compliance, probably active or breakout */
    if (!SFF8436_MEDIA_NONE(idprom)) return 0;

    /* reject any 40G compliance codes */
    if (SFF8436_MEDIA_40GE_CR4(idprom)) return 0;
    if (SFF8436_MEDIA_40GE_SR4(idprom)) return 0;
    if (SFF8436_MEDIA_40GE_LR4(idprom)) return 0;

    /* also does not report as active! Ugh. */
    if (SFF8436_MEDIA_40GE_ACTIVE(idprom)) return 0;

    /* reject any unrelated compliance codes */
    if (SFF8436_MEDIA_XGE_LRM(idprom)) return 0;
    if (SFF8436_MEDIA_XGE_LR(idprom)) return 0;
    if (SFF8436_MEDIA_XGE_SR(idprom)) return 0;

    /* make sure it's MM fiber */
    if (_sff8436_wavelength(idprom) != 850) return 0;

    /* does not report a fiber length, but does report a cable length */
    if (_sff8436_length_sm(idprom) > 0) return 0;
    if (_sff8436_length_om1(idprom) > 0) return 0;
    if (_sff8436_length_om2(idprom) > 0) return 0;
    if (_sff8436_length_om3(idprom) > 0) return 0;
    if (_sff8436_length_copper_active(idprom) == 0) return 0;

    /* maybe, possibly an AOC breakout cable */
    return 1;
}

/*
 * Infer cable length for fixed-length (AOC) optical cables
 *
 * XXX roth -- may also be able to cook up a rule for SFP+ cables too.
 */
static inline int
_sff8436_qsfp_40g_aoc_length(const uint8_t *idprom)
{
    /* module should be qsfp */
    if (!SFF8436_MODULE_QSFP_PLUS(idprom)
        && !SFF8436_MODULE_QSFP_PLUS_V2(idprom))
        return -1;

    /* no separable connector -- likely active */
    if (idprom[130] != SFF8436_CONN_NONE) return -1;

    /* make sure it's MM fiber */
    if (_sff8436_wavelength(idprom) != 850) return 0;

    /* does not report a fiber length, but does report a cable length */
    if (_sff8436_length_sm(idprom) > 0) return -1;
    if (_sff8436_length_om1(idprom) > 0) return -1;
    if (_sff8436_length_om2(idprom) > 0) return -1;
    if (_sff8436_length_om3(idprom) > 0) return -1;
    if (_sff8436_length_copper_active(idprom) > 0)
        return _sff8436_length_copper_active(idprom);

    return -1;
}

#endif /* __SFF_8436_H__ */
