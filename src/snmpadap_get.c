
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <string.h>

#include "snmpadap_common.h"



/* ------------------------------------------------------------------------------------------------
 * snmp_adapter_get
 *    IN pHost : <IP addr> / Name of the SNMP agent
 *    IN pOID : OID to get
 *    return :
 *    		0 	- success
 *    		-1 	- invalid arguments
 *    		1	- error
 */
int snmp_adapter_get(char *pHost, char *pOID)
{
	if(pHost == NULL || pOID == NULL)
	{
		printf("snmp_adapter_get() - Invalid Param!\n");
		return -1;
	}

    netsnmp_session  session = {}, *ss = NULL;
    netsnmp_pdu *pdu = NULL;
    netsnmp_pdu *response = NULL;

    oid theOID[MAX_OID_LEN] = {};
    size_t theOID_len = 0;

    netsnmp_variable_list *vars;
    int status;
    int count = 1;

    /*
     * Initialize the SNMP library
     */
    init_snmp("snmpdemoapp");

    /*
     * Initialize a "session" that defines who we're going to talk to
     */
    snmp_sess_init( &session ); /* defaults */
    session.peername = pHost;

    /* Murugan - For version 3: we need to set up the authentication parameters for talking to the server
     *           since we use v2c, we dont need this for now. */
    /* set the SNMP version number */
    session.version = SNMP_VERSION_2c;

    /* set the SNMPv1 community name used for authentication */
    session.community = COMCASTCOMMUNITYTOKEN;
    session.community_len = strlen(session.community);

    /*
     * Open the session
     */
    SOCK_STARTUP;
    ss = snmp_open(&session);  /* establish the session */
    if (!ss)
    {
      snmp_sess_perror("snmpget", &session);
      SOCK_CLEANUP;
      goto error1;
    }
    
    /*
     * Create the PDU for the data for our request.
     */
    pdu = snmp_pdu_create(SNMP_MSG_GET);
    theOID_len = MAX_OID_LEN;

    if (!snmp_parse_oid(pOID, theOID, &theOID_len))
    {
      snmp_perror(pOID);
      SOCK_CLEANUP;
      goto error1;
    }

    snmp_add_null_var(pdu, theOID, theOID_len);
  
    /*
     * Send the Request out. Its a sync (blocking) call.
     */
    status = snmp_synch_response(ss, pdu, &response);

    /*
     * Process the response.
     */
    if (status == STAT_SUCCESS && response->errstat == SNMP_ERR_NOERROR)
    {
		/*
		 * SUCCESS: Print the result variables
		 */

		for (vars = response->variables; vars; vars = vars->next_variable)
			print_variable(vars->name, vars->name_length, vars);

		/* manipuate the information ourselves */
		for (vars = response->variables; vars; vars = vars->next_variable)
		{
			if (vars->type == ASN_OCTET_STR)
			{
				char *sp = (char *) malloc(1 + vars->val_len);
				memcpy(sp, vars->val.string, vars->val_len);
				sp[vars->val_len] = '\0';
				printf("value #%d is a string: %s\n", count++, sp);
				free(sp);
			}
			else
				printf("value #%d is NOT a string\n", count++);
		}
	}
    else
    {
		/*
		 * FAILURE: print what went wrong!
		 */

		if (status == STAT_SUCCESS)
			fprintf(stderr, "Error in packet\nReason: %s\n", snmp_errstring(response->errstat));
		else if (status == STAT_TIMEOUT)
			fprintf(stderr, "Timeout: No response from %s.\n", session.peername);
		else
			snmp_sess_perror("snmpdemoapp", ss);

	}

    /*
     * Clean up:
     *  1) free the response.
     *  2) close the session.
     */
    if (response)
      snmp_free_pdu(response);

    snmp_close(ss);

    SOCK_CLEANUP;
    return (0);

error1:
	  /* do any local cleanup */
      return (1);
}



