/**********************************************************************
 *
 * 8636.h
 *
 * idprom defintions for QSFP+ modules.
 *
 **********************************************************************/

#ifndef __SFF_8636_H__
#define __SFF_8636_H__

#include <sff/sff_config.h>
#include "sff/8472.h"

/* identifier, bytes 128 page 0 (also byte 0) */

#define SFF8636_IDENT_UNKNOWN            SFF8472_IDENT_UNKNOWN
#define SFF8636_IDENT_GBIC               SFF8472_IDENT_GBIC
#define SFF8636_IDENT_BASEBOARD          SFF8472_IDENT_BASEBOARD
#define SFF8636_IDENT_SFP                SFF8472_IDENT_SFP
#define SFF8636_IDENT_XBI                SFF8472_IDENT_XBI
#define SFF8636_IDENT_XENPAK             SFF8472_IDENT_XENPAK
#define SFF8636_IDENT_XFP                SFF8472_IDENT_XFP
#define SFF8636_IDENT_XFF                SFF8472_IDENT_XFF
#define SFF8636_IDENT_XFPE               SFF8472_IDENT_XFPE
#define SFF8636_IDENT_XPAK               SFF8472_IDENT_XPAK
#define SFF8636_IDENT_X2                 SFF8472_IDENT_X2
#define SFF8636_IDENT_DWDM_SFP           SFF8472_IDENT_DWDM_SFP
#define SFF8636_IDENT_QSFP               SFF8472_IDENT_QSFP

/* additional values from this spec */
#define SFF8636_IDENT_QSFP_PLUS          0x0D
#define SFF8636_IDENT_QSFP28             0x11

#define SFF8636_MODULE_QSFP28(idprom)           \
    (idprom[0] == SFF8636_IDENT_QSFP28)

#define SFF8636_RX1_PWR(idprom)                \
    (idprom[34] << 8 | idprom[34 + 1])
#define SFF8636_RX2_PWR(idprom)                \
    (idprom[36] << 8 | idprom[36 + 1])
#define SFF8636_RX3_PWR(idprom)                \
    (idprom[38] << 8 | idprom[38 + 1])
#define SFF8636_RX4_PWR(idprom)                \
    (idprom[40] << 8 | idprom[40 + 1])

#define SFF8636_TX1_BIAS(idprom)              \
    (idprom[42] << 8 | idprom[42 + 1])
#define SFF8636_TX2_BIAS(idprom)              \
    (idprom[44] << 8 | idprom[44 + 1])
#define SFF8636_TX3_BIAS(idprom)              \
    (idprom[46] << 8 | idprom[46 + 1])
#define SFF8636_TX4_BIAS(idprom)              \
    (idprom[48] << 8 | idprom[48 + 1])

#define SFF8636_TX1_PWR(idprom)                \
    (idprom[50] << 8 | idprom[50 + 1])
#define SFF8636_TX2_PWR(idprom)                \
    (idprom[52] << 8 | idprom[52 + 1])
#define SFF8636_TX3_PWR(idprom)                \
    (idprom[54] << 8 | idprom[54 + 1])
#define SFF8636_TX4_PWR(idprom)                \
    (idprom[56] << 8 | idprom[56 + 1])

#define SFF8636_SFP_VOLT(idprom)              \
    (idprom[26] << 8 | idprom[26 + 1])
#define SFF8636_SFP_TEMP(idprom)              \
    (idprom[22] << 8 | idprom[22 + 1])

/* connector value, byte 130 page 0 */

#define SFF8636_CONN_UNKNOWN             SFF8472_CONN_UNKNOWN
#define SFF8636_CONN_SC                  SFF8472_CONN_SC
#define SFF8636_CONN_FC1_CU              SFF8472_CONN_FC1_CU
#define SFF8636_CONN_FC2_CU              SFF8472_CONN_FC2_CU
#define SFF8636_CONN_BNC                 SFF8472_CONN_BNC
#define SFF8636_CONN_FC_COAX             SFF8472_CONN_FC_COAX
#define SFF8636_CONN_FJ                  SFF8472_CONN_FJ
#define SFF8636_CONN_LC                  SFF8472_CONN_LC
#define SFF8636_CONN_MT_RJ               SFF8472_CONN_MT_RJ
#define SFF8636_CONN_MU                  SFF8472_CONN_MU
#define SFF8636_CONN_SG                  SFF8472_CONN_SG
#define SFF8636_CONN_SI_PIGTAIL          SFF8472_CONN_SI_PIGTAIL
#define SFF8636_CONN_MPO                 SFF8472_CONN_MPO
#define SFF8636_CONN_HSSDC_II            SFF8472_CONN_HSSDC_II
#define SFF8636_CONN_CU_PIGTAIL          SFF8472_CONN_CU_PIGTAIL
#define SFF8636_CONN_RJ45                SFF8472_CONN_RJ45

/* addtional values from this spec */
#define SFF8636_CONN_NONE                0x23

/* Amphenol QSFP, identified as "QSFP" (not "QSFP+") and copper
   pigtail */
