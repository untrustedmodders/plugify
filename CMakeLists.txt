cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

if(POLICY CMP0092)
    cmake_policy(SET CMP0092 NEW) # Don't add -W3 warning level by default.
endif()


project(plugify VERSION 1.0.0.0  DESCRIPTION "Plugify Core" HOMEPAGE_URL "https://github.com/untrustedmodders/plugify" LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

include(cmake/git.cmake)

# ------------------------------------------------------------------------------
# Build options for tests
option(PLUGIFY_BUILD_TESTS "Build all tests" OFF)
option(PLUGIFY_BUILD_FUNCTION "Build function library" OFF)

# ------------------------------------------------------------------------------
# Compilation options
option(PLUGIFY_BUILD_OBJECT_LIB "Build plugify as object library" OFF)
option(PLUGIFY_BUILD_SHARED_LIB "Build plugify as shared library" ON)
option(PLUGIFY_BUILD_SHARED_ASMJIT "Build asmjit as shared library" OFF)
option(PLUGIFY_BUILD_SHARED_CURL "Build curl as shared library" OFF)

option(PLUGIFY_USE_EXTERNAL_ASMJIT "Use external asmjit library" OFF)
option(PLUGIFY_USE_EXTERNAL_GLAZE "Use external glaze library" OFF)
option(PLUGIFY_USE_EXTERNAL_CURL "Use external curl library" ON)
option(PLUGIFY_USE_EXTERNAL_FMT "Use external fmt library" OFF)

option(PLUGIFY_LOGGING "Enable logs" ON)
option(PLUGIFY_DEBUG "Enable debuging mode (asserts)" ON)
option(PLUGIFY_OPENSSL "Enable OpenSSL for Curl's SSL/TLS" ON)

# ------------------------------------------------------------------------------
# Asmjit
if(NOT PLUGIFY_USE_EXTERNAL_ASMJIT)
    set(ASMJIT_STATIC $<BOOL:${PLUGIFY_BUILD_SHARED_ASMJIT}> CACHE BOOL "")

    add_subdirectory(external/asmjit)
endif()

# ------------------------------------------------------------------------------
# Glaze
if(NOT PLUGIFY_USE_EXTERNAL_GLAZE)
    add_subdirectory(external/glaze)
endif()

# ------------------------------------------------------------------------------
# Curl
if(NOT WIN32 AND NOT PLUGIFY_USE_EXTERNAL_CURL)
    if(PLUGIFY_BUILD_SHARED_CURL)
        set(BUILD_SHARED_LIBS ON CACHE BOOL "")
        set(BUILD_STATIC_LIBS OFF CACHE BOOL "")
    else()
        set(BUILD_SHARED_LIBS OFF CACHE BOOL "")
        set(BUILD_STATIC_LIBS ON CACHE BOOL "")
    endif()
    set(CURL_USE_OPENSSL ${PLUGIFY_OPENSSL} CACHE BOOL "")
    add_subdirectory(external/curl)
endif()

# ------------------------------------------------------------------------------
# Picosha2 & Miniz
add_subdirectory(external/picosha2)
add_subdirectory(external/miniz)

# ------------------------------------------------------------------------------
# Format
#include(CheckIncludeFileCXX)
#check_include_file_cxx("format" CXX_FORMAT_SUPPORT)
try_compile(CXX_FORMAT_SUPPORT ${CMAKE_BINARY_DIR}/format_check SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/test/format_check.cpp)
if(NOT CXX_FORMAT_SUPPORT AND NOT PLUGIFY_USE_EXTERNAL_FMT)
    add_subdirectory(external/fmt)
endif()

# ------------------------------------------------------------------------------
# Plugify
file(GLOB_RECURSE PLUGIFY_CORE_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "src/*.cpp")

if(NOT PLUGIFY_BUILD_OBJECT_LIB)
    if(PLUGIFY_BUILD_SHARED_LIB)
        add_library(${PROJECT_NAME} SHARED ${PLUGIFY_CORE_SOURCES})
    else()
        add_library(${PROJECT_NAME} STATIC ${PLUGIFY_CORE_SOURCES})
    endif()
else()
    add_library(${PROJECT_NAME} OBJECT ${PLUGIFY_CORE_SOURCES})
endif()

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_precompile_headers(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/pch.h)

# ------------------------------------------------------------------------------
# Glaze
if(PLUGIFY_USE_EXTERNAL_GLAZE)
	find_package(glaze REQUIRED)
    target_link_libraries(${PROJECT_NAME} PRIVATE glaze::glaze)
else()
    target_link_libraries(${PROJECT_NAME} PRIVATE glaze_glaze)
endif()

# ------------------------------------------------------------------------------
# Asmjit
if(PLUGIFY_USE_EXTERNAL_ASMJIT)
    find_package(asmjit REQUIRED)
    target_link_libraries(${PROJECT_NAME} PRIVATE asmjit::asmjit)
else()
    target_link_libraries(${PROJECT_NAME} PRIVATE asmjit)
    target_include_directories(${PROJECT_NAME} PRIVATE ${ASMJIT_SRC})

    if(MSVC)
        target_compile_options(asmjit PRIVATE /wd5054)
    else()
        if(MINGW)
            target_compile_options(asmjit PRIVATE -Wno-deprecated-enum-enum-conversion)
        else()
            target_compile_options(asmjit PRIVATE -Wno-deprecated-anon-enum-enum-conversion)
        endif()
        target_compile_options(asmjit PRIVATE -Wno-deprecated-declarations -Wno-error -Wno-pedantic -Wno-shadow)
    endif()
endif()

# ------------------------------------------------------------------------------
# Http
if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE winhttp.lib)
else()
    if(PLUGIFY_USE_EXTERNAL_CURL)
        find_package(CURL REQUIRED)
        target_link_libraries(${PROJECT_NAME} PRIVATE CURL::libcurl)
        message(STATUS "Found CURL version: ${CURL_VERSION_STRING}")
        message(STATUS "Using CURL include dir(s): ${CURL_INCLUDE_DIRS}")
        message(STATUS "Using CURL lib(s): ${CURL_LIBRARIES}")
    else()
        if(PLUGIFY_BUILD_SHARED_CURL)
            target_link_libraries(${PROJECT_NAME} PRIVATE libcurl_shared)
        else()
            target_link_libraries(${PROJECT_NAME} PRIVATE libcurl_static)
        endif()
        target_include_directories(${PROJECT_NAME} PRIVATE external/curl/include)
    endif()
