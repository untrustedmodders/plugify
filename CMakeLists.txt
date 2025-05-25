# ------------------------------------------------------------------------------
# Plugify
cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

if(POLICY CMP0092)
    cmake_policy(SET CMP0092 NEW) # Don't add -W3 warning level by default.
endif()


file(READ "${CMAKE_CURRENT_SOURCE_DIR}/version.txt" PLUGIFY_VERSION)
string(STRIP "${PLUGIFY_VERSION}" PLUGIFY_VERSION)
message(STATUS "Plugify Version: ${PLUGIFY_VERSION}")

# version.txt may contain a SemVer of the form 1.2.3-xxx+xxx
# Extract the first 3 numbers into CMake variables
string(REGEX REPLACE "[.+-]" ";" PLUGIFY_VERSION_LIST ${PLUGIFY_VERSION})
list(GET PLUGIFY_VERSION_LIST 0 PLUGIFY_VERSION_MAJOR)
list(GET PLUGIFY_VERSION_LIST 1 PLUGIFY_VERSION_MINOR)
list(GET PLUGIFY_VERSION_LIST 2 PLUGIFY_VERSION_PATCH)

# ------------------------------------------------------------------------------
# Project configuration
project(plugify VERSION "${PLUGIFY_VERSION_MAJOR}.${PLUGIFY_VERSION_MINOR}.${PLUGIFY_VERSION_PATCH}" DESCRIPTION "Plugify Core" HOMEPAGE_URL "https://github.com/untrustedmodders/plugify" LANGUAGES C CXX)

if(NOT CMAKE_BUILD_TYPE MATCHES "Debug|Devel|MinSizeRel|RelWithDebInfo|Release")
    message(STATUS "CMAKE_BUILD_TYPE not set, defaulting to Debug.")
    set(CMAKE_BUILD_TYPE Debug)
endif()

message(VERBOSE "*")
message(VERBOSE "* ${PROJECT_NAME} v${PROJECT_VERSION} (${CMAKE_BUILD_TYPE})")
message(VERBOSE "* Copyright (c) 2023-2025 untrustedmodders <untrustedmodders@gmail.com>")
message(VERBOSE "*")


# ------------------------------------------------------------------------------
# Includes
include(PlugifyUtils)

detect_operating_system()
detect_compiler()
detect_architecture()

include(PlugifyBuildOptions)
include(PlugifyRequirement)

# ------------------------------------------------------------------------------
# Plugify
if(PLUGIFY_INTERFACE)
    file(GLOB_RECURSE PLUGIFY_CORE_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "src/interface/*.cpp")
    set(PLUGIFY_CORE_SOURCES ${PLUGIFY_CORE_SOURCES} "src/core/plugify_provider.cpp" "src/core/plugify_context.cpp")
else()
    file(GLOB_RECURSE PLUGIFY_CORE_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "src/*.cpp")
endif()

if(NOT PLUGIFY_BUILD_OBJECT_LIB)
    if(PLUGIFY_BUILD_SHARED_LIB)
        add_library(${PROJECT_NAME} SHARED ${PLUGIFY_CORE_SOURCES})
    else()
        add_library(${PROJECT_NAME} STATIC ${PLUGIFY_CORE_SOURCES})
    endif()
else()
    add_library(${PROJECT_NAME} OBJECT ${PLUGIFY_CORE_SOURCES})
endif()
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_precompile_headers(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/pch.hpp)

include(PlugifyDependencies)

# ------------------------------------------------------------------------------

if(PLUGIFY_BUILD_OBJECT_LIB)
    target_compile_definitions(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}_EXPORTS)
endif()

if(PLUGIFY_COMPILER_MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX /MP)
elseif(PLUGIFY_COMPILER_CLANG_CL)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
else(PLUGIFY_COMPILER_GCC OR PLUGIFY_COMPILER_CLANG)
    target_compile_options(${PROJECT_NAME} PRIVATE -Wextra -Wshadow -Wconversion -Wpedantic -Werror)
