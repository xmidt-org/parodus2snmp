#ifndef _SNMPADAP_COMMON_H_
#define _SNMPADAP_COMMON_H_

#define COMCASTCOMMUNITYTOKEN "hDaFHJG7"

//Adapter Interfaces
/* ------------------------------------------------------------------------------------------------
 * snmp_adapter_get
 *    IN pHost : <IP addr> / Name of the SNMP agent
 *    IN pOID : OID to get
 *    return :
 *    		0 	- success
 *    		-1 	- invalid arguments
 *    		1	- error
 */
int snmp_adapter_get(int numargs, char *pargs[]);

/* ------------------------------------------------------------------------------------------------
 * snmp_adapter_set
 *    IN pHost : <IP addr> / Name of the SNMP agent
 *    IN pOID : OID to get
 *    return :
 *    		0 	- success
 *    		-1 	- invalid arguments
 *    		1	- error
 */
int snmp_adapter_set(int numargs, char *pargs[]);


#endif /* _SNMPADAP_COMMON_H_ */