#define SFF8636_MEDIA_40GE_CR(idprom)                    \
  (idprom[130] == SFF8636_CONN_CU_PIGTAIL)

/* QSFP+ compliance codes, bytes 131-138 */
#define SFF8636_CC131_EXTENDED           0x80 
#define SFF8636_CC131_XGE_BASE_LRM       SFF8472_CC3_XGE_BASE_LRM
#define SFF8636_CC131_XGE_BASE_LR        SFF8472_CC3_XGE_BASE_LR
#define SFF8636_CC131_XGE_BASE_SR        SFF8472_CC3_XGE_BASE_SR
#define SFF8636_CC131_40GE_BASE_CR4      0x08
#define SFF8636_CC131_40GE_BASE_SR4      0x04
#define SFF8636_CC131_40GE_BASE_LR4      0x02
#define SFF8636_CC131_40GE_ACTIVE        0x01

#define SFF8636_MEDIA_EXTENDED(idprom)                   \
  ((idprom[131] & SFF8636_CC131_EXTENDED) != 0)

#define SFF8636_MEDIA_XGE_LRM(idprom)                   \
  ((idprom[131] & SFF8636_CC131_XGE_BASE_LRM) != 0)
#define SFF8636_MEDIA_XGE_LR(idprom)                    \
  ((idprom[131] & SFF8636_CC131_XGE_BASE_LR) != 0)
#define SFF8636_MEDIA_XGE_SR(idprom)                    \
  ((idprom[131] & SFF8636_CC131_XGE_BASE_SR) != 0)

#define SFF8636_MEDIA_40GE_CR4(idprom)                  \
  ((idprom[131] & SFF8636_CC131_40GE_BASE_CR4) != 0)
#define SFF8636_MEDIA_40GE_SR4(idprom)                  \
  ((idprom[131] & SFF8636_CC131_40GE_BASE_SR4) != 0)
#define SFF8636_MEDIA_40GE_LR4(idprom)                  \
  ((idprom[131] & SFF8636_CC131_40GE_BASE_LR4) != 0)
#define SFF8636_MEDIA_40GE_ACTIVE(idprom)               \
  ((idprom[131] & SFF8636_CC131_40GE_ACTIVE) != 0)

#define SFF8636_MEDIA_NONE(idprom)              \
    (idprom[131] == 0)

#define SFF8636_CC192_100GE_AOC          0x01
#define SFF8636_CC192_100GE_SR4          0x02
#define SFF8636_CC192_100GE_LR4          0x03
#define SFF8636_CC192_100GE_CWDM4        0x06
#define SFF8636_CC192_100GE_PSM4         0x07
#define SFF8636_CC192_100GE_ACC          0x08
#define SFF8636_CC192_100GE_CR4          0x0B
#define SFF8636_CC192_25GE_CR_S          0x0C
#define SFF8636_CC192_25GE_CR_N          0x0D

#define SFF8636_MEDIA_100GE_AOC(idprom)                  \
  (idprom[192] == SFF8636_CC192_100GE_AOC)
#define SFF8636_MEDIA_100GE_SR4(idprom)                  \
  (idprom[192] == SFF8636_CC192_100GE_SR4)
#define SFF8636_MEDIA_100GE_LR4(idprom)                  \
  (idprom[192] == SFF8636_CC192_100GE_LR4)
#define SFF8636_MEDIA_100GE_CWDM4(idprom)                \
  (idprom[192] == SFF8636_CC192_100GE_CWDM4)
#define SFF8636_MEDIA_100GE_CR4(idprom)                  \
  (idprom[192] == SFF8636_CC192_100GE_CR4)
#define SFF8636_MEDIA_25GE_CR_S(idprom)                  \
  (idprom[192] == SFF8636_CC192_25GE_CR_S)
#define SFF8636_MEDIA_25GE_CR_N(idprom)                  \
  (idprom[192] == SFF8636_CC192_25GE_CR_N)

#define SFF8636_RX_PWR_TYPE_MASK            0x08
#define SFF8636_DOM_GET_RXPWR_TYPE(idprom)               \
        (idprom[220] & SFF8636_RX_PWR_TYPE_MASK)
#define SFF8636_TX_PWR_SUPPORT_MASK         0x04
#define SFF8636_DOM_GET_TXPWR_SUPPORT(idprom)               \
        (idprom[220] & SFF8636_TX_PWR_SUPPORT_MASK)

/*
 * Infer cable length for fixed-length (AOC) optical cables
 *
 * XXX roth -- may also be able to cook up a rule for SFP+ cables too.
 */
static inline int
_sff8636_qsfp28_100g_aoc_length(const uint8_t *idprom)
{
    /* module should be qsfp28 */
    if (!SFF8636_MODULE_QSFP28(idprom))
        return -1;

    /* no separable connector -- likely active */
    if (idprom[130] != SFF8636_CONN_NONE) return -1;

    return idprom[146];
}

#endif /* __SFF_8636_H__ */
