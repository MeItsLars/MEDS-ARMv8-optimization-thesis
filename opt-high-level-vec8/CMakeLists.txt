cmake_minimum_required(VERSION 3.10)

project(MEDS_opt)

find_package(OpenSSL REQUIRED)

set(PARAM "toy" CACHE STRING "Parameter set to use")
message("Building for parameter set: ${PARAM}")

set(DEUG OFF CACHE BOOL "Build in debug mode")
if (DEBUG)
    message("Building with debug flags")
    set(CMAKE_C_FLAGS "-g -Wall -Wno-unused-function")
    add_compile_definitions(DEBUG)
else()
    message("Building with release flags")
    set(CMAKE_C_FLAGS "-O3 -Wall -Wno-unused-function")
endif()

set(PROJECT_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src)
set(PROJECT_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)
set(NIST_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include/NIST)

# Manually set all source files
set(COMMON_SOURCES
    ${PROJECT_SOURCE_DIR}/bitstream.c
    ${PROJECT_SOURCE_DIR}/cyclecounter.c
    ${PROJECT_SOURCE_DIR}/keccak/fips202_vec.c
    ${PROJECT_SOURCE_DIR}/keccak/fips202.c
    ${PROJECT_SOURCE_DIR}/matrixmod.c
    ${PROJECT_SOURCE_DIR}/matrixmod_vec.c
    ${PROJECT_SOURCE_DIR}/meds.c
    ${PROJECT_SOURCE_DIR}/meds_vec.c
    ${PROJECT_SOURCE_DIR}/osfreq.c
    ${PROJECT_SOURCE_DIR}/randombytes.c
    ${PROJECT_SOURCE_DIR}/seed.c
    ${PROJECT_SOURCE_DIR}/util.c
    ${PROJECT_SOURCE_DIR}/util_vec.c
    ${PROJECT_SOURCE_DIR}/vec_16x4.c
)

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
    RESULT_VARIABLE PARAM_GEN_RESULT
)
if (NOT ${PARAM_GEN_RESULT} EQUAL 0)
    message(FATAL_ERROR "Failed to generate params.h")
endif()
message("> api.h")
execute_process(
    COMMAND bash "-c" "python ./params.py -a -p ${PARAM} > ${PARAM_INCLUDE_DIR}/api.h"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    RESULT_VARIABLE PARAM_GEN_RESULT
)
if (NOT ${PARAM_GEN_RESULT} EQUAL 0)
    message(FATAL_ERROR "Failed to generate api.h")
endif()

set(INCLUDE_DIRECTORIES ${PROJECT_INCLUDE_DIR} ${PROJECT_INCLUDE_DIR}/keccak ${PARAM_INCLUDE_DIR} ${NIST_INCLUDE_DIR})

##########################################################
# Build library (common source files)
##########################################################
add_library(common_sources STATIC ${COMMON_SOURCES})
target_include_directories(common_sources PUBLIC ${INCLUDE_DIRECTORIES})

add_library(common_sources_with_profiler STATIC ${COMMON_SOURCES})
target_include_directories(common_sources_with_profiler PUBLIC ${INCLUDE_DIRECTORIES})
target_compile_definitions(common_sources_with_profiler PRIVATE PROFILER)

enable_language(C ASM)

message("Adding keccak assembly")
set(ASM_KECCAK_DIR ${PROJECT_SOURCE_DIR}/keccak)
file(GLOB_RECURSE ASM_KECCAK_SOURCES ${ASM_KECCAK_DIR}/*.s)

# Preprocess keccak assembly files
message("Preprocessing keccak assembly files")
set(PREPROCESSED_ASM_KECCAK_DIR ${CMAKE_BINARY_DIR}/preprocessed_asm_keccak)
file(MAKE_DIRECTORY ${PREPROCESSED_ASM_KECCAK_DIR})
foreach(ASM_FILE ${ASM_KECCAK_SOURCES})
    get_filename_component(FILENAME ${ASM_FILE} NAME)
    set(PREPROCESSED_ASM_FILE ${PREPROCESSED_ASM_KECCAK_DIR}/${FILENAME}.s)
    message("Preprocessing ${ASM_FILE} into ${PREPROCESSED_ASM_FILE}")
    execute_process(
        COMMAND bash "-c" "cpp -P -I${PROJECT_INCLUDE_DIR} ${ASM_FILE} > ${PREPROCESSED_ASM_FILE}"
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        RESULT_VARIABLE PREPROCESS_RESULT
    )
    if (NOT ${PREPROCESS_RESULT} EQUAL 0)
        message(FATAL_ERROR "Failed to preprocess ${ASM_FILE}")
    endif()
    list(APPEND PREPROCESSED_ASM_KECCAK_SOURCES ${PREPROCESSED_ASM_FILE})
endforeach()

# Create 'asm_sources' library based on generated assembly files
add_library(asm_sources STATIC ${PREPROCESSED_ASM_KECCAK_SOURCES})

##########################################################
# Build settings
##########################################################
if(DEBUG)
    set(BUILD_DIR ${CMAKE_BINARY_DIR}/${PARAM}/debug)
else()
    set(BUILD_DIR ${CMAKE_BINARY_DIR}/${PARAM}/release)
endif()
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BUILD_DIR})

message("Building into ${BUILD_DIR}")
execute_process(
    COMMAND bash "-c" "mkdir -p ${BUILD_DIR}"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

##########################################################
# Build target: test
##########################################################
message("Adding target: test")
add_executable(test ${PROJECT_SOURCE_DIR}/test.c)
target_include_directories(test PRIVATE ${INCLUDE_DIRECTORIES})
target_link_libraries(test PRIVATE common_sources asm_sources ${OPENSSL_LIBRARIES})

##########################################################
# Build target: KAT_test
##########################################################
message("Adding target: KAT_test")
add_executable(KAT_test ${PROJECT_SOURCE_DIR}/KAT_test.c)
target_include_directories(KAT_test PRIVATE ${INCLUDE_DIRECTORIES})
target_link_libraries(KAT_test PRIVATE common_sources asm_sources ${OPENSSL_LIBRARIES})

##########################################################
# Build target: bench
##########################################################
message("Adding target: bench")
add_executable(bench ${PROJECT_SOURCE_DIR}/bench.c)
target_include_directories(bench PRIVATE ${INCLUDE_DIRECTORIES})
target_link_libraries(bench PRIVATE common_sources asm_sources ${OPENSSL_LIBRARIES})

##########################################################
# Build target: profile
##########################################################
message("Adding target: profile")
add_executable(profile ${PROJECT_SOURCE_DIR}/profile.c)
target_include_directories(profile PRIVATE ${INCLUDE_DIRECTORIES})
target_link_libraries(profile PRIVATE common_sources_with_profiler asm_sources ${OPENSSL_LIBRARIES})
target_compile_definitions(profile PRIVATE PROFILER)

##########################################################
# Build target: benchmark
##########################################################
message("Adding target: benchmark")
add_executable(benchmark ${PROJECT_SOURCE_DIR}/benchmark.c)
target_include_directories(benchmark PRIVATE ${INCLUDE_DIRECTORIES})
target_link_libraries(benchmark PRIVATE common_sources asm_sources ${OPENSSL_LIBRARIES})

##########################################################
# Build target: test_vec
##########################################################
message("Adding target: test_vec")
add_executable(test_vec ${PROJECT_SOURCE_DIR}/test_vec.c)
target_include_directories(test_vec PRIVATE ${INCLUDE_DIRECTORIES})
target_link_libraries(test_vec PRIVATE common_sources asm_sources ${OPENSSL_LIBRARIES})