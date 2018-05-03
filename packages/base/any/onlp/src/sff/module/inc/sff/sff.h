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
#ifndef __SFF_SFF_H__
#define __SFF_SFF_H__

#include <sff/sff_config.h>
#include <AIM/aim_pvs.h>

#include <dependmodules.x>

/* <auto.start.enum(ALL).header> */
/** sff_media_type */
typedef enum sff_media_type_e {
    SFF_MEDIA_TYPE_COPPER,
    SFF_MEDIA_TYPE_FIBER,
    SFF_MEDIA_TYPE_LAST = SFF_MEDIA_TYPE_FIBER,
    SFF_MEDIA_TYPE_COUNT,
    SFF_MEDIA_TYPE_INVALID = -1,
} sff_media_type_t;

/** Strings macro. */
#define SFF_MEDIA_TYPE_STRINGS \
{\
    "COPPER", \
    "FIBER", \
}
/** Enum names. */
const char* sff_media_type_name(sff_media_type_t e);

/** Enum values. */
int sff_media_type_value(const char* str, sff_media_type_t* e, int substr);

/** Enum descriptions. */
const char* sff_media_type_desc(sff_media_type_t e);

/** validator */
#define SFF_MEDIA_TYPE_VALID(_e) \
    ( (0 <= (_e)) && ((_e) <= SFF_MEDIA_TYPE_FIBER))

/** sff_media_type_map table. */
extern aim_map_si_t sff_media_type_map[];
/** sff_media_type_desc_map table. */
extern aim_map_si_t sff_media_type_desc_map[];

/** sff_module_caps */
typedef enum sff_module_caps_e {
    SFF_MODULE_CAPS_F_100 = 1,
    SFF_MODULE_CAPS_F_1G = 2,
    SFF_MODULE_CAPS_F_10G = 4,
    SFF_MODULE_CAPS_F_25G = 8,
    SFF_MODULE_CAPS_F_40G = 16,
    SFF_MODULE_CAPS_F_100G = 32,
} sff_module_caps_t;

/** Enum names. */
const char* sff_module_caps_name(sff_module_caps_t e);

/** Enum values. */
int sff_module_caps_value(const char* str, sff_module_caps_t* e, int substr);

/** Enum descriptions. */
const char* sff_module_caps_desc(sff_module_caps_t e);

/** Enum validator. */
int sff_module_caps_valid(sff_module_caps_t e);

/** validator */
#define SFF_MODULE_CAPS_VALID(_e) \
    (sff_module_caps_valid((_e)))

/** sff_module_caps_map table. */
extern aim_map_si_t sff_module_caps_map[];
/** sff_module_caps_desc_map table. */
extern aim_map_si_t sff_module_caps_desc_map[];

/** sff_module_type */
typedef enum sff_module_type_e {
    SFF_MODULE_TYPE_100G_AOC,
    SFF_MODULE_TYPE_100G_BASE_CR4,
    SFF_MODULE_TYPE_100G_BASE_SR4,
    SFF_MODULE_TYPE_100G_BASE_LR4,
    SFF_MODULE_TYPE_100G_CWDM4,
    SFF_MODULE_TYPE_100G_PSM4,
    SFF_MODULE_TYPE_40G_BASE_CR4,
    SFF_MODULE_TYPE_40G_BASE_SR4,
    SFF_MODULE_TYPE_40G_BASE_LR4,
    SFF_MODULE_TYPE_40G_BASE_LM4,
    SFF_MODULE_TYPE_40G_BASE_ACTIVE,
    SFF_MODULE_TYPE_40G_BASE_CR,
    SFF_MODULE_TYPE_40G_BASE_SR2,
    SFF_MODULE_TYPE_40G_BASE_SM4,
    SFF_MODULE_TYPE_40G_BASE_ER4,
    SFF_MODULE_TYPE_25G_BASE_CR,
    SFF_MODULE_TYPE_25G_BASE_SR,
    SFF_MODULE_TYPE_25G_BASE_LR,
    SFF_MODULE_TYPE_25G_BASE_AOC,
    SFF_MODULE_TYPE_10G_BASE_SR,
    SFF_MODULE_TYPE_10G_BASE_LR,
    SFF_MODULE_TYPE_10G_BASE_LRM,
    SFF_MODULE_TYPE_10G_BASE_ER,
    SFF_MODULE_TYPE_10G_BASE_CR,
    SFF_MODULE_TYPE_10G_BASE_SX,
    SFF_MODULE_TYPE_10G_BASE_LX,
    SFF_MODULE_TYPE_10G_BASE_ZR,
    SFF_MODULE_TYPE_10G_BASE_SRL,
    SFF_MODULE_TYPE_1G_BASE_SX,
    SFF_MODULE_TYPE_1G_BASE_LX,
    SFF_MODULE_TYPE_1G_BASE_CX,
    SFF_MODULE_TYPE_1G_BASE_T,
    SFF_MODULE_TYPE_100_BASE_LX,
    SFF_MODULE_TYPE_100_BASE_FX,
    SFF_MODULE_TYPE_4X_MUX,
    SFF_MODULE_TYPE_LAST = SFF_MODULE_TYPE_4X_MUX,
    SFF_MODULE_TYPE_COUNT,
    SFF_MODULE_TYPE_INVALID = -1,
} sff_module_type_t;

