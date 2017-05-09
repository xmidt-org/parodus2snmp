/**
 * Copyright 2017 Comcast Cable Communications Management, LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
/*
 * snmpadapter_parser.c
 *
 *  Created on: May 5, 2017
 *      Author: Murugan Viswanathan
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "snmpadapter_parser.h"

#define OID_ARRAY_NAME "names"

void free_snmp_record(snmpadapter_record* rec)
{
	if(rec == NULL)
		return;

	if(rec->type == SNMPADAPTER_TYPE_GET)
	{
		if(rec->u.get != NULL)
		{
			for (int c = 0; c < rec->u.get->count; c++)
			{
				free(rec->u.get->oid[c]);
				rec->u.get->oid[c] = NULL;
			}
			rec->u.get->count = 0;

			free(rec->u.get);
			rec->u.get = NULL;
		}
	}
	else if(rec->type == SNMPADAPTER_TYPE_SET)
	{

	}

	rec->type = 0;
	free(rec);
}


int extract_snmp_get_params(cJSON *request, snmpadapter_record* snmp_params)
{

	cJSON *oidArray = NULL;
	int oidCount, i;

	//NOTE : Caller free snmp_params->u.get by calling free_snmp_record()
	snmp_params->u.get = (snmpadapter_get_record*) malloc(sizeof(snmpadapter_get_record));
	memset(snmp_params->u.get, 0, (sizeof(snmpadapter_get_record)));

	snmp_params->type = SNMPADAPTER_TYPE_GET;
	printf("[SNMPADAPTER] extract_snmp_get_params() :snmp_params->type : %d\n",snmp_params->type);

	oidArray = cJSON_GetObjectItem(request, OID_ARRAY_NAME);
	oidCount = cJSON_GetArraySize(oidArray);
	snmp_params->u.get->count = oidCount;
	printf("[SNMPADAPTER] extract_snmp_get_params() :snmp_params->u.get->count : %d\n",snmp_params->u.get->count);

	for (i = 0; i < oidCount; i++)
	{
		char* str = cJSON_GetArrayItem(oidArray, i)->valuestring;
		size_t len = strlen(str);

		//NOTE : Caller free snmp_params->u.get>oid[i] by calling free_snmp_record()
		snmp_params->u.get->oid[i] = (char *) malloc(sizeof(char)*(len+1));
		strncpy(snmp_params->u.get->oid[i], str, len+1);
		printf("[SNMPADAPTER] extract_snmp_get_params() :snmp_params->u.get->oid[%d] : %s\n", i, snmp_params->u.get->oid[i]);
	}

	return 0;
}


int extract_snmp_set_params(cJSON *request, snmpadapter_record* snmp_params)
{

	return 0;
}

