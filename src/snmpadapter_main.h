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
 * snmpadapter_main.h
 *
 *  Created on: Apr 20, 2017
 *      Author: Murugan Viswanathan
 */

#ifndef PARODUS2SNMP_SRC_SNMPADAPTER_PARSER_H_
#define PARODUS2SNMP_SRC_SNMPADAPTER_PARSER_H_

/*----------------------------------------------------------------------------*/
/*                                   Macros                                   */
/*----------------------------------------------------------------------------*/

#define SNMPADAPTER_MAX_ARGS        150
/* #define CONTENT_TYPE_JSON        "application/json" */
#define CONTENT_TYPE_ASCII          "text/plain; charset=us-ascii"
#define DEVICE_PROPS_FILE           "/etc/device.properties"
#define CLIENT_PORT_NUM             6667
#define URL_SIZE                    64

/*
 * SNMP commands
 */
#define SNMPADAPTER_TRANSLATE 	"snmptranslate"
#define SNMPADAPTER_GET 		"snmpget"
#define SNMPADAPTER_GETNEXT 	"snmpgetnext"
#define SNMPADAPTER_WALK 		"snmpwalk"
#define SNMPADAPTER_TABLE 		"snmptable"
#define SNMPADAPTER_SET 		"snmpset"
#define SNMPADAPTER_BULKGET 	"snmpbulkget"
#define SNMPADAPTER_BULKWALK 	"snmpbulkwalk"
#define SNMPADAPTER_TRAP 		"snmptrap"

/*----------------------------------------------------------------------------*/
/*                               Data Structures                              */
/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/*                            File Scoped Variables                           */
/*----------------------------------------------------------------------------*/
/* none */

/*----------------------------------------------------------------------------*/
/*                             Function Prototypes                            */
/*----------------------------------------------------------------------------*/
/* none */

/*----------------------------------------------------------------------------*/
/*                             External Functions                             */
/*----------------------------------------------------------------------------*/
/*
 * none
 */

/*----------------------------------------------------------------------------*/
/*                             Internal functions                             */
/*----------------------------------------------------------------------------*/
/* none */

#endif /* PARODUS2SNMP_SRC_SNMPADAPTER_MAIN_H_ */

