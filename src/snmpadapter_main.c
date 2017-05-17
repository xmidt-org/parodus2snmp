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
 * snmpadapter_main.c
 *
 *  Created on: Apr 20, 2017
 *      Author: Murugan Viswanathan
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include <string.h>
#include <math.h>
#include <pthread.h>

#include <libparodus.h>
#include <wdmp-c.h>
#include "snmpadapter_main.h"
#include "snmpadapter_common.h"
#include "snmpadapter_parser.h"

/*----------------------------------------------------------------------------*/
/*                                   Macros                                   */
/*----------------------------------------------------------------------------*/
#define SNMPADAPTER_MAX_ARGS 		150
/* #define CONTENT_TYPE_JSON       "application/json" */
#define CONTENT_TYPE_ASCII 		"text/plain; charset=us-ascii"
#define DEVICE_PROPS_FILE   	"/etc/device.properties"
#define CLIENT_PORT_NUM     	6667
#define URL_SIZE 	    		64

// Enable this before integrating to RDKB gerrit */
/* #define RUN_ON_TARGET_GW	1 */

/*----------------------------------------------------------------------------*/
/*                            File Scoped Variables                           */
/*----------------------------------------------------------------------------*/
char* COMCAST_COMMUNITY_TOKEN = "hDaFHJG7";
char* COMCAST_COMMUNITY_CMD = "-c";
char* SNMPADAPTER_SUPPORTED_VERSION = "-v2c";
char* TARGET_AGENT = "10.255.244.168";


libpd_instance_t current_instance;
char parodus_url[URL_SIZE] = { '\0' };
char client_url[URL_SIZE] = { '\0' };

/*----------------------------------------------------------------------------*/
/*                             Function Prototypes                            */
/*----------------------------------------------------------------------------*/
/* none */

/*----------------------------------------------------------------------------*/
/*                             Internal functions                             */
/*----------------------------------------------------------------------------*/

/*
 * get_parodus_url
 */
#ifdef RUN_ON_TARGET_GW
static void get_parodus_url(char *parodus_url, char *client_url)
{

	FILE *fp = fopen(DEVICE_PROPS_FILE, "r");
	char atom_ip[URL_SIZE] =
	{	'\0'};

	if (NULL != fp)
	{
		char str[255] =
		{	'\0'};
		while (fscanf(fp, "%s", str) != EOF)
		{
			char *value = NULL;

			if ((value = strstr(str, "PARODUS_URL=")))
			{
				value = value + strlen("PARODUS_URL=");
				strncpy(parodus_url, value, (strlen(str) - strlen("PARODUS_URL=")));
			}

			if ((value = strstr(str, "ATOM_INTERFACE_IP=")))
			{
				value = value + strlen("ATOM_INTERFACE_IP=");
				strncpy(atom_ip, value, (strlen(str) - strlen("ATOM_INTERFACE_IP=")));
			}

		}
	}
	else
	{
		printf("Failed to open device.properties file:%s\n", DEVICE_PROPS_FILE);
	}
	fclose(fp);

	if (0 == parodus_url[0])
	{
		printf("parodus_url is not present in device. properties:%s\n", parodus_url);

	}

	if (0 == atom_ip[0])
	{
		printf("atom_ip is not present in device. properties:%s\n", atom_ip);

	}

	snprintf(client_url, URL_SIZE, "tcp://%s:%d", atom_ip, CLIENT_PORT_NUM);
	printf("client_url formed is %s\n", client_url);
	printf("parodus_url formed is %s\n", parodus_url);

	return;
}
#endif

/*
 * connect_to_parodus
 */
