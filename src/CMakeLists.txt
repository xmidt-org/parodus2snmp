#   Copyright 2017 Comcast Cable Communications Management, LLC
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.

set(PROJ_PARODUS2SNMP parodus2snmp)
file(GLOB SOURCES "*.c")

add_executable(parodus2snmp ${SOURCES})
#add_library(${PROJ_PARODUS2SNMP} STATIC ${HEADERS} ${SOURCES})
#add_library(${PROJ_PARODUS2SNMP}.shared SHARED ${HEADERS} ${SOURCES})
#set_target_properties(${PROJ_PARODUS2SNMP}.shared PROPERTIES OUTPUT_NAME ${PROJ_PARODUS2SNMP})
#set_property(TARGET ${PROJ_PARODUS2SNMP} PROPERTY C_STANDARD 99)
#set_property(TARGET ${PROJ_PARODUS2SNMP}.shared PROPERTY C_STANDARD 99)


target_link_libraries (${PROJ_PARODUS2SNMP}
    ${CMAKE_THREAD_LIBS_INIT}
    -llibparodus
    -lwrp-c
    -lmsgpackc
    -ltrower-base64
    -lnanomsg
    -lcimplog
    -lnetsnmp
    -lrt 
    -lcrypto    
    -lm
    -lcjson
    -lpthread
    -lrt
    )

install (TARGETS ${PROJ_PARODUS2SNMP} DESTINATION bin)
