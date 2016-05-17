/**************************************************************************//**
 *
 *
 *
 *****************************************************************************/
#include <sff/sff_config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <AIM/aim.h>
#include <sff/sff.h>
#include <sff/sff_db.h>

int
aim_main(int argc, char* argv[])
{
    int i;

    sff_db_entry_t* entries;
    sff_db_entry_t* p;
    int count;

    sff_db_get(&entries, &count);

    for(i = 0, p=entries; i < count; i++, p++) {
        int rv;
        sff_eeprom_t se;

        aim_printf(&aim_pvs_stdout, "Verifying entry %d: %s:%s:%s...\n",
                   i,
                   p->se.info.vendor,
                   p->se.info.model,
                   p->se.info.serial);


        if( (rv=sff_eeprom_parse(&se, p->se.eeprom)) < 0) {
            AIM_DIE("index=%d sff_eeprom_parse=%d\n", i, rv);
        }
        if (!se.identified) {
            AIM_DIE("index=%d identified=0\n", i);
        }
        if(strcmp(se.info.vendor, p->se.info.vendor)) {
            AIM_DIE("index=%d vendor expected '%s' got '%s'",
                    i, p->se.info.vendor, se.info.vendor);
        }
        if(strcmp(se.info.model, p->se.info.model)) {
            AIM_DIE("index=%d model expected '%s' got '%s'",
                    i, p->se.info.model, se.info.model);
        }
        if(strcmp(se.info.serial, p->se.info.serial)) {
            AIM_DIE("index=%d serial expected '%s' got '%s'",
                    i, p->se.info.serial, se.info.serial);
        }
        if(se.info.sfp_type != p->se.info.sfp_type) {
            AIM_DIE("index=%d sfp_type expected '%{sff_sfp_type}' got '%{sff_sfp_type}'",
                    i, p->se.info.sfp_type, se.info.sfp_type);
        }
        if(strcmp(se.info.sfp_type_name, p->se.info.sfp_type_name)) {
            AIM_DIE("index=%d type_name expected '%s' got '%s'",
                    i, p->se.info.sfp_type, se.info.sfp_type);
        }
        if(se.info.module_type != p->se.info.module_type) {
            AIM_DIE("index=%d module_type expected '%{sff_module_type}' got '%{sff_module_type}'",
                    i, p->se.info.module_type, se.info.module_type);
        }
        if(se.info.media_type != p->se.info.media_type) {
            AIM_DIE("index=%d media_type expected '%{sff_media_type}' got '%{sff_media_type}'\n",
                    i, p->se.info.media_type, se.info.media_type);
        }
        if(strcmp(se.info.media_type_name, p->se.info.media_type_name)) {
            AIM_DIE("index=%d media_type_name expected '%s' got '%s'",
                    i, p->se.info.media_type_name, se.info.media_type_name);
        }
        if (se.info.caps != p->se.info.caps) {
            AIM_DIE("index=%d caps expected '%{sff_module_caps}' got '%{sff_module_caps}'",
                    i, p->se.info.caps, se.info.caps);
        }

        if(se.info.length != p->se.info.length) {
            AIM_DIE("index=%d length expected %d got %d",
                    i, p->se.info.length, se.info.length);
        }
        if(se.info.length == -1 && se.info.length_desc[0]) {
            AIM_DIE("index=%d length_desc expected '%s' got '%s'",
                    i, '\0', se.info.length_desc);
        }
        else if(se.info.length != -1) {
            char tmp[32];
            snprintf(tmp, sizeof(tmp), "%dm", se.info.length);
            if(strcmp(tmp, se.info.length_desc)) {
                AIM_DIE("index=%d length_desc expected '%s' got '%s'",
                        i, tmp, se.info.length_desc);
            }
        }

        aim_printf(&aim_pvs_stdout, "Verifying entry %d: %s:%s:%s...PASSED\n",
                   i,
                   p->se.info.vendor,
                   p->se.info.model,
                   p->se.info.serial);

    }
    return 0;
}

