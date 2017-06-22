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
 * main.c
 *
 *  Created on: June 22, 2017
 *      Author: Murugan Viswanathan
 */

#include <stdio.h>
#include <stdlib.h>

#include "snmpadapter_main.h"


/* ------------------------------------------------------------------------------------------------
 * main()
 */
int main( int argc, char **argv)
{
    /*
     *  TESTS
     *
     char setstr[] = "./snmpset -v2c -c hDaFHJG7 10.255.244.168 1.3.6.1.2.1.69.1.3.8.0 i 2";
     char getstr[] = "./snmpget -v2c -c hDaFHJG7 10.255.244.168 1.3.6.1.2.1.69.1.3.8.0";
     char *pStr = NULL;

     //test snmp adapter set
     pStr = setstr;
     SnmpAdapterPrint("\n\nsnmp_adapter_set(): \n");
     snmpadapter_handle_request(pStr, NULL, NULL);

     //test snmp adapter get
     pStr = getstr;
     SnmpAdapterPrint("\n\nsnmp_adapter_get(): \n");
     snmpadapter_handle_request(pStr, NULL, NULL);
     *
     *  END TESTS
     */

    /*
     * Main
     */

    snmpadapter_main();
    return 1;
}