endif()

# ------------------------------------------------------------------------------
# Picosha2 & Miniz
target_link_libraries(${PROJECT_NAME} PRIVATE picosha2 miniz)

include(CheckCCompilerFlag)
check_c_compiler_flag("-fPIC" COMPILER_SUPPORTS_FPIC)
if(COMPILER_SUPPORTS_FPIC)
    target_compile_options(miniz PRIVATE -fPIC)
endif()

# ------------------------------------------------------------------------------
# Format
if(NOT CXX_FORMAT_SUPPORT)
    if(PLUGIFY_USE_EXTERNAL_FMT)
        find_package(fmt REQUIRED)
        target_link_libraries(${PROJECT_NAME} PUBLIC fmt::fmt)
    else()
        target_link_libraries(${PROJECT_NAME} PUBLIC fmt)
    endif()
endif()

# ------------------------------------------------------------------------------
# System
target_link_libraries(${PROJECT_NAME} PRIVATE git-info)

include(GenerateExportHeader)
generate_export_header(${PROJECT_NAME}
        BASE_NAME PLUGIFY
        EXPORT_MACRO_NAME PLUGIFY_API
        NO_EXPORT_MACRO_NAME PLUGIFY_PRIVATE
        EXPORT_FILE_NAME ${CMAKE_BINARY_DIR}/exports/${PROJECT_NAME}_export.h
        STATIC_DEFINE PLUGIFY_STATIC
        )
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_BINARY_DIR}/exports)