endif()

if(LINUX)
    target_compile_definitions(${PROJECT_NAME} PUBLIC _GLIBCXX_USE_CXX11_ABI=$<IF:$<BOOL:${PLUGIFY_USE_ABI0}>,0,1>)
endif()

if(PLUGIFY_HAS_LIBCPP)
    target_compile_options(${PROJECT_NAME} PUBLIC -stdlib=libc++)
    if(PLUGIFY_HAS_STATIC_LIBCPP)
        target_link_libraries(${PROJECT_NAME} PUBLIC -static-libc++ -static-libgcc)
    endif()
else()
    if(PLUGIFY_HAS_STATIC_LIBSTDCPP)
        target_link_libraries(${PROJECT_NAME} PUBLIC -static-libstdc++ -static-libgcc)
    endif()
endif()

# Check and add each flag
if(PLUGIFY_EXTRA_FLAGS)
    foreach(FLAG ${PLUGIFY_EXTRA_FLAGS})
        string(REPLACE "-" "" FLAG_NAME ${FLAG})
        string(REPLACE "/" "" FLAG_NAME ${FLAG_NAME})
        string(TOUPPER ${FLAG_NAME} FLAG_VAR)
        set(CHECK_VAR "COMPILER_SUPPORTS_${FLAG_VAR}")

        include(CheckCXXCompilerFlag)
        check_cxx_compiler_flag(${FLAG} ${CHECK_VAR})

        if(${${CHECK_VAR}})
            message(STATUS "Compiler supports ${FLAG}")
            target_compile_options(${PROJECT_NAME} PUBLIC ${FLAG})
        else()
            message(WARNING "Compiler does not support ${FLAG}")
        endif()
    endforeach()
endif()

get_directory_property(HAS_PARENT PARENT_DIRECTORY)
if(PLUGIFY_CPU_ARCH_ARM64 OR PLUGIFY_CPU_ARCH_ARM32)
    set(PLUGIFY_USE_ARM TRUE)
    if (HAS_PARENT)
        set(PLUGIFY_USE_ARM TRUE PARENT_SCOPE)
    endif()
else()
    set(PLUGIFY_USE_ARM FALSE)
    if (HAS_PARENT)
        set(PLUGIFY_USE_ARM FALSE PARENT_SCOPE)
    endif()
endif()

