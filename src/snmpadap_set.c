
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

#include "snmpadap_common.h"


static int quiet = 0;

static void optProc(int argc, char *const *argv, int opt)
{
    switch (opt) {
    case 'C':
        while (*optarg) {
            switch (*optarg++) {
            case 'q':
                quiet = 1;
                break;

            default:
                fprintf(stderr, "Unknown flag passed to -C: %c\n",
                        optarg[-1]);
                return;
            }
        }
    }
}

int snmp_adapter_set(int numargs, char *pargs[])
{
    netsnmp_session session, *ss;
    netsnmp_pdu    *pdu, *response = NULL;
    netsnmp_variable_list *vars;
    int             arg;
    int             count;
    int             current_name = 0;
    int             current_type = 0;
    int             current_value = 0;
    char           *names[SNMP_MAX_CMDLINE_OIDS];
    char            types[SNMP_MAX_CMDLINE_OIDS];
    char           *values[SNMP_MAX_CMDLINE_OIDS];
    oid             name[MAX_OID_LEN];
    size_t          name_length;
    int             status;
    int             failures = 0;
    int             exitval = 0;

    putenv(strdup("POSIXLY_CORRECT=1"));

    /*
     * get the common command line arguments
     */
    switch (arg = snmp_parse_args(numargs, pargs, &session, "C:", optProc))
    {
    case NETSNMP_PARSE_ARGS_ERROR:
    case NETSNMP_PARSE_ARGS_ERROR_USAGE:
        return 1;
    case NETSNMP_PARSE_ARGS_SUCCESS_EXIT:
        return 0;
    default:
        break;
    }

    if (arg >= numargs)
    {
        fprintf(stderr, "Missing object name\n");
        return 1;
    }
    if ((numargs - arg) > 3*SNMP_MAX_CMDLINE_OIDS)
    {
        fprintf(stderr, "Too many assignments specified. ");
        fprintf(stderr, "Only %d allowed in one request.\n", SNMP_MAX_CMDLINE_OIDS);
        return 1;
    }

    /*
     * get object names, types, and values
     */
    for (; arg < numargs; arg++)
    {
        names[current_name++] = pargs[arg++];
        if (arg < numargs) {
            switch (*pargs[arg]) {
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
                fprintf(stderr, "%s: Bad object type: %c\n", pargs[arg - 1],
                        *pargs[arg]);
                return 1;
            }
        }
        else
        {
            fprintf(stderr, "%s: Needs type and value\n", pargs[arg - 1]);
            return 1;
        }
        if (arg < numargs)
            values[current_value++] = pargs[arg];
        else
        {
            fprintf(stderr, "%s: Needs value\n", pargs[arg - 2]);
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
        return 1;
    }

    /*
     * do the request
     */
    status = snmp_synch_response(ss, pdu, &response);

    if (status == STAT_SUCCESS)
    {
        if (response->errstat == SNMP_ERR_NOERROR)
        {
            if (!quiet)
            {
                for (vars = response->variables; vars;
                     vars = vars->next_variable)
                    print_variable(vars->name, vars->name_length, vars);
            }
        }
        else
        {
            fprintf(stderr, "Error in packet.\nReason: %s\n",
                    snmp_errstring(response->errstat));
            if (response->errindex != 0)
            {
                fprintf(stderr, "Failed object: ");
                for (count = 1, vars = response->variables;
                     vars && (count != response->errindex);
                     vars = vars->next_variable, count++);
                if (vars)
                    fprint_objid(stderr, vars->name, vars->name_length);
                fprintf(stderr, "\n");
            }
            exitval = 2;
        }
    }
    else if (status == STAT_TIMEOUT)
    {
        fprintf(stderr, "Timeout: No Response from %s\n",
                session.peername);
        exitval = 1;
    }
    else
    {   /* status == STAT_ERROR */
        snmp_sess_perror("snmpset", ss);
        exitval = 1;
    }

    if (response)
        snmp_free_pdu(response);
    snmp_close(ss);
    SOCK_CLEANUP;
    return exitval;
}