if(PLUGIFY_BUILD_OBJECT_LIB)
    target_compile_definitions(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}_EXPORTS)
endif()

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
    if(NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_compile_options(${PROJECT_NAME} PRIVATE /MP)
    endif()
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wextra -Wshadow -Wconversion -Wpedantic -Werror)
endif()

if(WIN32)
    set(PLUGIFY_PLATFORM "windows")
elseif(APPLE)
    set(PLUGIFY_PLATFORM "apple")
elseif(UNIX)
    set(PLUGIFY_PLATFORM "linux")
endif()

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(PLUGIFY_ARCH_X86 64)
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(PLUGIFY_ARCH_X86 32)
endif()

target_compile_definitions(${PROJECT_NAME} PUBLIC
        PLUGIFY_FORMAT_SUPPORT=$<BOOL:${CXX_FORMAT_SUPPORT}>
        )

target_compile_definitions(${PROJECT_NAME} PRIVATE
        PLUGIFY_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
        PLUGIFY_VERSION_MINOR=${PROJECT_VERSION_MINOR}
        PLUGIFY_VERSION_PATCH=${PROJECT_VERSION_PATCH}
        PLUGIFY_VERSION_TWEAK=${PROJECT_VERSION_TWEAK}
        PLUGIFY_PLATFORM="${PLUGIFY_PLATFORM}"
        PLUGIFY_PLATFORM_WINDOWS=$<BOOL:${WIN32}>
        PLUGIFY_PLATFORM_APPLE=$<BOOL:${APPLE}>
        PLUGIFY_PLATFORM_LINUX=$<BOOL:${UNIX}>
        PLUGIFY_ARCH_X86=${PLUGIFY_ARCH_X86}
        PLUGIFY_LOGGING=$<BOOL:${PLUGIFY_LOGGING}>
        PLUGIFY_DEBUG=$<BOOL:${PLUGIFY_DEBUG}>
        PLUGIFY_COMPILED_COMPILER="${CMAKE_CXX_COMPILER}"
        PLUGIFY_COMPILED_SYSTEM="${CMAKE_SYSTEM}"
        PLUGIFY_COMPILED_GENERATOR="${CMAKE_GENERATOR}"
        PLUGIFY_MODULE_SUFFIX="${CMAKE_SHARED_LIBRARY_SUFFIX}"
        PLUGIFY_MODULE_PREFIX="${CMAKE_SHARED_LIBRARY_PREFIX}")

if (PLUGIFY_BUILD_FUNCTION)
    add_library(${PROJECT_NAME}-function OBJECT "${CMAKE_CURRENT_SOURCE_DIR}/include/plugify/function.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/include/plugify/function.h")
    target_include_directories(${PROJECT_NAME}-function PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
    if(PLUGIFY_USE_EXTERNAL_ASMJIT)
        target_link_libraries(${PROJECT_NAME}-function PRIVATE asmjit::asmjit)
    else()
        target_link_libraries(${PROJECT_NAME}-function PRIVATE asmjit)
        target_include_directories(${PROJECT_NAME}-function PRIVATE ${ASMJIT_SRC})
    endif()
    target_compile_definitions(${PROJECT_NAME}-function PRIVATE
            PLUGIFY_PLATFORM_WINDOWS=$<BOOL:${WIN32}>
            PLUGIFY_PLATFORM_APPLE=$<BOOL:${APPLE}>
            PLUGIFY_PLATFORM_LINUX=$<BOOL:${UNIX}>
            PLUGIFY_ARCH_X86=${PLUGIFY_ARCH_X86})
endif()

if(PLUGIFY_BUILD_TESTS)
    add_subdirectory(test/plug)
endif()
