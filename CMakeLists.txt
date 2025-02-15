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

if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

message(VERBOSE "*")
message(VERBOSE "* ${PROJECT_NAME} v${PROJECT_VERSION} (${CMAKE_BUILD_TYPE})")
message(VERBOSE "* Copyright (c) 2023-2024 untrustedmodders <untrustedmodders@gmail.com>")
message(VERBOSE "*")


# ------------------------------------------------------------------------------
# Includes
include(cmake/git.cmake)

include(CheckCXXSourceCompiles)
include(CheckCXXCompilerFlag)
include(CMakePushCheckState)


# ------------------------------------------------------------------------------
# Compiler stuff
option(PLUGIFY_USE_LIBCPP "Use libc++ by adding -stdlib=libc++ flag if available." OFF)
option(PLUGIFY_USE_STATIC_STDLIB "Enable static std library linkage to avoid ABI issues by adding -static-* flags if available." OFF)
option(PLUGIFY_USE_SANITIZER "Enable sanitizers by adding -fsanitize=address -fno-omit-frame-pointer -fsanitize=undefined flags if available." OFF)
option(PLUGIFY_USE_CLANG_TIDY "Enable static analysis with clang-tidy" OFF)
option(PLUGIFY_USE_ABI0 "Enable use of the older C++ ABI, which was the default in GCC versions before GCC 5" OFF)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang|GNU")
    if(PLUGIFY_USE_LIBCPP)
        check_cxx_compiler_flag(-stdlib=libc++ PLUGIFY_HAS_LIBCPP)
        if(NOT PLUGIFY_HAS_LIBCPP)
            message(VERBOSE "The option PLUGIFY_USE_LIBCPP is set but libc++ is not available.")
        endif()

        if(PLUGIFY_USE_STATIC_STDLIB)
            check_cxx_compiler_flag(-static-libc++ PLUGIFY_HAS_STATIC_LIBCPP)
            if(NOT PLUGIFY_HAS_LIBCPP)
                message(VERBOSE "The option PLUGIFY_USE_STATIC_STDLIB is set but static-libc++ is not available.")
            endif()
        endif()
    else()
        if(PLUGIFY_USE_STATIC_STDLIB)
            check_cxx_compiler_flag(-static-libstdc++ PLUGIFY_HAS_STATIC_LIBSTDCPP)
            if(NOT PLUGIFY_HAS_LIBCPP)
                message(VERBOSE "The option PLUGIFY_USE_STATIC_STDLIB is set but static-libstdc++ is not available.")
            endif()
        endif()
    endif()

    if(PLUGIFY_USE_STATIC_STDLIB)
        check_cxx_compiler_flag(-static-libgcc PLUGIFY_HAS_STATIC_LIBGCC)
        if(NOT PLUGIFY_HAS_LIBCPP)
            message(VERBOSE "The option PLUGIFY_USE_STATIC_STDLIB is set but static-libgcc is not available.")
        endif()
    endif()
endif()

if(PLUGIFY_USE_SANITIZER)
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang|GNU")
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

message(VERBOSE "Processor: ${CMAKE_SYSTEM_PROCESSOR}")

if(CMAKE_SYSTEM_PROCESSOR MATCHES "amd64.*|x86_64.*|AMD64.*")
    set(PLUGIFY_X86_64 TRUE)
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "i686.*|i386.*|x86.*|amd64.*|AMD64.*")
    set(PLUGIFY_X86 TRUE)
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^(aarch64.*|AARCH64.*)")
    set(PLUGIFY_AARCH64 TRUE)
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^(arm.*|ARM.*)")
    set(PLUGIFY_ARM TRUE)
else()
    message(STATUS "Processor is not supported!")
endif()

# detect host architecture
if(NOT DEFINED PLUGIFY_USE_ARM)
    if(PLUGIFY_AARCH64 OR PLUGIFY_ARM)
        message(STATUS "Detected ARM host. Setting PLUGIFY_USE_ARM to true.")
        set(PLUGIFY_USE_ARM TRUE)
    else()
        message(STATUS "Detected non-ARM host. Setting PLUGIFY_USE_ARM to false.")
        set(PLUGIFY_USE_ARM FALSE)
    endif()
