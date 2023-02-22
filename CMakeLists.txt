cmake_minimum_required(VERSION 3.24)
project(psql_parse)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -Wall -Wextra -Wpedantic")

add_compile_options(-fsanitize=undefined)
add_link_options(-fsanitize=undefined)

find_program(MOLD mold)
find_program(LLD lld)
if(MOLD)
    add_compile_options(-fuse-ld=mold)
elseif(LLD)
    add_compile_options(-fuse-ld=lld)
endif()

#[============[
# Dependencies
#]============]

include(FindFLEX)
find_package(FLEX REQUIRED)
message(STATUS "Flex version: ${FLEX_VERSION}")

include(FindBISON)
find_package(BISON REQUIRED)
message(STATUS "Bison version: ${BISON_VERSION}")

include(FetchContent)
FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2
        GIT_TAG        ab6c7375be9a8e71ee84c6f8537113f9f47daf99
)
FetchContent_MakeAvailable(Catch2)


#[=======[
# Headers
#]=======]

include_directories(
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_CURRENT_BINARY_DIR}
)

#[==========[
# Generation
#]==========]

set(SCAN_GRAMMAR "${CMAKE_SOURCE_DIR}/src/scan.l")
set(SCAN_CC "${CMAKE_CURRENT_BINARY_DIR}/scan.cpp")
set(SCAN_H "scan.h")
set(PARSE_GRAMMAR "${CMAKE_SOURCE_DIR}/src/parse.y")
set(PARSE_CC "${CMAKE_CURRENT_BINARY_DIR}/parse.cpp")

bison_target(gen_parser ${PARSE_GRAMMAR} ${PARSE_CC})
flex_target(gen_scanner ${SCAN_GRAMMAR} ${SCAN_CC})

add_flex_bison_dependency(gen_scanner gen_parser)


#[=======[
# Library
#]=======]


set(SOURCES
        ${FLEX_gen_scanner_OUTPUTS}
        ${BISON_gen_parser_OUTPUTS}

        "${CMAKE_SOURCE_DIR}/src/driver.cpp"
        "${CMAKE_SOURCE_DIR}/src/scanner.cpp"

        "${CMAKE_SOURCE_DIR}/src/ast/expr.cpp"
        "${CMAKE_SOURCE_DIR}/src/ast/stmt.cpp"
        "${CMAKE_SOURCE_DIR}/src/ast/create.cpp")


add_library(psql_parse ${SOURCES})

#[=======[
# Testing
#]=======]

set(TEST_SOURCES
        test/test_utils.cpp
        ${CMAKE_SOURCE_DIR}/test/tester.cpp)

add_executable(runtests ${TEST_SOURCES})
target_link_libraries(runtests PRIVATE psql_parse Catch2::Catch2WithMain)

