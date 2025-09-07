cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

if(POLICY CMP0092)
    cmake_policy(SET CMP0092 NEW) # Don't add -W3 warning level by default.
endif()


project(containers VERSION 1.0.0.0  DESCRIPTION "Plugify Containers Test" HOMEPAGE_URL "https://github.com/untrustedmodders/plugify" LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchCatch2)

enable_testing()

#
# Plug
#
file(GLOB_RECURSE TESTS_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.cpp")

add_executable(${PROJECT_NAME} ${TESTS_SOURCES} ${Catch2_SOURCE_DIR}/extras/catch_amalgamated.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE plugify::plugify Catch2::Catch2WithMain)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${Catch2_SOURCE_DIR}/extras)

target_compile_definitions(${PROJECT_NAME} PRIVATE TEST_VARIANT_HAS_NO_REFERENCES=1)

if(NOT COMPILER_SUPPORTS_FORMAT)
    target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt-header-only)
endif()

include(CTest)
include(Catch)
catch_discover_tests(${PROJECT_NAME})

target_compile_options(${PROJECT_NAME} PRIVATE ${PLUGIFY_COMPILE_OPTIONS})