static void connect_to_parodus()
{
	int backoffRetryTime = 0;
	int backoff_max_time = 9;
	int max_retry_sleep;
	//Retry Backoff count shall start at c=2 & calculate 2^c - 1.
	int c = 2;
	int retval = -1;

	pthread_detach(pthread_self());

	max_retry_sleep = (int) pow(2, backoff_max_time) - 1;
	printf("max_retry_sleep is %d\n", max_retry_sleep);

#ifdef RUN_ON_TARGET_GW
	// get values from etc/device properties file on target gateway
	get_parodus_url(parodus_url, client_url);
#else /* hardcoded values - for test */
	strncpy(parodus_url, SNMPADAPTER_PARODUS_URL, URL_SIZE);
	snprintf(client_url, URL_SIZE, "tcp://%s:%d", "127.0.0.1", CLIENT_PORT_NUM);
#endif

	//libpd_cfg_t cfg1 = { .service_name = "iot", .receive = true, .keepalive_timeout_secs = 64, .parodus_url = parodus_url, .client_url = client_url };
	//libpd_cfg_t cfg1 = { .service_name = "snmp", .receive = true, .keepalive_timeout_secs = 64, .parodus_url = parodus_url, .client_url = client_url };
	libpd_cfg_t cfg1 = { .service_name = "config", .receive = true, .keepalive_timeout_secs = 64, .parodus_url = parodus_url, .client_url = client_url };

	printf("libparodus_init with parodus url %s and client url %s\n", cfg1.parodus_url, cfg1.client_url);

	while (1)
	{
		if (backoffRetryTime < max_retry_sleep)
		{
			backoffRetryTime = (int) pow(2, c) - 1;
		}

		printf("New backoffRetryTime value calculated as %d seconds\n", backoffRetryTime);
		int ret = libparodus_init(&current_instance, &cfg1);
		printf("ret is %d\n", ret);
		if (ret == 0)
		{
			printf("Init for parodus Success..!\n");
			break;
		}
		else
		{
			printf("Init for parodus failed: '%s' !!!\n", libparodus_strerror(ret));
			sleep(backoffRetryTime);
			c++;
		}
		retval = libparodus_shutdown(current_instance);
		printf("libparodus_shutdown retval %d\n", retval);
	}
}

/*
 * getargs
 */
static int getargs(char* str, int* pargc, char** pargv)
{
	if (!str || !pargc || !pargv)
	{
		printf("[SNMPADAPTER] getargs() : error params!\n");
		return 1;
	}

	*pargc = 0;

	char *token = strtok(str, " \t\n\r");
	if (!token)
	{
		printf("[SNMPADAPTER] getargs() : Error parsing getargs param !\n");
		return 1;
	}

	pargv[(*pargc)++] = token;

	while ((token = strtok(NULL, " \t\n\r")) && (*pargc < SNMPADAPTER_MAX_ARGS))
	{
		pargv[(*pargc)++] = token;
	}

	if (token)
	{
		printf("[SNMPADAPTER] getargs() : Exceeded Max allowed tokens : %d \n", SNMPADAPTER_MAX_ARGS);
	}

	return 0; // success
}


/*
 * snmpadapter_create_command
 *   - create snmp command to send to net-snmp using snmp data
 *   - call snmpadapter_delete_command() to free allocated data
 *   return the size of allocated command.
 *   return 0 if fail
 */
