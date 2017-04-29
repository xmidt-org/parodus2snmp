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
#include <time.h>
#include <unistd.h>

#include <string.h>
#include <math.h>
#include <pthread.h>

#include <libparodus.h>
#include "snmpadapter_main.h"
#include "snmpadapter_common.h"

/*----------------------------------------------------------------------------*/
/*                                   Macros                                   */
/*----------------------------------------------------------------------------*/
#define SNMPADAP_MAX_ARGS 		128
#define CONTENT_TYPE_JSON       "application/json"
#define DEVICE_PROPS_FILE   	"/etc/device.properties"
#define CLIENT_PORT_NUM     	6667
#define URL_SIZE 	    		64

// Enable this before integrating to RDKB gerrit */
//#define RUN_ON_TARGET_GW	1

/*----------------------------------------------------------------------------*/
/*                            File Scoped Variables                           */
/*----------------------------------------------------------------------------*/

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
	char atom_ip[URL_SIZE] = { '\0' };

	if (NULL != fp)
	{
		char str[255] = { '\0' };
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
	strncpy(parodus_url, "tcp://192.168.101.1:6666", URL_SIZE);
	snprintf(client_url, URL_SIZE, "tcp://%s:%d", "192.168.101.3", CLIENT_PORT_NUM);
	printf("RUN_ON_TARGET_GW: parodus_url is %s\n", parodus_url);
	printf("RUN_ON_TARGET_GW: client_url is %s\n", client_url);
#endif

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
			printf("Init for parodus Success..!!\n");
			break;
		}
		else
		{
			printf("Init for parodus failed: '%s'\n", libparodus_strerror(ret));
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
		printf("[SNMPADAPTER] Error parsing getargs param !\n");
		return 1;
	}

	pargv[(*pargc)++] = token;

	while ((token = strtok(NULL, " \t\n\r")) && (*pargc < SNMPADAP_MAX_ARGS))
	{
		pargv[(*pargc)++] = token;
	}

	if (token)
	{
		printf("[SNMPADAPTER] Exceeded Max allowed tokens : %d \n", SNMPADAP_MAX_ARGS);
	}

	return 0; // success
}

/*
 * snmpadapter_handle_request
 */
static int snmpadapter_handle_request(char* request, char *transactionId, char **response)
{
	printf("[SNMPADAPTER] snmpcmd: %s\n", request);

	int argc = 0;
	char* argv[SNMPADAP_MAX_ARGS] = { };

	getargs(request, &argc, argv);

	if (argc == 0 || argv[0] == NULL)
	{
		printf("[SNMPADAPTER] getargs() - could't parse arguments !\n");
		return 1; //error
	}

	printf("[SNMPADAPTER] argc: %d\n", argc);

	int cnt = argc, i = 0;
	while (cnt--)
	{
		printf("[SNMPADAPTER] argv[%d] : %s\n", i, argv[i]);
		i++;
	}

	//find appropriate adapter method to call
	if (strstr(argv[0], SNMPADAP_GET) != NULL)
	{
		// call snmp adapter get
		snmp_adapter_get(argc, argv);

		// return back value string

	}
	else if (strstr(argv[0], SNMPADAP_SET) != NULL)
	{
		// call snmp adapter set
		snmp_adapter_set(argc, argv);

		// return back success/error response
	}

	return 0;
}

/*
 * diff_time
 */
static long diff_time(struct timespec *starttime, struct timespec *finishtime)
{
	long msec;
	msec=(finishtime->tv_sec-starttime->tv_sec)*1000;
	msec+=(finishtime->tv_nsec-starttime->tv_nsec)/1000000;
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
			printf("Libparodus failed to recieve message: '%s'\n", libparodus_strerror(rtn));
			sleep(5);
			continue;
		}

		if (wrp_msg->msg_type == WRP_MSG_TYPE__REQ)
		{
			res_wrp_msg = (wrp_msg_t *) malloc(sizeof(wrp_msg_t));
			memset(res_wrp_msg, 0, sizeof(wrp_msg_t));

			clock_gettime(CLOCK_REALTIME, startPtr);

			//processRequest((char *) wrp_msg->u.req.payload, wrp_msg->u.req.transaction_uuid, ((char **) (&(res_wrp_msg->u.req.payload))));
			snmpadapter_handle_request((char *) wrp_msg->u.req.payload, wrp_msg->u.req.transaction_uuid, ((char **) (&(res_wrp_msg->u.req.payload))));

			printf("Response payload is %s\n", (char *) (res_wrp_msg->u.req.payload));
			res_wrp_msg->u.req.payload_size = strlen(res_wrp_msg->u.req.payload);
			res_wrp_msg->msg_type = wrp_msg->msg_type;
			res_wrp_msg->u.req.source = wrp_msg->u.req.dest;
			res_wrp_msg->u.req.dest = wrp_msg->u.req.source;
			res_wrp_msg->u.req.transaction_uuid = wrp_msg->u.req.transaction_uuid;

			contentType = (char *) malloc(sizeof(char) * (strlen(CONTENT_TYPE_JSON) + 1));
			strncpy(contentType, CONTENT_TYPE_JSON, strlen(CONTENT_TYPE_JSON) + 1);
			res_wrp_msg->u.req.content_type = contentType;

			int sendStatus = libparodus_send(current_instance, res_wrp_msg);
			printf("sendStatus is %d\n", sendStatus);
			if (sendStatus == 0)
			{
				printf("Sent message successfully to parodus\n");
			}
			else
			{
				printf("Failed to send message: '%s'\n", libparodus_strerror(sendStatus));
			}

			clock_gettime(CLOCK_REALTIME, endPtr);
			printf("Elapsed time : %ld ms\n", diff_time(startPtr, endPtr));
			wrp_free_struct(res_wrp_msg);
		}
		free(wrp_msg);
	}

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
	 */
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
	/*
	 *  END TESTS
	 */


	/*
	 * Main
	 */

	connect_to_parodus();
	send_receive_from_parodus();

	return 1;
}

