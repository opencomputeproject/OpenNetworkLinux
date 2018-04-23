
/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright 2017 (C) Delta Networks, Inc.
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
#include "eeprom_info.h"
#include <string.h>

#define PSU_MODEL_LEN		11
#define PSU_SERIES_LEN		14

/*
	function:
		search the eeprom with the key 
	variables:
		eeprom: the eeprom data;
		key : the searching key	 string
		mode : 1 means model search, 0 means series search
	return:
		success, return index which points to the finding string
	    failed, return -1
*/

int eeprom_info_find(char *eeprom, int len, const char *key,int mode)
{
	int index=0;
	int found=0;
	int key_len=0;
	if(!eeprom || !key)
		return -1;

	key_len=strlen(key);
	
	while(index < len-key_len){
		if (!strncmp(&eeprom[index], key, key_len)){
			found=1;
			break;
		}
		index++;
	}
	if(found){
		/*mode is 1 means the model search and mode is 0 means the series search*/
		if((mode == 1) && (index < len-PSU_MODEL_LEN)) 
			return index;
		else if ((mode == 0) && (index < len-PSU_SERIES_LEN))
			return index;
		else
			return -1;
	}

	return -1;

}

int eeprom_info_get(uint8_t *eeprom, int len, char *type, char *v)
{
	const char psu_model_key[]="DPS";
	const char psu_460_series_key[]="DZRD";
	const char psu_550_series_key[]="GVVD";
	int index=0;
	char model[PSU_MODEL_LEN+1]={'\0'};
	char * eep=NULL;
	if(!eeprom || !type ||!v)
		return -1;
	eep=(char *)eeprom;
	/*fan eeprom is not now*/
	if((strcmp(type, "fan_model")==0) ||(strcmp(type,"fan_series"))==0)
		return 0;
	/*first get the psu tpye*/
	index = eeprom_info_find(eep,len,psu_model_key,1);
	if(index <0)
		return -1;
	strncpy(model,&eep[index],PSU_MODEL_LEN);

	if((strcmp(type,"psu_model"))==0){
		strncpy(v,model,PSU_MODEL_LEN);
	}
	else if ((strcmp(type,"psu_series"))==0){
		if(strstr(model,"460")){
			index = eeprom_info_find(eep,len,psu_460_series_key,0);
			if(index <0)
				return -1;
			strncpy(v,&eep[index],PSU_SERIES_LEN);
		}
		else if(strstr(model,"550")){
			index = eeprom_info_find(eep,len,psu_550_series_key,0);
			if(index <0)
				return -1;
			strncpy(v,&eep[index],PSU_SERIES_LEN);
		} 
	}
	else
		return -1;

	return 0;
}

