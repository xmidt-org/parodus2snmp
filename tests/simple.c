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
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <CUnit/Basic.h>


#include <wdmp-c.h>

/*----------------------------------------------------------------------------*/
/*                                   Macros                                   */
/*----------------------------------------------------------------------------*/
/* none */

/*----------------------------------------------------------------------------*/
/*                               Data Structures                              */
/*----------------------------------------------------------------------------*/
/* none */

/*----------------------------------------------------------------------------*/
/*                            File Scoped Variables                           */
/*----------------------------------------------------------------------------*/
/* none */

/*----------------------------------------------------------------------------*/
/*                             Function Prototypes                            */
/*----------------------------------------------------------------------------*/
extern int getargs(char* str, int* pargc, char** pargv);
extern void get_parodus_url(char *parodus_url, char *client_url);
extern int snmpadapter_create_command(req_struct* snmpdata, char** command);

/*----------------------------------------------------------------------------*/
/*                             Internal functions                             */
/*----------------------------------------------------------------------------*/
void test_all( void )
{
}

void test_getargs()
{
    //Test - int getargs(char* str, int* pargc, char** pargv)

    printf("\n START test_getargs()...  ");

    char setstr[] = "./snmpset -v2c -c hDaFHJG7 10.255.244.168 1.3.6.1.2.1.69.1.3.8.0 i 2";
    int argc = 0;
    char* argv[150] = { };
    char *pStr = strdup(setstr);
    getargs(pStr, &argc, argv);
    CU_ASSERT_EQUAL(8, argc);
    CU_ASSERT_STRING_EQUAL("./snmpset", argv[0]);
    CU_ASSERT_STRING_EQUAL("-v2c", argv[1]);
    CU_ASSERT_STRING_EQUAL("-c", argv[2]);
    CU_ASSERT_STRING_EQUAL("hDaFHJG7", argv[3]);
    CU_ASSERT_STRING_EQUAL("10.255.244.168", argv[4]);
    CU_ASSERT_STRING_EQUAL("1.3.6.1.2.1.69.1.3.8.0", argv[5]);
    CU_ASSERT_STRING_EQUAL("i", argv[6]);
    CU_ASSERT_STRING_EQUAL("2", argv[7]);
    free(pStr);

    printf("\t END   test_getargs() \n");

}

void test_get_parodus_url()
{
    //Test - void get_parodus_url(char *parodus_url, char *client_url)
    printf("\n START test_get_parodus_url()...  ");

    char parodus_url[64] = { '\0' };
    char client_url[64] = { '\0' };

    get_parodus_url(parodus_url, client_url);

    CU_ASSERT_FALSE(0 == parodus_url[0]);
    CU_ASSERT_FALSE(0 == client_url[0]);

    printf("\t END   test_get_parodus_url() \n");
}

void test_snmpadapter_create_command()
{
    //Test - int snmpadapter_create_command(req_struct* snmpdata, char** command)

    printf("\n START test_snmpadapter_create_command()...  ");

    req_struct Req = {0};
    Req.reqType = GET;
    Req.u.getReq = (get_req_t *)malloc(sizeof(get_req_t));
    CU_ASSERT(NULL != Req.u.getReq);
    Req.u.getReq->paramCnt = 2;
    Req.u.getReq->paramNames[0] = "1.3.6.1.4.1.17270.50.2.3.16.1.2.1"; //SNMPv2-SMI::enterprises.17270.50.2.3.16.1.2.1 = Hex-STRING: 74 86 7A 69 7B A6
    Req.u.getReq->paramNames[1] = "1.3.6.1.4.1.17270.50.2.3.16.1.5.1"; //SNMPv2-SMI::enterprises.17270.50.2.3.16.1.5.1 = STRING: "Murugan-PC"

    char* snmpcommand = NULL;
    int len = snmpadapter_create_command(&Req, &snmpcommand);
    CU_ASSERT(len != 0);
    CU_ASSERT_STRING_EQUAL("snmpget -v2c -c hDaFHJG7 10.255.244.168 1.3.6.1.4.1.17270.50.2.3.16.1.2.1 1.3.6.1.4.1.17270.50.2.3.16.1.5.1", snmpcommand);

    printf("\t END   test_snmpadapter_create_command() \n");
}


void add_suites( CU_pSuite *suite )
{
    *suite = CU_add_suite( "parodus2snmp tests", NULL, NULL );
    CU_add_test( *suite, "Test all", test_all );
    CU_add_test( *suite, "Test getargs method", test_getargs );
    CU_add_test( *suite, "Test get_parodus_url method", test_get_parodus_url );
    CU_add_test( *suite, "Test snmpadapter_create_command method", test_snmpadapter_create_command );
}

/*----------------------------------------------------------------------------*/
/*                             External Functions                             */
/*----------------------------------------------------------------------------*/
int main( void )
{
    unsigned rv = 1;
    CU_pSuite suite = NULL;

    if( CUE_SUCCESS == CU_initialize_registry() ) {
        add_suites( &suite );

        if( NULL != suite ) {
            CU_basic_set_mode( CU_BRM_VERBOSE );
            CU_basic_run_tests();
            printf( "\n" );
            CU_basic_show_failures( CU_get_failure_list() );
            printf( "\n\n" );
            rv = CU_get_number_of_tests_failed();
        }

        CU_cleanup_registry();
    }

    if( 0 != rv ) {
        return 1;
    }

    return 0;
}