static int snmpadapter_create_command(req_struct* snmpdata, char** command)
{
	if (!snmpdata || !command)
		return 0; //failed

	char* pstr = NULL;
	int cmdsize = 0;

	/* calculate command size, malloc and copy data*/
	if (snmpdata->reqType == GET)
	{
		if (snmpdata->u.getReq == NULL)
			return 0; //failed

		int statsize = cmdsize = strlen(SNMPADAPTER_GET) + 1
						+ strlen(SNMPADAPTER_SUPPORTED_VERSION) + 1
						+ strlen(COMCAST_COMMUNITY_CMD) + 1
						+ strlen(COMCAST_COMMUNITY_TOKEN) + 1
						+ strlen(TARGET_AGENT) + 1;

		for (int c = 0; c < snmpdata->u.getReq->paramCnt; c++)
		{
			cmdsize += strlen(snmpdata->u.getReq->paramNames[c]) + 1;
		}

		pstr = *command = (char*) malloc (cmdsize + 1);
		if(pstr == NULL)
			return 0;

		snprintf(pstr, statsize+1, "%s %s %s %s %s ",
				SNMPADAPTER_GET,
				SNMPADAPTER_SUPPORTED_VERSION,
				COMCAST_COMMUNITY_CMD,
				COMCAST_COMMUNITY_TOKEN,
				TARGET_AGENT);
		pstr += statsize;

		for (int c = 0; c < snmpdata->u.getReq->paramCnt; c++)
		{
			size_t n = strlen(snmpdata->u.getReq->paramNames[c]) + 1;
			snprintf(pstr, n+1, "%s ", snmpdata->u.getReq->paramNames[c]);
			pstr +=  n;
		}

		*pstr = '\0';
	}
	else if (snmpdata->reqType == SET)
	{
		if (snmpdata->u.setReq == NULL)
			return 0; //failed

		int statsize = cmdsize = strlen(SNMPADAPTER_SET) + 1
						+ strlen(SNMPADAPTER_SUPPORTED_VERSION) + 1
						+ strlen(COMCAST_COMMUNITY_CMD) + 1
						+ strlen(COMCAST_COMMUNITY_TOKEN) + 1
						+ strlen(TARGET_AGENT) + 1;

		printf("snmpdata->u.set->count = %d\n", (int)snmpdata->u.setReq->paramCnt);
		for (int c = 0; c < snmpdata->u.setReq->paramCnt; c++)
		{
			cmdsize += strlen(snmpdata->u.setReq->param[c].name) + 1; //oid name
			cmdsize += sizeof(char) + 1; //oid type
			cmdsize += strlen(snmpdata->u.setReq->param[c].value) + 1; //oid value
		}

		pstr = *command = (char*) malloc (cmdsize + 1);
		if(pstr == NULL)
			return 0;

		snprintf(pstr, statsize+1, "%s %s %s %s %s ",
				SNMPADAPTER_SET,
				SNMPADAPTER_SUPPORTED_VERSION,
				COMCAST_COMMUNITY_CMD,
				COMCAST_COMMUNITY_TOKEN,
				TARGET_AGENT);
		pstr += statsize;

		for (int c = 0; c < snmpdata->u.setReq->paramCnt; c++)
		{
			size_t n = strlen(snmpdata->u.setReq->param[c].name) + 1
					+ sizeof(char) + 1
					+ strlen(snmpdata->u.setReq->param[c].value) + 1;

			// get type
			char tc = snmpadapter_get_snmp_type(snmpdata->u.setReq->param[c].type);

			printf("%d - oid  [%s]\n", c, snmpdata->u.setReq->param[c].name);
			printf("%d - type [%c]\n", c, snmpdata->u.setReq->param[c].type);
			printf("%d - value[%s]\n", c, snmpdata->u.setReq->param[c].value);

			//snprintf(pstr, n+1, "%s %c %s ", snmpdata->u.setReq->param[c].name, *(char*)&(snmpdata->u.setReq->param[c].type), snmpdata->u.setReq->param[c].value);
			snprintf(pstr, n+1, "%s %c %s ", snmpdata->u.setReq->param[c].name, tc, snmpdata->u.setReq->param[c].value);
			pstr +=  n;
		}

		*pstr = '\0';
	}

	return (int)(pstr - *command);
}

static void snmpadapter_delete_command(char* command)
{
	if(command != NULL)
		free(command);

	return;
}

/*
static int snmpadapter_create_command(snmpadapter_record* snmpdata, int* pargc, char** pargv)
{
	if (snmpdata == NULL)
		return 1; //failed

	if (snmpdata->type == SNMPADAPTER_TYPE_GET)
	{
		*pargc = 0;

		pargv[(*pargc)++] = SNMPADAPTER_GET;
		pargv[(*pargc)++] = SNMPADAPTER_SUPPORTED_VERSION;
		pargv[(*pargc)++] = COMCAST_COMMUNITY_CMD;
		pargv[(*pargc)++] = COMCAST_COMMUNITY_TOKEN;
		pargv[(*pargc)++] = TARGET_AGENT;


		if(snmpdata->u.get == NULL)
		{
			*pargc = 0; pargv[0] = NULL;
			return 1; //failed
		}

		for(int c = 0; c < snmpdata->u.get->count && c < SNMPADAPTER_MAX_OIDS; c++)
		{
			pargv[(*pargc)++] = snmpdata->u.get->oid[c];
		}
	}
	else if (snmpdata->type == SNMPADAPTER_TYPE_SET)
	{

	}

	return 0; //success
}
*/

/*
 * snmpadapter_get_snmpdata
 *   - Get SNMP command and data from Request payload
 */
