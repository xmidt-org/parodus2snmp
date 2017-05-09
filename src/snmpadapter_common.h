#ifndef _SNMPADAPTER_COMMON_H_
#define _SNMPADAPTER_COMMON_H_

/*----------------------------------------------------------------------------*/
/*                                   Macros                                   */
/*----------------------------------------------------------------------------*/

/*
#define COMCAST_COMMUNITY_TOKEN "hDaFHJG7"
#define COMCAST_COMMUNITY_CMD "-c"
#define SNMPADAPTER_SUPPORTED_VERSION  "-v2c"
#define TARGET_AGENT  "10.255.244.168"
*/


#ifndef RUN_ON_TARGET_GW
#define SNMPADAPTER_PARODUS_URL "tcp://127.0.0.1:6666"
#endif

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