set(PLUGIFY_COMPILE_DEFINITIONS
        PLUGIFY_VERSION="${PLUGIFY_VERSION}"
        PLUGIFY_PLATFORM="${PLUGIFY_PLATFORM}"
        PLUGIFY_PLATFORM_WINDOWS=$<BOOL:${WIN32}>
        PLUGIFY_PLATFORM_APPLE=$<BOOL:${APPLE}>
        PLUGIFY_PLATFORM_LINUX=$<BOOL:${LINUX}>
        PLUGIFY_PLATFORM_ANDROID=$<BOOL:${ANDROID}>
        PLUGIFY_PLATFORM_ORBIS=$<BOOL:${ORBIS}>
        PLUGIFY_PLATFORM_PROSPERO=$<BOOL:${PROSPERO}>
        PLUGIFY_PLATFORM_SWITCH=$<BOOL:${SWITCH}>
        # PLUGIFY_PLATFORM_BSD=$<BOOL:${BSD}>
        PLUGIFY_PLATFORM_UNIX=$<BOOL:${UNIX}>
        PLUGIFY_ARCH_BITS=$<IF:$<EQUAL:${CMAKE_SIZEOF_VOID_P},8>,64,32>
        PLUGIFY_ARCH_ARM=$<BOOL:${PLUGIFY_USE_ARM}>
        PLUGIFY_IS_BIG_ENDIAN=$<STREQUAL:${CMAKE_CXX_BYTE_ORDER},BIG_ENDIAN>
        PLUGIFY_IS_DEBUG=$<STREQUAL:${CMAKE_BUILD_TYPE},Debug>
        PLUGIFY_IS_RELEASE=$<STREQUAL:${CMAKE_BUILD_TYPE},Release>

        PLUGIFY_CXX_STANDARD=${CMAKE_CXX_STANDARD}
        PLUGIFY_COMPILED_COMPILER="${CMAKE_CXX_COMPILER_ID}"
        PLUGIFY_COMPILED_SYSTEM="${CMAKE_SYSTEM}"
        PLUGIFY_COMPILED_GENERATOR="${CMAKE_GENERATOR}"
        PLUGIFY_LIBRARY_SUFFIX="${CMAKE_SHARED_LIBRARY_SUFFIX}"
        PLUGIFY_LIBRARY_PREFIX="${CMAKE_SHARED_LIBRARY_PREFIX}"
        PLUGIFY_EXECUTABLE_SUFFIX="${CMAKE_EXECUTABLE_SUFFIX}"
        PLUGIFY_EXECUTABLE_PREFIX="${CMAKE_EXECUTABLE_PREFIX}"

        PLUGIFY_COMPILER_MSVC=$<BOOL:${PLUGIFY_COMPILER_MSVC}>
        PLUGIFY_COMPILER_CLANG=$<BOOL:${PLUGIFY_COMPILER_CLANG}>
        PLUGIFY_COMPILER_GCC=$<BOOL:${PLUGIFY_COMPILER_GCC}>
        PLUGIFY_COMPILER_INTEL=$<BOOL:${PLUGIFY_COMPILER_INTEL}>
)

target_compile_definitions(${PROJECT_NAME} PUBLIC
        PLUGIFY_FORMAT_SUPPORT=$<BOOL:${COMPILER_SUPPORTS_FORMAT}>
        PLUGIFY_STACKTRACE_SUPPORT=$<BOOL:${COMPILER_SUPPORTS_STACKTRACE}>
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
        ${PLUGIFY_COMPILE_DEFINITIONS}
        PLUGIFY_INTERFACE=$<BOOL:${PLUGIFY_INTERFACE}>
        PLUGIFY_DOWNLOADER=$<BOOL:${PLUGIFY_DOWNLOADER}>
        PLUGIFY_LOGGING=$<BOOL:${PLUGIFY_LOGGING}>
        PLUGIFY_DEBUG=$<BOOL:${PLUGIFY_DEBUG}>
        PLUGIFY_SEPARATE_SOURCE_FILES=0
)

if(PLUGIFY_BUILD_SHARED_LIB AND NOT WIN32)
    if(APPLE)
        #target_link_options(${PROJECT_NAME} PRIVATE "-Wl,-exported_symbols_list,${CMAKE_CURRENT_SOURCE_DIR}/sym/exported_symbols.lds")
    elseif(UNIX)
        target_link_options(${PROJECT_NAME} PRIVATE "-Wl,--version-script,${CMAKE_CURRENT_SOURCE_DIR}/sym/version_script.lds")
    endif()
endif()

include(PlugifySanitizer)
include(PlugifyTools)

# ------------------------------------------------------------------------------
# Test
if(PLUGIFY_BUILD_TESTS)
    configure_file(
            ${CMAKE_SOURCE_DIR}/plugify.pconfig
            ${CMAKE_BINARY_DIR}/plugify.pconfig
            COPYONLY
    )
    configure_file(
            ${CMAKE_SOURCE_DIR}/crashpad.jsonc
            ${CMAKE_BINARY_DIR}/crashpad.jsonc
            COPYONLY
    )
    add_subdirectory(test/plug)
    add_subdirectory(test/containers)
endif()

# ------------------------------------------------------------------------------
# Documentation
if(PLUGIFY_BUILD_DOCS)
    find_package(Doxygen REQUIRED)

    if(DOXYGEN_FOUND)
        add_subdirectory(docs)
    endif()
endif()
