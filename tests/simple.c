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

/*----------------------------------------------------------------------------*/
/*                             Internal functions                             */
/*----------------------------------------------------------------------------*/
void test_all( void )
{
}

void test_getargs()
{
    //Test - static int getargs(char* str, int* pargc, char** pargv)

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

    printf("\t END   test_getargs(). \n");

}

void add_suites( CU_pSuite *suite )
{
    *suite = CU_add_suite( "parodus2snmp tests", NULL, NULL );
    CU_add_test( *suite, "Test all", test_all );
    CU_add_test( *suite, "Test getargs method", test_getargs );
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
