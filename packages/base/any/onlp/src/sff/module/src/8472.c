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
 *
 *
 ***********************************************************/
#include <sff/8472.h>

#if 0

/*
 * some CR cables identify as infiniband copper
 * some CR cables identify as FC twinax
 * some CR cables identify their electrical compliance
 * using bytes 60,61
 * some CR cables identify as FC electrical intra-
 * or inter-enclosure (bytes 7, 8)
 */

int
sff8472_inf_1x(const uint8_t* idprom)
{
    if ((idprom[3] & SFF8472_CC3_INF_1X_CU_PASSIVE) != 0) return 1;
    if ((idprom[3] & SFF8472_CC3_INF_1X_CU_ACTIVE) != 0) return 1;
    if ((idprom[3] & SFF8472_CC3_INF_1X_LX) != 0) return 1;
    if ((idprom[3] & SFF8472_CC3_INF_1X_SX) != 0) return 1;
    return 0;
}

int
sff8472_inf_1x_cu_active(const uint8_t* idprom)
{
    if ((idprom[3] & SFF8472_CC3_INF_1X_CU_PASSIVE) != 0) return 0;
    if ((idprom[3] & SFF8472_CC3_INF_1X_LX) != 0) return 0;
    if ((idprom[3] & SFF8472_CC3_INF_1X_SX) != 0) return 0;

    if ((idprom[3] & SFF8472_CC3_INF_1X_CU_ACTIVE) != 0) return 1;

    return 0;
}

int
sff8472_inf_1x_cu_passive(const uint8_t* idprom)
{
    if ((idprom[3] & SFF8472_CC3_INF_1X_CU_ACTIVE) != 0) return 0;
    if ((idprom[3] & SFF8472_CC3_INF_1X_LX) != 0) return 0;
    if ((idprom[3] & SFF8472_CC3_INF_1X_SX) != 0) return 0;

    if ((idprom[3] & SFF8472_CC3_INF_1X_CU_PASSIVE) != 0) return 1;

    return 0;
}

int
sff8472_fc_media(const uint8_t* idprom)
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

int
sff8472_fc_media_tw(const uint8_t* idprom)
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

int
sff8472_tech_fc(const uint8_t* idprom)
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

int
sff8472_tech_fc_el(const uint8_t* idprom)
{
    /* non-EL types */
    if ((idprom[7] & SFF8472_CC7_FC_TECH_LC) != 0) return 0;
    if ((idprom[7] & SFF8472_CC7_FC_TECH_SA) != 0) return 0;
    if ((idprom[8] & SFF8472_CC8_FC_TECH_LL) != 0) return 0;
    if ((idprom[8] & SFF8472_CC8_FC_TECH_SL) != 0) return 0;
    if ((idprom[8] & SFF8472_CC8_FC_TECH_SN) != 0) return 0;

    if (((idprom[7] & SFF8472_CC7_FC_TECH_EL) != 0)
        && ((idprom[8] & SFF8472_CC8_FC_TECH_EL) == 0))
        return 1;
    if (((idprom[7] & SFF8472_CC7_FC_TECH_EL) == 0)
        && ((idprom[8] & SFF8472_CC8_FC_TECH_EL) != 0))
        return 1;

    return 0;
}

/* do not specify an FC speed code unless you actually FC */
int
sff8472_fc_speed_ok(const uint8_t* idprom)
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
 * XXX roth
 * some CR cables do not list an SFP+ active/passive bit,
 * but register as active or passive via infiniband instead
 */

int
sff8472_sfp_plus_passive(const uint8_t* idprom)
{
    if ((idprom[8] & SFF8472_CC8_SFP_PASSIVE) != 0) return 1;

    /* also allow pre-standard cables identifying as infiniband */
    if (_sff8472_inf_1x_cu_passive(idprom)) return 1;

    return 0;
}

int
sff8472_sfp_plus_active(const uint8_t* idprom)
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

#define _SFF8472_COMPLIANCE_PASSIVE_FC(idprom)  \
    (((idprom[60] & SFF8471_CC60_FC_PI_4) != 0) \
     && (idprom[61] == 0))

#define _SFF8472_COMPLIANCE_PASSIVE_SFF(idprom) \
    (((idprom[60] & SFF8471_CC60_SFF8431) != 0) \
     && (idprom[61] == 0))

#define _SFF8472_COMPLIANCE_ACTIVE_FC(idprom)                   \
    ((((idprom[60] & SFF8471_CC60_FC_PI_4) != 0)                \
      || (idprom[60] & SFF8471_CC60_FC_PI_4_LIMITING) != 0)     \
     && (idprom[61] == 0))

#define _SFF8472_COMPLIANCE_ACTIVE_SFF(idprom)                  \
    ((((idprom[60] & SFF8471_CC60_SFF8431) != 0)                \
      || (idprom[60] & SFF8471_CC60_SFF8431_LIMITING) != 0)     \
     && (idprom[61] == 0))

/*
 * Cisco pre-standard CR cables
 */
int
sff8472_hack_cr(const uint8_t* idprom)
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

/* grab-bag to detect pre-standard CR media */
int
sff8472_media_cr_passive(const uint8_t* idprom)
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
    if (idprom[6] != 0) return 0;

    if (_sff8472_sfp_plus_passive(idprom))
        maybe = 1;

    if (_sff8472_fc_media_tw(idprom))
        maybe = 1;
    else if (_sff8472_fc_media(idprom))
        return 0;

    if (!_sff8472_fc_speed_ok(idprom))
        return 0;

    if (_sff8472_hack_cr(idprom))
        maybe = 1;

    if (maybe) {
        if (!_SFF8472_COMPLIANCE_PASSIVE_FC(idprom)
            && !_SFF8472_COMPLIANCE_PASSIVE_SFF(idprom)
            && (_SFF8472_WAVELENGTH(idprom) != 850)
            && !_SFF8472_COMPLIANCE_UNSPEC(idprom)
            && !_sff8472_hack_cr(idprom))
            return 0;
    }

    return maybe;
}

int
sff8472_media_cr_active(const uint8_t* idprom)
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
    if (idprom[6] != 0) return 0;

    if (_sff8472_sfp_plus_active(idprom))
        maybe = 1;

    if (_sff8472_fc_media_tw(idprom))
        maybe = 1;
    else if (_sff8472_fc_media(idprom))
        return 0;

    if (!_sff8472_fc_speed_ok(idprom))
        return 0;

    if (maybe) {
        if (!_SFF8472_COMPLIANCE_ACTIVE_FC(idprom)
            && !_SFF8472_COMPLIANCE_ACTIVE_SFF(idprom)
            && !_SFF8472_COMPLIANCE_UNSPEC(idprom))
            return 0;
    }

    return maybe;
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


#endif