/** Strings macro. */
#define SFF_MODULE_TYPE_STRINGS \
{\
    "100G_AOC", \
    "100G_BASE_CR4", \
    "100G_BASE_SR4", \
    "100G_BASE_LR4", \
    "100G_CWDM4", \
    "100G_PSM4", \
    "40G_BASE_CR4", \
    "40G_BASE_SR4", \
    "40G_BASE_LR4", \
    "40G_BASE_LM4", \
    "40G_BASE_ACTIVE", \
    "40G_BASE_CR", \
    "40G_BASE_SR2", \
    "40G_BASE_SM4", \
    "40G_BASE_ER4", \
    "25G_BASE_CR", \
    "25G_BASE_SR", \
    "25G_BASE_LR", \
    "25G_BASE_AOC", \
    "10G_BASE_SR", \
    "10G_BASE_LR", \
    "10G_BASE_LRM", \
    "10G_BASE_ER", \
    "10G_BASE_CR", \
    "10G_BASE_SX", \
    "10G_BASE_LX", \
    "10G_BASE_ZR", \
    "10G_BASE_SRL", \
    "1G_BASE_SX", \
    "1G_BASE_LX", \
    "1G_BASE_CX", \
    "1G_BASE_T", \
    "100_BASE_LX", \
    "100_BASE_FX", \
    "4X_MUX", \
}
/** Enum names. */
const char* sff_module_type_name(sff_module_type_t e);

/** Enum values. */
int sff_module_type_value(const char* str, sff_module_type_t* e, int substr);

/** Enum descriptions. */
const char* sff_module_type_desc(sff_module_type_t e);

/** validator */
#define SFF_MODULE_TYPE_VALID(_e) \
    ( (0 <= (_e)) && ((_e) <= SFF_MODULE_TYPE_4X_MUX))

/** sff_module_type_map table. */
extern aim_map_si_t sff_module_type_map[];
/** sff_module_type_desc_map table. */
extern aim_map_si_t sff_module_type_desc_map[];

/** sff_sfp_type */
typedef enum sff_sfp_type_e {
    SFF_SFP_TYPE_SFP,
    SFF_SFP_TYPE_QSFP,
    SFF_SFP_TYPE_QSFP_PLUS,
    SFF_SFP_TYPE_QSFP28,
    SFF_SFP_TYPE_SFP28,
    SFF_SFP_TYPE_LAST = SFF_SFP_TYPE_SFP28,
    SFF_SFP_TYPE_COUNT,
    SFF_SFP_TYPE_INVALID = -1,
} sff_sfp_type_t;

/** Strings macro. */
#define SFF_SFP_TYPE_STRINGS \
{\
    "SFP", \
    "QSFP", \
    "QSFP_PLUS", \
    "QSFP28", \
    "SFP28", \
}
/** Enum names. */
const char* sff_sfp_type_name(sff_sfp_type_t e);

/** Enum values. */
int sff_sfp_type_value(const char* str, sff_sfp_type_t* e, int substr);

/** Enum descriptions. */
const char* sff_sfp_type_desc(sff_sfp_type_t e);

/** validator */
#define SFF_SFP_TYPE_VALID(_e) \
    ( (0 <= (_e)) && ((_e) <= SFF_SFP_TYPE_SFP28))

