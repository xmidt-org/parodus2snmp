#ifndef _SNMPADAPTER_COMMON_H_
#define _SNMPADAPTER_COMMON_H_

/*----------------------------------------------------------------------------*/
/*                                   Macros                                   */
/*----------------------------------------------------------------------------*/

#ifndef RUN_ON_TARGET_GW
#define SNMPADAPTER_PARODUS_URL "tcp://127.0.0.1:6666"
#endif

/*
 * Error Types
 */
#define SNMPADAPTER_SUCCESS 				"Success"
#define SNMPADAPTER_NO_ERROR_EXIT 			"No Error"
#define SNMPADAPTER_PARSE_ARGS_ERROR 		"Error Parsing Arguments"
#define SNMPADAPTER_MISSING_PARAM_ERROR 	"Missing OID Type or Value"
#define SNMPADAPTER_MISSING_OBJECT_ERROR 	"Missing Object name"
#define SNMPADAPTER_MAX_OIDS_ERROR 			"Too many Object Identifiers specified"
#define SNMPADAPTER_BAD_OID_TYPE_ERROR 		"Bad Object Identifier Type specified"
#define SNMPADAPTER_SNMP_SESSION_ERROR 		"Error creating SNMP session"
#define SNMPADAPTER_SNMP_TIMEOUT_ERROR 		"Timeout Communicating to SNMP Agent"
#define SNMPADAPTER_SNMP_UNKNOWN_ERROR 		"Unknown Error calling SNMP agent"


/*----------------------------------------------------------------------------*/
/*                             Function Prototypes                            */
/*----------------------------------------------------------------------------*/
/*
 * Adapter Interfaces
 */
/* ------------------------------------------------------------------------------------------------
 * snmp_adapter_get
 *    IN pHost : <IP addr> / Name of the SNMP agent
 *    IN pOID : OID to get
 *    return :
 *    		0 	- success
 *    		-1 	- invalid arguments
 *    		1	- error
 */
int snmp_adapter_send_receive_get(int numargs, char *pargs[], char **response);

/* ------------------------------------------------------------------------------------------------
 * snmp_adapter_set
 *    IN pHost : <IP addr> / Name of the SNMP agent
 *    IN pOID : OID to get
 *    return :
 *    		0 	- success
 *    		-1 	- invalid arguments
 *    		1	- error
 */
int snmp_adapter_send_receive_set(int numargs, char *pargs[], char **response);


#endif /* _SNMPADAPTER_COMMON_H_ */
