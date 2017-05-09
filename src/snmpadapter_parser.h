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
 * snmpadapter_parser.h
 *
 *  Created on: May 5, 2017
 *      Author: Murugan Viswanathan
 */
#include <cJSON.h>

#ifndef __SNMPADAPTER_PARSER_H__
#define __SNMPADAPTER_PARSER_H__

/*----------------------------------------------------------------------------*/
/*                                   Macros                                   */
/*----------------------------------------------------------------------------*/

#define SNMPADAPTER_MAX_OIDS 128

/*----------------------------------------------------------------------------*/
/*                               Data Structures                              */
/*----------------------------------------------------------------------------*/

/*
 * SNMP Adapter - supported snmp request types
 */
typedef enum _SNMPADAPTER_TYPE
{
	SNMPADAPTER_TYPE_UNDEFINED = 0,
	SNMPADAPTER_TYPE_GET = 1,
	SNMPADAPTER_TYPE_SET

}SNMPADAPTER_TYPE;

/*
 * OIDs
 */
typedef struct _oid_struct
{
    char* oid;
    char* value;
    char type;
} oid_struct;

/*
 * struct to support GET request for OIDs
 */
typedef struct _snmpadapter_get_record
{
    char* oid[SNMPADAPTER_MAX_OIDS];
    int count;
} snmpadapter_get_record;

/*
 * struct to support SNMP SET request for OIDs
 */
typedef struct _snmpadapter_set_record
{
	oid_struct *param;
    int count;
} snmpadapter_set_record;

typedef struct _snmpadapter_record
{
	SNMPADAPTER_TYPE type;
    union
	{
		snmpadapter_get_record *get;
		snmpadapter_set_record *set;
    } u;
} snmpadapter_record;


/*----------------------------------------------------------------------------*/
/*                            File Scoped Variables                           */
/*----------------------------------------------------------------------------*/
/* none */

/*----------------------------------------------------------------------------*/
/*                             Function Prototypes                            */
/*----------------------------------------------------------------------------*/
/* none */

/*----------------------------------------------------------------------------*/
/*                             External Functions                             */
/*----------------------------------------------------------------------------*/
/*
 * none
 */

/*----------------------------------------------------------------------------*/
/*                             Internal functions                             */
/*----------------------------------------------------------------------------*/

void free_snmp_record(snmpadapter_record* rec);

int extract_snmp_get_params(cJSON *request, snmpadapter_record* snmp_params);

int extract_snmp_set_params(cJSON *request, snmpadapter_record* snmp_params);


#endif /* __SNMPADAPTER_PARSER_H__ */