endif()

if(NOT PLUGIFY_USE_ARM)
    # Define compiler-specific flags
    set(GCC_CLANG_FLAGS -msse -msse2 -msse3 -mssse3 -msse4.1 -msse4.2 -msha)
    set(MSVC_FLAGS /arch:AVX /arch:AVX2) # /arch:SSE /arch:SSE2 only for x86
    set(INTEL_FLAGS -xSSE2 -xSSE3 -xSSSE3 -xSSE4.1 -xSSE4.2 -xSHA)

    # Determine the compiler and set the appropriate flags
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang|GNU")
        set(PLUGIFY_SIMD_FLAGS ${GCC_CLANG_FLAGS})
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        set(PLUGIFY_SIMD_FLAGS ${MSVC_FLAGS})
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
        set(PLUGIFY_SIMD_FLAGS ${INTEL_FLAGS})
    else()
        message(STATUS "Unsupported compiler: ${CMAKE_CXX_COMPILER_ID}")
    endif()
endif()

# ------------------------------------------------------------------------------
# Compilation options
option(PLUGIFY_BUILD_TESTS "Enable building tests." ON)
option(PLUGIFY_BUILD_JIT "Build jit object library." OFF)
option(PLUGIFY_BUILD_ASSEMBLY "Build assembly object library." OFF)
option(PLUGIFY_BUILD_DOCS "Enable building with documentation." OFF)

option(PLUGIFY_BUILD_OBJECT_LIB "Build plugify as object library." OFF)
option(PLUGIFY_BUILD_SHARED_LIB "Build plugify as shared library." ON)
option(PLUGIFY_BUILD_SHARED_ASMJIT "Build asmjit as shared library." OFF)
option(PLUGIFY_BUILD_SHARED_CURL "Build curl as shared library." OFF)

option(PLUGIFY_USE_EXTERNAL_ASMJIT "Use external asmjit library." OFF)
option(PLUGIFY_USE_EXTERNAL_GLAZE "Use external glaze library." OFF)
option(PLUGIFY_USE_EXTERNAL_CURL "Use external curl library." ON)
option(PLUGIFY_USE_EXTERNAL_FMT "Use external fmt library." OFF)

option(PLUGIFY_INTERFACE "Build as lightweight interface for language modules." OFF)
option(PLUGIFY_DOWNLOADER "Enable downloader for package manager." ON)
option(PLUGIFY_LOGGING "Enable logging system." ON)
option(PLUGIFY_DEBUG "Enable debuging mode (asserts)." ON)

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

