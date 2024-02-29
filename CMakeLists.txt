# ------------------------------------------------------------------------------
# Plugify
cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

if(POLICY CMP0092)
    cmake_policy(SET CMP0092 NEW) # Don't add -W3 warning level by default.
endif()


# ------------------------------------------------------------------------------
# Project configuration
project(plugify VERSION 1.0.0.0  DESCRIPTION "Plugify Core" HOMEPAGE_URL "https://github.com/untrustedmodders/plugify" LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

message(VERBOSE "*")
message(VERBOSE "* ${PROJECT_NAME} v${PROJECT_VERSION} (${CMAKE_BUILD_TYPE})")
message(VERBOSE "* Copyright (c) 2023-2024 untrustedmodders <untrustedmodders@gmail.com>")
message(VERBOSE "*")


# ------------------------------------------------------------------------------
# Includes
include(cmake/git.cmake)

include(CheckCXXSourceCompiles)
include(CMakePushCheckState)

include(CheckCCompilerFlag)
check_c_compiler_flag("-fPIC" COMPILER_SUPPORTS_FPIC)


# ------------------------------------------------------------------------------
# Compiler stuff
option(PLUGIFY_USE_LIBCPP "Use libc++ by adding -stdlib=libc++ flag if available." OFF)
option(PLUGIFY_USE_SANITIZER "Enable sanitizers by adding -fsanitize=address -fno-omit-frame-pointer -fsanitize=undefined flags if available." OFF)
option(PLUGIFY_USE_CLANG_TIDY "Enable static analysis with clang-tidy" OFF)

if(PLUGIFY_USE_LIBCPP)
    if(NOT WIN32)
        cmake_push_check_state()

        set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} -stdlib=libc++")

        check_cxx_source_compiles("
            #include<type_traits>
            int main() { return std::is_same_v<int, char>; }
        " PLUGIFY_HAS_LIBCPP)

        cmake_pop_check_state()
    endif()

    if(NOT PLUGIFY_HAS_LIBCPP)
        message(VERBOSE "The option PLUGIFY_HAS_LIBCPP is set but libc++ is not available.")
    endif()
endif()

if(PLUGIFY_USE_SANITIZER)
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        set(PLUGIFY_HAS_SANITIZER TRUE CACHE BOOL "" FORCE)
        mark_as_advanced(PLUGIFY_HAS_SANITIZER)
    endif()

    if(NOT PLUGIFY_HAS_SANITIZER)
        message(VERBOSE "The option PLUGIFY_USE_SANITIZER is set but sanitizer support is not available.")
    endif()
endif()

if(PLUGIFY_USE_CLANG_TIDY)
    find_program(PLUGIFY_CLANG_TIDY_EXECUTABLE "clang-tidy")

    if(NOT PLUGIFY_CLANG_TIDY_EXECUTABLE)
        message(VERBOSE "The option PLUGIFY_USE_CLANG_TIDY is set but clang-tidy executable is not available.")
    endif()
endif()

# ------------------------------------------------------------------------------
# Compilation options
option(PLUGIFY_BUILD_TESTS "Enable building tests." OFF)
option(PLUGIFY_BUILD_FUNCTION "Build function object library." OFF)
option(PLUGIFY_BUILD_DOCS "Enable building with documentation." OFF)

option(PLUGIFY_BUILD_OBJECT_LIB "Build plugify as object library." OFF)
option(PLUGIFY_BUILD_SHARED_LIB "Build plugify as shared library." ON)
option(PLUGIFY_BUILD_SHARED_ASMJIT "Build asmjit as shared library." OFF)
option(PLUGIFY_BUILD_SHARED_CURL "Build curl as shared library." OFF)

option(PLUGIFY_USE_EXTERNAL_ASMJIT "Use external asmjit library." OFF)
option(PLUGIFY_USE_EXTERNAL_GLAZE "Use external glaze library." OFF)
option(PLUGIFY_USE_EXTERNAL_CURL "Use external curl library." ON)
option(PLUGIFY_USE_EXTERNAL_FMT "Use external fmt library." OFF)

option(PLUGIFY_LOGGING "Enable logging system." ON)
option(PLUGIFY_DEBUG "Enable debuging mode (asserts)." ON)


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
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_precompile_headers(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/pch.h)

# ------------------------------------------------------------------------------
# Glaze
if(PLUGIFY_USE_EXTERNAL_GLAZE)
    find_package(glaze REQUIRED)
else()
    include(cmake/glaze.cmake)
endif()
target_link_libraries(${PROJECT_NAME} PRIVATE glaze::glaze)

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
        include(cmake/curl.git)
        target_link_libraries(${PROJECT_NAME} PRIVATE ${CURL_LIBRARIES})
        target_include_directories(${PROJECT_NAME} PRIVATE ${CURL_INCLUDE_DIRS})
    endif()
endif()

# ------------------------------------------------------------------------------
# SHA256 & Miniz
include(cmake/sha256.cmake)
include(cmake/miniz.cmake)
target_link_libraries(${PROJECT_NAME} PRIVATE sha256 miniz)
if(COMPILER_SUPPORTS_FPIC)
    target_compile_options(sha256 PRIVATE -fPIC)
    target_compile_options(miniz PRIVATE -fPIC)
endif()

# ------------------------------------------------------------------------------
# Format
cmake_push_check_state()

check_cxx_source_compiles("
    #include<format>
    #include<string>
    int main() { std::string a = std::format(\"{}.{}.{}\", \"Hello\", \"World\", \"C++\"); return 0; }
    " COMPILER_SUPPORTS_FORMAT)

cmake_pop_check_state()

if(NOT COMPILER_SUPPORTS_FORMAT)
    if(PLUGIFY_USE_EXTERNAL_FMT)
        find_package(fmt REQUIRED)
    else()
        include(cmake/fmt.cmake)
    endif()
    target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt-header-only)
endif()

# ------------------------------------------------------------------------------
#
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

if(PLUGIFY_HAS_LIBCPP)
    target_compile_options(${PROJECT_NAME} PRIVATE -stdlib=libc++)
endif()

if(PLUGIFY_HAS_SANITIZER)
    target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>: -fsanitize=address -fno-omit-frame-pointer -fsanitize=undefined>)
    target_link_libraries(${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>: -fsanitize=address -fno-omit-frame-pointer -fsanitize=undefined>)
endif()

if(PLUGIFY_CLANG_TIDY_EXECUTABLE)
    set(CMAKE_CXX_CLANG_TIDY "${PLUGIFY_CLANG_TIDY_EXECUTABLE}; --config-file=.clang-tidy; --header-filter=src/.*; --extra-arg=/EHsc")
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
        PLUGIFY_FORMAT_SUPPORT=$<BOOL:${COMPILER_SUPPORTS_FORMAT}>
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

if(PLUGIFY_BUILD_SHARED_LIB AND NOT WIN32)
    if(${CMAKE_VERSION} VERSION_GREATER 3.13 OR ${CMAKE_VERSION} VERSION_EQUAL 3.13)
        if(APPLE)
            target_link_options(${PROJECT_NAME} PRIVATE "-Wl,-exported_symbols_list,${CMAKE_CURRENT_SOURCE_DIR}/src/exported_symbols.lds")
        elseif(UNIX)
            target_link_options(${PROJECT_NAME} PRIVATE "-Wl,--version-script,${CMAKE_CURRENT_SOURCE_DIR}/src/version_script.lds")
        endif()
    else()
        if(APPLE)
            target_link_libraries(${PROJECT_NAME} PRIVATE "-Wl,-exported_symbols_list,${CMAKE_CURRENT_SOURCE_DIR}/src/exported_symbols.lds")
        elseif(UNIX)
            target_link_libraries(${PROJECT_NAME} PRIVATE "-Wl,--version-script,${CMAKE_CURRENT_SOURCE_DIR}/src/version_script.lds")
        endif()
    endif()
endif()

# ------------------------------------------------------------------------------
# Tools
if(PLUGIFY_BUILD_FUNCTION)
    add_library(${PROJECT_NAME}-function OBJECT "${CMAKE_CURRENT_SOURCE_DIR}/include/plugify/function.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/include/plugify/function.h")
    add_library(${PROJECT_NAME}::${PROJECT_NAME}-function ALIAS ${PROJECT_NAME}-function)
    target_include_directories(${PROJECT_NAME}-function PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
    if(PLUGIFY_USE_EXTERNAL_ASMJIT)
        find_package(asmjit REQUIRED)
    else()
        include(cmake/asmjit.cmake)
        target_include_directories(${PROJECT_NAME}-function PRIVATE ${ASMJIT_SRC})
    endif()
    target_link_libraries(${PROJECT_NAME}-function PRIVATE asmjit::asmjit)
    target_compile_definitions(${PROJECT_NAME}-function PRIVATE
            PLUGIFY_PLATFORM_WINDOWS=$<BOOL:${WIN32}>
            PLUGIFY_PLATFORM_APPLE=$<BOOL:${APPLE}>
            PLUGIFY_PLATFORM_LINUX=$<BOOL:${UNIX}>
            PLUGIFY_ARCH_X86=${PLUGIFY_ARCH_X86})
endif()

# ------------------------------------------------------------------------------
# Test
if(PLUGIFY_BUILD_TESTS)
    add_subdirectory(test/plug)
endif()

# ------------------------------------------------------------------------------
# Documentation
if(PLUGIFY_BUILD_DOCS)
    find_package(Doxygen REQUIRED)

    if(DOXYGEN_FOUND)
        add_subdirectory(docs)
    endif()
endif()
