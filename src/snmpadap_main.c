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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "snmpadap_main.h"
#include "snmpadap_common.h"

#define SNMPADAP_MAX_ARGS 128

/*----------------------------------------------------------------------------*/
/*                                   Macros                                   */
/*----------------------------------------------------------------------------*/
/* none */

/*----------------------------------------------------------------------------*/
/*                               Data Structures                              */
/*----------------------------------------------------------------------------*/
/* none */

/*----------------------------------------------------------------------------*/
/*                            File Scoped Variables                           */
/*----------------------------------------------------------------------------*/
/* none */

/*----------------------------------------------------------------------------*/
/*                             Function Prototypes                            */
/*----------------------------------------------------------------------------*/
/* none */

/*----------------------------------------------------------------------------*/
/*                             Internal functions                             */
/*----------------------------------------------------------------------------*/
/*
 * getargs
 */
static int getargs(char* str, int* pargc, char** pargv)
{
	if(!str || !pargc || !pargv)
	{
		printf ("[SNMPADAPTER] getargs() : error params!\n");
		return 1;
	}

	*pargc = 0;

	char *token = strtok(str, " \t\n\r");
	if(!token)
	{
		printf("[SNMPADAPTER] Error parsing getargs param !\n");
		return 1;
	}

	pargv[(*pargc)++]  = token;

	while((token = strtok(NULL, " \t\n\r")) && (*pargc < SNMPADAP_MAX_ARGS))
	{
		pargv[(*pargc)++] = token;
	}

	if(token)
	{
		printf("[SNMPADAPTER] Exceeded Max allowed tokens : %d \n", SNMPADAP_MAX_ARGS);
	}

	return 0; // success
}

/*----------------------------------------------------------------------------*/
/*                             External Functions                             */
/*----------------------------------------------------------------------------*/
/*
 * snmpadap_main
 */
int snmpadap_main(char* str)
{
	printf("[SNMPADAPTER] snmpcmd: %s\n", str);

	int argc = 0;
	char* argv[SNMPADAP_MAX_ARGS] = {};

	getargs(str, &argc, argv);

	if(argc == 0 || argv[0] == NULL)
	{
		printf("[SNMPADAPTER] getargs() - could't parse arguments !\n");
		return 1; //error
	}

	printf("[SNMPADAPTER] argc: %d\n", argc);

	int cnt = argc, i = 0;
	while(cnt--)
	{
		printf("[SNMPADAPTER] argv[%d] : %s\n", i, argv[i]);
		i++;
	}

	//find appropriate adapter method to call
	if(strstr(argv[0], SNMPADAP_GET) != NULL)
	{
		// call snmp adapter get
		snmp_adapter_get("10.255.244.168", "10.255.244.168 1.3.6.1.4.1.1429.79.6.1.1");

		// return back value string

	}
	else if(strstr(argv[0], SNMPADAP_SET) != NULL)
	{
		// call snmp adapter set
		snmp_adapter_set(argc, argv);

		// return back success/error response
	}

    return 0;
}


/* ------------------------------------------------------------------------------------------------
 * Test
 */
int main()
{
	char* CM_IP = "10.255.244.168";

	char setstr[] = "./snmpset -v2c -c hDaFHJG7 10.255.244.168 1.3.6.1.2.1.69.1.3.8.0 i 2";

	char oid1[] = "1.3.6.1.2.1.69.1.3.8.0";
	char oid2[] = "1.3.6.1.4.1.1429.79.6.1.1";

	//test snmp adapter set
	printf("\n\nsnmp_adapter_set(): \n");
	snmpadap_main(setstr);

	//test snmp adapter get
	printf("\n\nsnmp_adapter_get(): \n");
	snmp_adapter_get(CM_IP, oid1);

	return 1;
}