/*
static int snmpadapter_get_snmpdata(char* payload, snmpadapter_record** psnmpdata)
{
	cJSON *json_obj = cJSON_Parse(payload);
	if (json_obj != NULL)
	{
		char* command = get_snmp_command_name(json_obj);

		if (command != NULL)
		{
			if (strcmp(command, "GET") == 0)
			{
				extract_snmp_get_params(json_obj, psnmpdata);
			}
			else if ((strcmp(command, "SET") == 0))
			{
				extract_snmp_set_params(json_obj, psnmpdata);
			}
			else
			{
				printf("[SNMPADAPTER] snmpadapter_get_snmpdata() : Unknown Command : \"%s\"\n", command);
			}
		}
		cJSON_Delete(json_obj);
	}
	else
	{
		printf("[SNMPADAPTER] snmpadapter_get_snmpdata() : Could not parse payload!!\n");
	}

	return 0; //success
}
*/

/*
 * snmpadapter_handle_request
 */
static int snmpadapter_handle_request(char* request, char *transactionId, char **response)
{
	int ret = 0;
	printf("[SNMPADAPTER] snmpadapter_handle_request() : request: %s\n, transactionId = %s\n", request, transactionId);

//	snmpadapter_record* snmpdata = NULL;
//	if(snmpadapter_get_snmpdata(request, &snmpdata))
//		return 1; //fail

	req_struct* snmpdata = NULL;
	wdmp_parse_request(request, &snmpdata);
	if(snmpdata == NULL)
	{
		ret = 1; //failed
		goto exit1;
	}

	char* snmpcommand = NULL;
	int len = snmpadapter_create_command(snmpdata, &snmpcommand);
	if(snmpcommand == NULL || strlen(snmpcommand) == 0 || len == 0)
	{
		ret = 1; //failed
		goto exit1;
	}

	printf("[SNMPADAPTER] snmpadapter_handle_request() : command [%s]\nlength=%d\n", snmpcommand, len);

	int argc = 0;
	char* argv[SNMPADAPTER_MAX_ARGS] = { };
	getargs(snmpcommand, &argc, argv);
	if (argc == 0 || argv[0] == NULL)
	{
		printf("[SNMPADAPTER] snmpadapter_handle_request() : could't parse arguments !\n");
		ret = 1; //failed
		goto exit1;
	}

	printf("[SNMPADAPTER] snmpadapter_handle_request() : argc: %d\n", argc);

	int cnt = argc, i = 0;
	while (cnt--)
	{
		printf("[SNMPADAPTER] snmpadapter_handle_request() : argv[%d] : %s\n", i, argv[i]);
		i++;
	}

	if (strstr(argv[0], SNMPADAPTER_GET) != NULL)
	{
		// call snmp adapter get, return back response
		printf("call snmp_adapter_send_receive_get...\n");
		snmp_adapter_send_receive_get(argc, argv, response);
	}
	else if (strstr(argv[0], SNMPADAPTER_SET) != NULL)
	{
		// call snmp adapter set. return back success/error response
		printf("call snmp_adapter_send_receive_set...\n");
		snmp_adapter_send_receive_set(argc, argv, response);
	}

exit1:
	if(snmpcommand)
		snmpadapter_delete_command(snmpcommand);
	if (snmpdata)
		wdmp_free_req_struct(snmpdata);
	return ret;
}

/*
 * diff_time
 */
static long diff_time(struct timespec *starttime, struct timespec *finishtime)
{
	long msec;
	msec = (finishtime->tv_sec - starttime->tv_sec) * 1000;
	msec += (finishtime->tv_nsec - starttime->tv_nsec) / 1000000;
	return msec;
}

/*
 * send_receive_from_parodus
 */
