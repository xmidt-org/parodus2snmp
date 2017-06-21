/*
 * snmpadapter_set.c - snmp adapter to make SET requests to a network entity.
 *
 */
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

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <string.h>

#include "snmpadapter_common.h"

static int quiet = 0;

int snmp_adapter_send_receive_set(int numargs, char *pargs[], char **response)
{

#ifdef SNMPADAPTER_TEST_USINGSTUBS
    //if using stubs to test, don't make net-snmp calls such as
    // snmp_parse_args(), snmp_open(), snmp_pdu_create() or snmp_synch_response()
    // Assume success and return.
    *response = strdup(SNMPADAPTER_TEST_SUCCESS);
    return 0; //
#endif

    netsnmp_session session, *ss;
    netsnmp_pdu *pdu, *responsepdu = NULL;
    netsnmp_variable_list *vars;
    int arg;
    int count;
    int current_name = 0;
    int current_type = 0;
    int current_value = 0;
    char *names[SNMP_MAX_CMDLINE_OIDS];
    char types[SNMP_MAX_CMDLINE_OIDS];
    char *values[SNMP_MAX_CMDLINE_OIDS];
    oid name[MAX_OID_LEN];
    size_t name_length;
    int status;
    int failures = 0;
    int exitval = 0;

    putenv(strdup("POSIXLY_CORRECT=1"));

    /*
     * get the common command line arguments
     */
    switch (arg = snmp_parse_args(numargs, pargs, &session, "C:", NULL))
    {
        case NETSNMP_PARSE_ARGS_ERROR:
        case NETSNMP_PARSE_ARGS_ERROR_USAGE:
            *response = strdup(SNMPADAPTER_PARSE_ARGS_ERROR);
            return 1;
        case NETSNMP_PARSE_ARGS_SUCCESS_EXIT:
            *response = strdup(SNMPADAPTER_NO_ERROR_EXIT);
            return 0;
        default:
            break;
    }

    if (arg >= numargs)
    {
        SnmpAdapterError("[SNMPADAPTER] snmp_adapter_send_receive_set() - Missing object name\n");
        *response = strdup(SNMPADAPTER_MISSING_OBJECT_ERROR);
        return 1;
    }
    if ((numargs - arg) > 3 * SNMP_MAX_CMDLINE_OIDS)
    {
        SnmpAdapterError("[SNMPADAPTER] snmp_adapter_send_receive_set() - Too many assignments specified. ");
        SnmpAdapterError("[SNMPADAPTER] snmp_adapter_send_receive_set() - Only %d allowed in one request.\n", SNMP_MAX_CMDLINE_OIDS);
        *response = strdup(SNMPADAPTER_MAX_OIDS_ERROR);
        return 1;
    }

    /*
     * get object names, types, and values
     */
    for (; arg < numargs; arg++)
    {
        names[current_name++] = pargs[arg++];
        if (arg < numargs)
        {
            switch (*pargs[arg])
            {
                case '=':
                case 'i':
                case 'u':
                case '3':
                case 't':
                case 'a':
                case 'o':
                case 's':
                case 'x':
                case 'd':
                case 'b':
#ifdef NETSNMP_WITH_OPAQUE_SPECIAL_TYPES
                case 'I':
                case 'U':
                case 'F':
                case 'D':
#endif
                    types[current_type++] = *pargs[arg++];
                    break;
                default:
                    SnmpAdapterError("[SNMPADAPTER] snmp_adapter_send_receive_set() - %s: Bad object type: %c\n", pargs[arg - 1], *pargs[arg]);
                    *response = strdup(SNMPADAPTER_BAD_OID_TYPE_ERROR);
                    return 1;
            }
        }
        else
        {
            SnmpAdapterError("[SNMPADAPTER] snmp_adapter_send_receive_set() - %s: Needs type and value\n", pargs[arg - 1]);
            *response = strdup(SNMPADAPTER_MISSING_PARAM_ERROR);
            return 1;
        }
        if (arg < numargs)
            values[current_value++] = pargs[arg];
        else
        {
            SnmpAdapterError("[SNMPADAPTER] snmp_adapter_send_receive_set() - %s: Needs value\n", pargs[arg - 2]);
            *response = strdup(SNMPADAPTER_MISSING_PARAM_ERROR);
            return 1;
        }
    }

    SOCK_STARTUP;

    /*
     * open an SNMP session
     */
    ss = snmp_open(&session);
    if (ss == NULL)
    {
        SOCK_CLEANUP;
        *response = strdup(SNMPADAPTER_SNMP_SESSION_ERROR);
        return 1;
    }

    /*
     * create PDU for SET request and add object names and values to request
     */
    pdu = snmp_pdu_create(SNMP_MSG_SET);
    for (count = 0; count < current_name; count++)
    {
        name_length = MAX_OID_LEN;
        if (snmp_parse_oid(names[count], name, &name_length) == NULL)
        {
            snmp_perror(names[count]);
            failures++;
        }
        else if (snmp_add_var(pdu, name, name_length, types[count], values[count]))
        {
            snmp_perror(names[count]);
            failures++;
        }
    }

    if (failures)
    {
        snmp_close(ss);
        SOCK_CLEANUP;
        *response = strdup(SNMPADAPTER_PARSE_ARGS_ERROR);
        return 1;
    }

    /*
     * do the request
     */
    status = snmp_synch_response(ss, pdu, &responsepdu);

    if (status == STAT_SUCCESS)
    {
        if (responsepdu->errstat == SNMP_ERR_NOERROR)
        {
            *response = strdup(SNMPADAPTER_SUCCESS);
            if (!quiet)
            {
                for (vars = responsepdu->variables; vars;
                        vars = vars->next_variable)
                    print_variable(vars->name, vars->name_length, vars);
            }
        }
        else
        {
            *response = strdup(snmp_errstring(responsepdu->errstat));
            SnmpAdapterError("[SNMPADAPTER] snmp_adapter_send_receive_set() - Error in packet.\nReason: %s\n", snmp_errstring(responsepdu->errstat));
            if (responsepdu->errindex != 0)
            {
                SnmpAdapterError("[SNMPADAPTER] snmp_adapter_send_receive_set() - Failed object: ");
                for (count = 1, vars = responsepdu->variables;
                        vars && (count != responsepdu->errindex);
                        vars = vars->next_variable, count++)
                    ;
                if (vars)
                    fprint_objid(stderr, vars->name, vars->name_length);
                SnmpAdapterError("\n");
            }
            exitval = 2;
        }
    }
    else if (status == STAT_TIMEOUT)
    {
        SnmpAdapterError("[SNMPADAPTER] snmp_adapter_send_receive_set() - Timeout: No Response from %s\n", session.peername);
        *response = strdup(SNMPADAPTER_SNMP_TIMEOUT_ERROR);
        exitval = 1;
    }
    else
    { /* status == STAT_ERROR */
        snmp_sess_perror("snmpset", ss);
        *response = strdup(SNMPADAPTER_SNMP_UNKNOWN_ERROR);
        exitval = 1;
    }

    if (responsepdu)
        snmp_free_pdu(responsepdu);
    snmp_close(ss);
    SOCK_CLEANUP;
    return exitval;
}