if(NOT PLUGIFY_INTERFACE)
    # ------------------------------------------------------------------------------
    # Glaze
    if(PLUGIFY_USE_EXTERNAL_GLAZE)
        find_package(glaze REQUIRED)
    else()
        include(cmake/glaze.cmake)
    endif()
    target_link_libraries(${PROJECT_NAME} PRIVATE glaze::glaze)

    # ------------------------------------------------------------------------------
    # Http/Curl
    if(PLUGIFY_DOWNLOADER)
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
    endif()

    # ------------------------------------------------------------------------------
    # Miniz
    include(cmake/miniz.cmake)
    target_link_libraries(${PROJECT_NAME} PRIVATE miniz)
    set_property(TARGET miniz PROPERTY POSITION_INDEPENDENT_CODE ON)
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
target_link_libraries(${PROJECT_NAME} PUBLIC plugify-git)

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
else(CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang|GNU")
    target_compile_options(${PROJECT_NAME} PRIVATE -Wextra -Wshadow -Wconversion -Wpedantic -Werror)
endif()

if(PLUGIFY_HAS_SANITIZER)
    target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>: -fsanitize=address -fno-omit-frame-pointer -fsanitize=undefined>)
    target_link_libraries(${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>: -fsanitize=address -fno-omit-frame-pointer -fsanitize=undefined>)
endif()

if(PLUGIFY_CLANG_TIDY_EXECUTABLE)
    set(CMAKE_CXX_CLANG_TIDY "${PLUGIFY_CLANG_TIDY_EXECUTABLE}; --config-file=.clang-tidy; --header-filter=src/.*; --extra-arg=/EHsc")
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
if(PLUGIFY_SIMD_FLAGS)
    foreach(FLAG ${PLUGIFY_SIMD_FLAGS})
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

file(READ "${CMAKE_CURRENT_SOURCE_DIR}/version.txt" PLUGIFY_VERSION)
string(STRIP "${PLUGIFY_VERSION}" PLUGIFY_VERSION)

if(WIN32)
    set(PLUGIFY_PLATFORM "windows")
elseif(APPLE)
    set(PLUGIFY_PLATFORM "apple")
elseif(UNIX)
    set(PLUGIFY_PLATFORM "linux")
elseif(BSD)
    set(PLUGIFY_PLATFORM "bsd")
elseif(ORBIS)
    set(PLUGIFY_PLATFORM "orbis")
elseif(PROSPERO)
    set(PLUGIFY_PLATFORM "prospero")
elseif(SWITCH)
    set(PLUGIFY_PLATFORM "switch")
endif()

set(PLUGIFY_COMPILE_DEFINITIONS
        PLUGIFY_VERSION="${PLUGIFY_VERSION}"
        PLUGIFY_PLATFORM="${PLUGIFY_PLATFORM}"
        PLUGIFY_PLATFORM_WINDOWS=$<BOOL:${WIN32}>
        PLUGIFY_PLATFORM_APPLE=$<BOOL:${APPLE}>
        PLUGIFY_PLATFORM_LINUX=$<BOOL:${LINUX}>
        PLUGIFY_PLATFORM_ANDROID=$<BOOL:${ANDROID}>
        PLUGIFY_PLATFORM_BSD=$<BOOL:${BSD}>
        PLUGIFY_PLATFORM_ORBIS=$<BOOL:${ORBIS}>
        PLUGIFY_PLATFORM_PROSPERO=$<BOOL:${PROSPERO}>
        PLUGIFY_PLATFORM_SWITCH=$<BOOL:${SWITCH}>
        PLUGIFY_ARCH_BITS=$<IF:$<EQUAL:${CMAKE_SIZEOF_VOID_P},8>,64,32>
        PLUGIFY_ARCH_ARM=$<BOOL:${PLUGIFY_USE_ARM}>
        PLUGIFY_IS_BIG_ENDIAN=$<STREQUAL:${CMAKE_CXX_BYTE_ORDER},BIG_ENDIAN>
        PLUGIFY_IS_DEBUG=$<STREQUAL:${CMAKE_BUILD_TYPE},Debug>
        PLUGIFY_IS_RELEASE=$<STREQUAL:${CMAKE_BUILD_TYPE},Release>

        PLUGIFY_COMPILED_COMPILER="${CMAKE_CXX_COMPILER_ID}"
        PLUGIFY_COMPILED_SYSTEM="${CMAKE_SYSTEM}"
        PLUGIFY_COMPILED_GENERATOR="${CMAKE_GENERATOR}"
        PLUGIFY_LIBRARY_SUFFIX="${CMAKE_SHARED_LIBRARY_SUFFIX}"
        PLUGIFY_LIBRARY_PREFIX="${CMAKE_SHARED_LIBRARY_PREFIX}"

        PLUGIFY_COMPILER_MSVC=$<CXX_COMPILER_ID:MSVC>
        PLUGIFY_COMPILER_CLANG=$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>
        PLUGIFY_COMPILER_GCC=$<CXX_COMPILER_ID:GNU>
        PLUGIFY_COMPILER_INTEL=$<CXX_COMPILER_ID:Intel>
)

target_compile_definitions(${PROJECT_NAME} PUBLIC
        PLUGIFY_FORMAT_SUPPORT=$<BOOL:${COMPILER_SUPPORTS_FORMAT}>
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
        target_link_options(${PROJECT_NAME} PRIVATE "-Wl,-exported_symbols_list,${CMAKE_CURRENT_SOURCE_DIR}/sym/exported_symbols.lds")
    elseif(UNIX)
        target_link_options(${PROJECT_NAME} PRIVATE "-Wl,--version-script,${CMAKE_CURRENT_SOURCE_DIR}/sym/version_script.lds")
    endif()
endif()

if(PLUGIFY_BUILD_TESTS)
    set(PLUGIFY_BUILD_JIT ON)
    set(PLUGIFY_BUILD_ASSEMBLY ON)
endif()

# ------------------------------------------------------------------------------
# Tools
if(PLUGIFY_BUILD_JIT)
    if(PLUGIFY_USE_ARM)
        set(PLUGIFY_JIT_SOURCES
                "${CMAKE_CURRENT_SOURCE_DIR}/include/plugify/jit/callback_arm.cpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/include/plugify/jit/call_arm.cpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/include/plugify/jit/helpers_arm.cpp"
        )
    else()
        set(PLUGIFY_JIT_SOURCES
                "${CMAKE_CURRENT_SOURCE_DIR}/include/plugify/jit/callback_x86.cpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/include/plugify/jit/call_x86.cpp"
                "${CMAKE_CURRENT_SOURCE_DIR}/include/plugify/jit/helpers_x86.cpp"
        )
    endif()
    add_library(${PROJECT_NAME}-jit OBJECT ${PLUGIFY_JIT_SOURCES})
    add_library(${PROJECT_NAME}::${PROJECT_NAME}-jit ALIAS ${PROJECT_NAME}-jit)
    target_include_directories(${PROJECT_NAME}-jit PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
    if(PLUGIFY_USE_EXTERNAL_ASMJIT)
        find_package(asmjit REQUIRED)
    else()
        include(cmake/asmjit.cmake)
        target_include_directories(${PROJECT_NAME}-jit PRIVATE ${ASMJIT_SRC})
    endif()
    target_link_libraries(${PROJECT_NAME}-jit PRIVATE asmjit::asmjit)
    if(MSVC)
        target_compile_options(asmjit PUBLIC /wd5054)
    elseif(MINGW)
        target_compile_options(asmjit PUBLIC -Wno-deprecated-enum-enum-conversion)
    else()
        target_compile_options(asmjit PUBLIC -Wno-deprecated-anon-enum-enum-conversion -Wno-deprecated-enum-enum-conversion)
    endif()
    target_compile_definitions(${PROJECT_NAME}-jit PRIVATE
            ${PLUGIFY_COMPILE_DEFINITIONS}
            PLUGIFY_SEPARATE_SOURCE_FILES=1
    )
    target_include_directories(${PROJECT_NAME}-jit PUBLIC ${CMAKE_BINARY_DIR}/exports)
    if(LINUX)
        target_compile_definitions(${PROJECT_NAME}-jit PUBLIC _GLIBCXX_USE_CXX11_ABI=$<IF:$<BOOL:${PLUGIFY_USE_ABI0}>,0,1>)
    endif()
    if(PLUGIFY_HAS_LIBCPP)
        target_compile_options(${PROJECT_NAME}-jit PUBLIC -stdlib=libc++)
    endif()
endif()
if(PLUGIFY_BUILD_ASSEMBLY)
    add_library(${PROJECT_NAME}-assembly OBJECT "${CMAKE_CURRENT_SOURCE_DIR}/src/utils/assembly.cpp")
    add_library(${PROJECT_NAME}::${PROJECT_NAME}-assembly ALIAS ${PROJECT_NAME}-assembly)
    target_include_directories(${PROJECT_NAME}-assembly PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
    target_compile_definitions(${PROJECT_NAME}-assembly PRIVATE
            ${PLUGIFY_COMPILE_DEFINITIONS}
            PLUGIFY_SEPARATE_SOURCE_FILES=1
    )
    target_include_directories(${PROJECT_NAME}-assembly PUBLIC ${CMAKE_BINARY_DIR}/exports)
    if(LINUX)
        target_compile_definitions(${PROJECT_NAME}-assembly PUBLIC _GLIBCXX_USE_CXX11_ABI=$<IF:$<BOOL:${PLUGIFY_USE_ABI0}>,0,1>)
    endif()
    if(PLUGIFY_HAS_LIBCPP)
        target_compile_options(${PROJECT_NAME}-assembly PUBLIC -stdlib=libc++)
    endif()
endif()

# ------------------------------------------------------------------------------
# Test
if(PLUGIFY_BUILD_TESTS)
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