static void send_receive_from_parodus()
{
	int rtn;
	wrp_msg_t *wrp_msg;
	wrp_msg_t *res_wrp_msg;

	struct timespec start, end, *startPtr, *endPtr;
	startPtr = &start;
	endPtr = &end;
	char *contentType = NULL;

	while (1)
	{
		rtn = libparodus_receive(current_instance, &wrp_msg, 2000);
		if (rtn == 1)
		{
			continue;
		}

		if (rtn != 0)
		{
			printf("[SNMPADAPTER] send_receive_from_parodus() : Libparodus failed to recieve message: '%s'\n", libparodus_strerror(rtn));
			sleep(5);
			continue;
		}

		switch (wrp_msg->msg_type)
		{
			case WRP_MSG_TYPE__REQ:

				printf("[SNMPADAPTER] send_receive_from_parodus() : received WRP_MSG_TYPE__REQ \n");
				res_wrp_msg = (wrp_msg_t *) malloc(sizeof(wrp_msg_t));
				memset(res_wrp_msg, 0, sizeof(wrp_msg_t));

				clock_gettime(CLOCK_REALTIME, startPtr);

				// Process request message and get the response payload to send back
				snmpadapter_handle_request((char *) wrp_msg->u.req.payload, wrp_msg->u.req.transaction_uuid, ((char **) (&(res_wrp_msg->u.req.payload))));

				if(res_wrp_msg->u.req.payload == NULL)
				{
					printf("[SNMPADAPTER] send_receive_from_parodus() : Response payload is NULL !!\n");

					//MURUGAN: TODO : prepare and send appropriate error message in response
					res_wrp_msg->u.req.payload_size = 0;
					continue;
				}
				else
				{
					printf("[SNMPADAPTER] send_receive_from_parodus() : Response payload is %s\n", (char *) (res_wrp_msg->u.req.payload));
					res_wrp_msg->u.req.payload_size = strlen(res_wrp_msg->u.req.payload);
				}

				// Send Response back

				res_wrp_msg->msg_type = wrp_msg->msg_type;
				res_wrp_msg->u.req.source = wrp_msg->u.req.dest;
				res_wrp_msg->u.req.dest = wrp_msg->u.req.source;
				res_wrp_msg->u.req.transaction_uuid = wrp_msg->u.req.transaction_uuid;
				contentType = (char *) malloc(sizeof(char) * (strlen(CONTENT_TYPE_ASCII) + 1));
				strncpy(contentType, CONTENT_TYPE_ASCII, strlen(CONTENT_TYPE_ASCII) + 1);
				res_wrp_msg->u.req.content_type = contentType;

				int sendStatus = libparodus_send(current_instance, res_wrp_msg);
				if (sendStatus == 0)
				{
					printf("[SNMPADAPTER] send_receive_from_parodus() : Sent message successfully to parodus\n");
				}
				else
				{
					printf("[SNMPADAPTER] send_receive_from_parodus() : Failed to send message!! : '%s'\n", libparodus_strerror(sendStatus));
				}

				clock_gettime(CLOCK_REALTIME, endPtr);
				printf("[SNMPADAPTER] send_receive_from_parodus() : Elapsed time : %ld ms\n", diff_time(startPtr, endPtr));

				//free response data structures
				wrp_free_struct(res_wrp_msg);

				break;

			case WRP_MSG_TYPE__CREATE:
			case WRP_MSG_TYPE__RETREIVE:
			case WRP_MSG_TYPE__UPDATE:
			case WRP_MSG_TYPE__DELETE:
				printf("SNMPADAPTER: send_receive_from_parodus() - received CRUD \n");
				break;

			default:
				printf("SNMPADAPTER: send_receive_from_parodus() - received unknown type \n");
				break;
		}

		//free request data structure
		free(wrp_msg);
	}//while (1)

	libparodus_shutdown(current_instance);

	printf("End of parodus_upstream\n");
	return;
}

/* ------------------------------------------------------------------------------------------------
 * main()
 */
int main()
{
/*
 *  TESTS
 *
	char setstr[] = "./snmpset -v2c -c hDaFHJG7 10.255.244.168 1.3.6.1.2.1.69.1.3.8.0 i 2";
	char getstr[] = "./snmpget -v2c -c hDaFHJG7 10.255.244.168 1.3.6.1.2.1.69.1.3.8.0";
	char *pStr = NULL;

	//test snmp adapter set
	pStr = setstr;
	printf("\n\nsnmp_adapter_set(): \n");
	snmpadapter_handle_request(pStr, NULL, NULL);

	//test snmp adapter get
	pStr = getstr;
	printf("\n\nsnmp_adapter_get(): \n");
	snmpadapter_handle_request(pStr, NULL, NULL);
*
*  END TESTS
*/


	/*
	 * Main
	 */

	connect_to_parodus();
	send_receive_from_parodus();

	return 1;
}

