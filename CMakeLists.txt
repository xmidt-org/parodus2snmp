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

cmake_minimum_required(VERSION 2.8.7)

project(parodus2snmp)
include(ExternalProject)
include(CTest)

#add_definitions(-std=c99)
add_definitions(-std=gnu99)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -g -Werror -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c99 -g -Werror -Wall")

set(INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/_install)
set(PREFIX_DIR ${CMAKE_CURRENT_BINARY_DIR}/_prefix)
set(INCLUDE_DIR ${INSTALL_DIR}/include)
set(LIBRARY_DIR ${INSTALL_DIR}/lib)
set(LIBRARY_DIR64 ${INSTALL_DIR}/lib64)
include_directories(${INCLUDE_DIR}
                    ${INCLUDE_DIR}/cjson
                    ${INCLUDE_DIR}/wrp-c                    
                    ${INCLUDE_DIR}/libparodus
                    ${INCLUDE_DIR}/libseshat
                    ${INCLUDE_DIR}/msgpack
                    ${INCLUDE_DIR}/trower-base64
                    ${INCLUDE_DIR}/wdmp-c
                    ${INCLUDE_DIR}/cimplog)
include_directories (SYSTEM /usr/include)

enable_testing()

if(BUILD_TESTING)
add_definitions(-DSNMPADAPTER_TEST_STANDALONE -DSNMPADAPTER_TEST_USINGSTUBS)
endif()

#find net-snmp opensource package 
include(FindPackageNetSNMP.cmake)

if (NOT BUILD_YOCTO)

# libseshat external dependency
#-------------------------------------------------------------------------------
ExternalProject_Add(_libseshat
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/_prefix/libseshat
    GIT_REPOSITORY https://github.com/Comcast/seshat.git
    GIT_TAG "master"
    CMAKE_ARGS += -DCMAKE_INSTALL_PREFIX=${INSTALL_DIR}
                  -DMSGPACK_ENABLE_CXX=OFF
                  -DMSGPACK_BUILD_EXAMPLES=OFF
                  -DBUILD_TESTING=OFF
)
add_library(libseshat STATIC SHARED IMPORTED)
add_dependencies(libseshat _libseshat)


# base64 external dependency
#-------------------------------------------------------------------------------
ExternalProject_Add(_libparodus
    PREFIX ${PREFIX_DIR}/libparodus
    GIT_REPOSITORY https://github.com/Comcast/libparodus.git
    GIT_TAG "master"
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${INSTALL_DIR} -DBUILD_TESTING=OFF)
add_library(libparodus STATIC IMPORTED)
add_dependencies(libparodus _libparodus)


# nanoMsg external dependency
#-------------------------------------------------------------------------------
ExternalProject_Add(nanomsg
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/_prefix/nanomsg
    GIT_REPOSITORY https://github.com/nanomsg/nanomsg.git
    #GIT_TAG "096998834451219ee7813d8977f6a4027b0ccb43"
    CMAKE_ARGS += -DCMAKE_INSTALL_PREFIX=${INSTALL_DIR}
)
add_library(libnanomsg STATIC SHARED IMPORTED)
add_dependencies(libnanomsg nanomsg)


# base64 external dependency
#-------------------------------------------------------------------------------
ExternalProject_Add(trower-base64
    PREFIX ${PREFIX_DIR}/trower-base64
    GIT_REPOSITORY https://github.com/Comcast/trower-base64.git
    GIT_TAG "master"
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${INSTALL_DIR} -DBUILD_TESTING=OFF)
add_library(libtrower-base64 STATIC IMPORTED)
add_dependencies(libtrower-base64 trower-base64)


# msgpack-c external dependency
#-------------------------------------------------------------------------------
ExternalProject_Add(msgpack
    PREFIX ${PREFIX_DIR}/msgpack
    GIT_REPOSITORY https://github.com/msgpack/msgpack-c.git
    GIT_TAG "c6e6dbc608366090c12b142b3832604e6aa12f54"
    CMAKE_ARGS += -DCMAKE_INSTALL_PREFIX=${INSTALL_DIR}
    -DMSGPACK_ENABLE_CXX=OFF
    -DMSGPACK_BUILD_EXAMPLES=OFF -DBUILD_TESTING=OFF)
add_library(libmsgpack STATIC IMPORTED)
add_dependencies(libmsgpack msgpack)


# cJSON external dependency
#-------------------------------------------------------------------------------
ExternalProject_Add(cJSON
    #PREFIX ${PREFIX_DIR}/cJSON
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/_prefix/cJSON
    GIT_REPOSITORY https://github.com/DaveGamble/cJSON.git
    GIT_TAG "aafb64a1c549b7b927e339df6d35b1d5059dc235"
    CMAKE_ARGS += -DCMAKE_INSTALL_PREFIX=${INSTALL_DIR}
)
add_library(libcJSON STATIC SHARED IMPORTED)
add_dependencies(libcJSON cJSON)

# wdmp-c external dependency
#-------------------------------------------------------------------------------
ExternalProject_Add(wdmp-c
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/_prefix/wdmp-c
    #GIT_REPOSITORY https://github.com/MuruganViswanathan/wdmp-c.git
    GIT_REPOSITORY https://github.com/Comcast/wdmp-c.git
    GIT_TAG "master"
    CMAKE_ARGS += -DCMAKE_INSTALL_PREFIX=${INSTALL_DIR}
                  -DBUILD_TESTING=OFF
)
add_library(libwdmp-c STATIC SHARED IMPORTED)
add_dependencies(libwdmp-c wdmp-c)


# wrp-c external dependency
#-------------------------------------------------------------------------------
ExternalProject_Add(wrp-c
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/_prefix/wrp-c
    GIT_REPOSITORY https://github.com/Comcast/wrp-c.git
    GIT_TAG "master"
    CMAKE_ARGS += -DCMAKE_INSTALL_PREFIX=${INSTALL_DIR}
                  -DMSGPACK_ENABLE_CXX=OFF
                  -DMSGPACK_BUILD_EXAMPLES=OFF
                  -DBUILD_TESTING=OFF
)
add_library(libwrp-c STATIC SHARED IMPORTED)
add_dependencies(libwrp-c wrp-c)


# cimplog external dependency
ExternalProject_Add(cimplog
    PREFIX ${PREFIX_DIR}/cimplog
    GIT_REPOSITORY https://github.com/Comcast/cimplog.git
    GIT_TAG "master"
    CMAKE_ARGS += -DCMAKE_INSTALL_PREFIX=${INSTALL_DIR}
                  -DBUILD_TESTING=OFF
)
add_library(libcimplog STATIC IMPORTED)
add_dependencies(libcimplog cimplog)

endif ()

link_directories ( ${LIBRARY_DIR} ${LIBRARY_DIR64} )

add_subdirectory(src)

if (BUILD_TESTING)
	add_subdirectory(tests)
endif (BUILD_TESTING)
