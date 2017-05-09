/***********************************************************************
 Copyright 1988, 1989, 1991, 1992 by Carnegie Mellon University

 All Rights Reserved

 Permission to use, copy, modify, and distribute this software and its
 documentation for any purpose and without fee is hereby granted,
 provided that the above copyright notice appear in all copies and that
 both that copyright notice and this permission notice appear in
 supporting documentation, and that the name of CMU not be
 used in advertising or publicity pertaining to distribution of the
 software without specific, written prior permission.

 CMU DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 CMU BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 SOFTWARE.
 ******************************************************************/

/*
 * snmpadapter_get.c - snmp adapter to make GET requests to a network entity.
 *
 *  Created on: Apr 15, 2017
 *      Author: Murugan Viswanathan
 */

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/utilities.h>
#include <net-snmp/net-snmp-includes.h>

#include "snmpadapter_common.h"

#define NETSNMP_DS_APP_DONT_FIX_PDUS 0
#define MAX_RESPONSE_BUFFER_SIZE 512

int snmp_adapter_send_receive_get(int numargs, char* pargs[], char **response)
{
	char responsebuffer[MAX_RESPONSE_BUFFER_SIZE] = { };
	netsnmp_session session, *ss;
	netsnmp_pdu *pdu;
	netsnmp_pdu *responsepdu;
	netsnmp_variable_list *vars;
	int arg;
	int count;
	int current_name = 0;
	char *names[SNMP_MAX_CMDLINE_OIDS];
	oid name[MAX_OID_LEN];
	size_t name_length;
	int status;
	int failures = 0;
	int exitval = 0;

	/*
	 * get the command arguments
	 */
	/*
	 printf("[SNMPADAPTER] snmp_adapter_send_receive_get() : numargs: %d\n", numargs);
	 int cnt = numargs, i = 0;
	 while (cnt--)
	 {
	 printf("[SNMPADAPTER] snmp_adapter_send_receive_get() : pargs[%d] : %s\n", i, pargs[i]);
	 i++;
	 }
	 */
	switch (arg = snmp_parse_args(numargs, pargs, &session, "C:", NULL))
	{
		case NETSNMP_PARSE_ARGS_ERROR:
		case NETSNMP_PARSE_ARGS_ERROR_USAGE:
			exit(1);
		case NETSNMP_PARSE_ARGS_SUCCESS_EXIT:
			exit(0);
		default:
			break;
	}

	if (arg >= numargs)
	{
		fprintf(stderr, "[SNMPADAPTER] snmp_adapter_send_receive_get() - Missing object name\n");
		exit(1);
	}

	if ((numargs - arg) > SNMP_MAX_CMDLINE_OIDS)
	{
		fprintf(stderr, "[SNMPADAPTER] snmp_adapter_send_receive_get() - Too many object identifiers specified. ");
		fprintf(stderr, "[SNMPADAPTER] snmp_adapter_send_receive_get() - Only %d allowed in one request.\n", SNMP_MAX_CMDLINE_OIDS);
		exit(1);
	}

	/*
	 * get the object names
	 */
	for (; arg < numargs; arg++)
		names[current_name++] = pargs[arg];

	SOCK_STARTUP;

	/*
	 * Open an SNMP session.
	 */
	ss = snmp_open(&session);
	if (ss == NULL)
	{
		/*
		 * diagnose snmp_open errors with the input netsnmp_session pointer
		 */
		snmp_sess_perror("snmpget", &session);
		SOCK_CLEANUP;
		exit(1);
	}

	/*
	 * Create PDU for GET request and add object names to request.
	 */
	pdu = snmp_pdu_create(SNMP_MSG_GET);
	for (count = 0; count < current_name; count++)
	{
		name_length = MAX_OID_LEN;
		if (!snmp_parse_oid(names[count], name, &name_length))
		{
			snmp_perror(names[count]);
			failures++;
		}
		else
			snmp_add_null_var(pdu, name, name_length);
	}
	if (failures)
	{
		snmp_close(ss);
		SOCK_CLEANUP;
		exit(1);
	}

	/*
	 * Perform the request.
	 *
	 * If the Get Request fails, note the OID that caused the error,
	 * "fix" the PDU (removing the error-prone OID) and retry.
	 */
	retry: status = snmp_synch_response(ss, pdu, &responsepdu);
	if (status == STAT_SUCCESS)
	{
		if (responsepdu->errstat == SNMP_ERR_NOERROR)
		{
			char* p = responsebuffer;
			int len = 0;
			for (vars = responsepdu->variables;
					vars && len < MAX_RESPONSE_BUFFER_SIZE - 1;
					vars = vars->next_variable)
			{
				snprint_variable(p, MAX_RESPONSE_BUFFER_SIZE - len, vars->name, vars->name_length, vars);
				len = strlen(responsebuffer);
				responsebuffer[len] = ',';
				p = responsebuffer + len + 1;

				print_variable(vars->name, vars->name_length, vars);
			}

			*(--p) = '\0'; //null terminate after all vars are copied to responsebuffer
			printf("MURUGAN: responsebuffer=[%s]\n", responsebuffer);

			// malloc response. caller free response after use
			if ((*response = (char*) calloc(len + 1, 1)) != NULL)
			{
				strncpy(*response, responsebuffer, len + 1);
			}
		}
		else
		{
			fprintf(stderr, "[SNMPADAPTER] snmp_adapter_send_receive_get() - Error in packet\nReason: %s\n", snmp_errstring(responsepdu->errstat));

			if (responsepdu->errindex != 0)
			{
				fprintf(stderr, "[SNMPADAPTER] snmp_adapter_send_receive_get() - Failed object: ");
				for (count = 1, vars = responsepdu->variables;
						vars && count != responsepdu->errindex;
						vars = vars->next_variable, count++)
					/*EMPTY*/;
				if (vars)
				{
					fprint_objid(stderr, vars->name, vars->name_length);
				}
				fprintf(stderr, "\n");
			}
			exitval = 2;

			/*
			 * retry if the errored variable was successfully removed
			 */
			if (!netsnmp_ds_get_boolean(NETSNMP_DS_APPLICATION_ID,
			NETSNMP_DS_APP_DONT_FIX_PDUS))
			{
				pdu = snmp_fix_pdu(responsepdu, SNMP_MSG_GET);
				snmp_free_pdu(responsepdu);
				responsepdu = NULL;
				if (pdu != NULL)
				{
					goto retry;
				}
			}
		} /* endif -- SNMP_ERR_NOERROR */

	}
	else if (status == STAT_TIMEOUT)
	{
		fprintf(stderr, "[SNMPADAPTER] snmp_adapter_send_receive_get() - Timeout: No Response from %s.\n", session.peername);
		exitval = 1;

	}
	else
	{ /* status == STAT_ERROR */
		snmp_sess_perror("snmpget", ss);
		exitval = 1;

	} /* endif -- STAT_SUCCESS */

	if (responsepdu)
		snmp_free_pdu(responsepdu);
	snmp_close(ss);
	SOCK_CLEANUP;
	printf("end\n");

	return exitval;
}
/* eof */

