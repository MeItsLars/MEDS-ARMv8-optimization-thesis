cmake_minimum_required(VERSION 3.10)

project(MEDS_opt)

find_package(OpenSSL REQUIRED)

option(DEBUG "Build with debug flags" OFF)
if (DEBUG)
    message("Building with debug flags")
    set(CMAKE_C_FLAGS "-g -Wall")
    add_compile_definitions(DEBUG)
else()
    message("Building with release flags")
    set(CMAKE_C_FLAGS "-O3 -Wall")
endif()

if(NOT DEFINED PARAM OR PARAM STREQUAL "")
    set(PARAM "toy")
    message("No parameter set specified, using 'toy' parameter set")
endif()
message("Building for parameter set: ${PARAM}")

set(PROJECT_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src)
set(PROJECT_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)

##########################################################
# Generate relevant header files
##########################################################
set(PARAM_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include/${PARAM})

message("Generating header files into ${PARAM_INCLUDE_DIR}")
execute_process(
    COMMAND bash "-c" "mkdir -p ${PARAM_INCLUDE_DIR}"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

message("> params.h")
execute_process(
    COMMAND bash "-c" "python ./params.py -p ${PARAM} > ${PARAM_INCLUDE_DIR}/params.h"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
message("> api.h")
execute_process(
    COMMAND bash "-c" "python ./params.py -a -p ${PARAM} > ${PARAM_INCLUDE_DIR}/api.h"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

##########################################################
# Build
##########################################################
set(BUILD_DIR ${CMAKE_BINARY_DIR}/${PARAM})

message("Building into ${BUILD_DIR}")
execute_process(
    COMMAND bash "-c" "mkdir -p ${BUILD_DIR}"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

set(INCLUDE_DIRECTORIES ${PROJECT_INCLUDE_DIR} ${PARAM_INCLUDE_DIR})