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

/*----------------------------------------------------------------------------*/
/*                                   Macros                                   */
/*----------------------------------------------------------------------------*/
/*
 * SNMP commands
 */
#define SNMPADAP_TRANSLATE 	"snmptranslate"
#define SNMPADAP_GET 		"snmpget"
#define SNMPADAP_GETNEXT 	"snmpgetnext"
#define SNMPADAP_WALK 		"snmpwalk"
#define SNMPADAP_TABLE 		"snmptable"
#define SNMPADAP_SET 		"snmpset"
#define SNMPADAP_BULKGET 	"snmpbulkget"
#define SNMPADAP_BULKWALK 	"snmpbulkwalk"
#define SNMPADAP_TRAP 		"snmptrap"


/*----------------------------------------------------------------------------*/
/*                               Data Structures                              */
/*----------------------------------------------------------------------------*/
/*
 * SNMP Adapter - supported types
 */
typedef enum _SNMPADAP_TYPE
{
	SNMPADAPTYPE_GET = 1,
	SNMPADAPTYPE_SET,

}SNMPADAP_TYPE;


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
