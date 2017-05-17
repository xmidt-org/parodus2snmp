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

#include <wdmp-c.h>
#include "snmpadapter_parser.h"

/* JSON request /response names */
#define SNMPADAPTER_REQUEST_COMMAND_NAME "command"
#define SNMPADAPTER_OID_ARRAY_NAME "names"
#define SNMPADAPTER_OID_PARAM_ARRAY_NAME "parameters"
#define SNMPADAPTER_OID_PARAM_NAME "name"
#define SNMPADAPTER_OID_TYPE_NAME "dataType"
#define SNMPADAPTER_OID_VALUE_NAME "value"


void snmpadapter_free_snmp_record(snmpadapter_record* rec)
{
	if(rec == NULL)
		return;

	if(rec->type == SNMPADAPTER_TYPE_GET)
	{
		if(rec->u.get != NULL)
		{
			for (int c = 0; c < rec->u.get->count; c++)
			{
				if(rec->u.get->oid[c])
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
		if(rec->u.set != NULL)
		{
			for(int c = 0; c < rec->u.set->count; c++)
			{
				if(rec->u.set->param[c].oid)
					free(rec->u.set->param[c].oid);
				if(rec->u.set->param[c].value)
					free(rec->u.set->param[c].value);
				rec->u.set->param[c].type = 0;
			}

			rec->u.set->count = 0;

			free(rec->u.set);
			rec->u.set = NULL;
		}
	}

	rec->type = 0;
	free(rec);
}

/* Return one of the SNMP types
 * 	i: INTEGER, u: unsigned INTEGER, t: TIMETICKS, a: IPADDRESS
 *	o: OBJID, s: STRING, x: HEX STRING, d: DECIMAL STRING, b: BITS
 *	U: unsigned int64, I: signed int64, F: float, D: double
 *
 */
char snmpadapter_get_snmp_type(DATA_TYPE type)
{
	switch (type)
	{
		case WDMP_STRING:
			return 's';
		case WDMP_INT:
			return 'i';
		case WDMP_UINT:
			return 'u';
		case WDMP_BOOLEAN:
			return 'i';
		case WDMP_DATETIME:
			return 't';
		case WDMP_BASE64:
			return 'U';
		case WDMP_LONG:
			return 'I';
		case WDMP_ULONG:
			return 'U';
		case WDMP_FLOAT:
			return 'F';
		case WDMP_DOUBLE:
			return 'D';
		case WDMP_BYTE:
			return 'i';
		case WDMP_NONE:
			return 's';
	}

	return 's'; //default
}


char* snmpadapter_get_snmp_command_name(cJSON *request)
{
	if(request)
		return cJSON_GetObjectItem(request, SNMPADAPTER_REQUEST_COMMAND_NAME)->valuestring;
	else
		return NULL;
}

/*
 * e.g.
 * json: {"command":"GET","names":["1.3.6.1.2.1.69.1.3.8.0","1.3.6.1.2.1.69.1.3.8.1","1.3.6.1.2.1.69.1.3.8.2"]}
 */
int snmpadapter_extract_snmp_get_params(cJSON *request, snmpadapter_record** psnmp_params)
{
	cJSON *oidArray = NULL;
	int oidCount = 0, i = 0;

	snmpadapter_record* snmp_params = (snmpadapter_record*) malloc(sizeof(snmpadapter_record));
	if(!snmp_params)
		return 1;

	memset(snmp_params, 0, sizeof(snmpadapter_record));
	*psnmp_params = snmp_params; //allocate memory and pass to caller to free

	//NOTE : Caller free snmp_params->u.get by calling free_snmp_record()
	snmp_params->u.get = (snmpadapter_get_record*) malloc(sizeof(snmpadapter_get_record));
	memset(snmp_params->u.get, 0, (sizeof(snmpadapter_get_record)));

	snmp_params->type = SNMPADAPTER_TYPE_GET;
	printf("[SNMPADAPTER] extract_snmp_get_params() :snmp_params->type : %d\n",snmp_params->type);

	oidArray = cJSON_GetObjectItem(request, SNMPADAPTER_OID_ARRAY_NAME);
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

/*
 * e.g.
 * {"command":"SET","parameters":[{"name":"Device.X_RDKCENTRAL-COM_XDNS.DefaultDeviceDnsIPv4","dataType":0,"value":"75.75.75.10"},{"name":"Device.X_RDKCENTRAL-COM_XDNS.DefaultDeviceDnsIPv6","dataType":0,"value":"2001:558:feed::7510"},{"name":"Device.X_RDKCENTRAL-COM_XDNS.DefaultDeviceTag","dataType":0,"value":"Level1_Protected Browsing"}]}
 *
 */
int snmpadapter_extract_snmp_set_params(cJSON *request, snmpadapter_record** psnmp_params)
{
	cJSON *oidParamArray = NULL;
	int oidCount = 0, i = 0;

	snmpadapter_record* snmp_params = (snmpadapter_record*) malloc(sizeof(snmpadapter_record));
	if (!snmp_params)
		return 1;

	memset(snmp_params, 0, sizeof(snmpadapter_record));
	*psnmp_params = snmp_params; //allocate memory and pass to caller to free

	//NOTE : Caller free snmp_params->u.set by calling free_snmp_record()
	snmp_params->u.set = (snmpadapter_set_record*) malloc(sizeof(snmpadapter_set_record));
	memset(snmp_params->u.get, 0, (sizeof(snmpadapter_set_record)));

	snmp_params->type = SNMPADAPTER_TYPE_SET;

	oidParamArray = cJSON_GetObjectItem(request, SNMPADAPTER_OID_PARAM_ARRAY_NAME);

	oidCount = cJSON_GetArraySize(oidParamArray);
	snmp_params->u.set->count = oidCount;

	for (i = 0; i < oidCount; i++)
	{
		char* val = NULL;
		int len = 0;
		cJSON* oidparam = cJSON_GetArrayItem(oidParamArray, i);

		val = cJSON_GetObjectItem(oidparam, SNMPADAPTER_OID_PARAM_NAME)->valuestring;
		len = strlen(val);
		snmp_params->u.set->param[i].oid = (char *) malloc(sizeof(char) * (len + 1));
		strncpy(snmp_params->u.set->param[i].oid, val, len + 1);

		val = cJSON_GetObjectItem(oidparam, SNMPADAPTER_OID_VALUE_NAME)->valuestring;
		len = strlen(val);
		snmp_params->u.set->param[i].value = (char *) malloc(sizeof(char) * (len + 1));
		strncpy(snmp_params->u.set->param[i].value, val, len + 1);

		snmp_params->u.set->param[i].type = cJSON_GetObjectItem(oidparam, SNMPADAPTER_OID_TYPE_NAME)->valueint;
	}

	return 0;
}