/** sff_sfp_type_map table. */
extern aim_map_si_t sff_sfp_type_map[];
/** sff_sfp_type_desc_map table. */
extern aim_map_si_t sff_sfp_type_desc_map[];
/* <auto.end.enum(ALL).header> */

/**
 * @brief Determine the SFP type (from the idprom data).
 * @param idprom The SFF idprom.
 */
sff_sfp_type_t sff_sfp_type_get(const uint8_t* idprom);

/**
 * @brief Determine the SFF module type (from the idprom data).
 * @param idprom The SFF idprom.
 */
sff_module_type_t sff_module_type_get(const uint8_t* idprom);

/**
 * @brief Determine the SFF Media type (from the idprom data)./
 * @param idprom The SFF idprom.
 */
sff_media_type_t sff_media_type_get(sff_module_type_t mt);

/**
 * @brief Determine the SFF module capabilities (from the idprom data).
 * @param idprom The SFF idprom.
 * @param caps [out] Receives the capabilities.
 * @returns 0 on successful parse.
 * @returns < 0 on error.
 */
int sff_module_caps_get(sff_module_type_t mt, uint32_t* caps);


typedef struct sff_info_s {

    /** Vendor Name */
    char vendor[17];

    /** Model Number */
    char model[17];

    /** Serial Number */
    char serial[17];

    /** SFP Type */
    sff_sfp_type_t sfp_type;

    /** SFP Type Name */
    const char* sfp_type_name;

    /** Module Type */
    sff_module_type_t module_type;

    /** Module Type Name */
    const char* module_type_name;

    /** Media Type */
    sff_media_type_t media_type;

    /** Media Type Name */
    const char* media_type_name;

    /** Capabilities */
    sff_module_caps_t caps;

    /** Cable length, if available */
    int length;
    char length_desc[16];
} sff_info_t;

/**
 * SFF Module Information Structure
 */
typedef struct sff_eeprom_s {
    /** Raw eeprom data */
    uint8_t eeprom[256];

    /** computed checksums for eeprom contents */
    uint8_t cc_base;
    uint8_t cc_ext;

    /** Whether this EEPROM was successfully parsed and identified. */
    int identified;

    /** Parsed SFF Information */
    sff_info_t info;

} sff_eeprom_t;


/**
 * @brief Initialize an SFF module information structure.
 * @param rv [out] Receives the data.
 * @param eeprom Raw EEPROM data.
 * @note if eeprom is != NULL it will be copied into rv->eeprom first.
 * @note if eeprom is NULL it is assumed the rv->eeprom buffer
 * has already been initialized.
 */
int sff_eeprom_parse(sff_eeprom_t* rv, uint8_t* eeprom);

/**
 * @brief Initialize an SFF module information structure from a file.
 * @param rv [out] Receives thh data.
 * @param fname The filename.
 */
int sff_eeprom_parse_file(sff_eeprom_t* rv, const char* fname);

/**
 * @brief Clear an sff_eeprom_t structure.
 * @param eeprom The eeprom structure.
 */
void sff_eeprom_invalidate(sff_eeprom_t *info);

/**
 * @brief Determine if this is a valid SFP
 *     (whether or not we can parse it)
 * @param info The info structure.
 * @param verbose Whether to report errors on invalid contents.
 */
int sff_eeprom_validate(sff_eeprom_t *info, int verbose);

/**
 * @brief Show an sff info structure.
 * @param info The info structure.
 * @param pvs The output pvs.
 */
void sff_info_show(sff_info_t* info, aim_pvs_t* pvs);

/**
 * @brief Initialize an info structure based on module type.
 */
int sff_info_init(sff_info_t* pinfo, sff_module_type_t type,
                  const char* vendor, const char* model, const char* serial,
                  int length);


int sff_info_from_module_type(sff_info_t* info, sff_sfp_type_t st,
                              sff_module_type_t mt);

#ifdef DEPENDMODULE_INCLUDE_CJSON_UTIL

#include <cjson_util/cjson_util.h>

/**
 * @brief Return a JSON representation of the sff_info_t structure.
 * @param cj Add keys this object. If NULL a new object is created.
 * @param info The info structure.
 */
cJSON* sff_info_json(cJSON* cj, sff_info_t* info);

#endif /* DEPENDMODULE_CJSON_UTIL */

#endif /* __SFF_SFF_H__ */
