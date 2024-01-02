cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

if(POLICY CMP0092)
    cmake_policy(SET CMP0092 NEW) # Don't add -W3 warning level by default.
endif()


project(wizard VERSION 1.0.0.0  DESCRIPTION "Wizard Core" HOMEPAGE_URL "https://github.com/untrustedmodders/wizard")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

include(cmake/git.cmake)

# ------------------------------------------------------------------------------
# Build options for tests
option(WIZARD_BUILD_TESTS "Build all tests" ON)

# ------------------------------------------------------------------------------
# Compilation options
option(WIZARD_BUILD_DLL "Build dll & lib instead of tests" ON)
option(WIZARD_BUILD_SHARED_LIB "Build wizard as shared library" ON)
option(WIZARD_BUILD_SHARED_ASMJIT "Build asmjit as shared library" OFF)
option(WIZARD_BUILD_SHARED_PHYSFS "Build physfs as shared library" OFF)

option(WIZARD_USE_EXTERNAL_ASMJIT "Use external asmjit library" OFF)
option(WIZARD_USE_EXTERNAL_GLAZE "Use external glaze library" OFF)
option(WIZARD_USE_EXTERNAL_PHYSFS "Use external physfs library" OFF)
option(WIZARD_USE_EXTERNAL_FMT "Use external fmt library" OFF)

option(WIZARD_LOGGING "Enable logs" ON)

# ------------------------------------------------------------------------------
# Asmjit
if(NOT WIZARD_USE_EXTERNAL_ASMJIT)
    set(ASMJIT_STATIC $<BOOL:${WIZARD_BUILD_SHARED_ASMJIT}> CACHE BOOL "")

    add_subdirectory(external/asmjit)
endif()

# ------------------------------------------------------------------------------
# Glaze
if(NOT WIZARD_USE_EXTERNAL_GLAZE)
    add_subdirectory(external/glaze)
endif()

# ------------------------------------------------------------------------------
# Physfs
if(NOT WIZARD_USE_EXTERNAL_PHYSFS)
    if(WIZARD_BUILD_SHARED_PHYSFS)
        set(PHYSFS_BUILD_SHARED ON CACHE BOOL "")
        set(PHYSFS_BUILD_STATIC OFF CACHE BOOL "")
    else()
        set(PHYSFS_BUILD_SHARED OFF CACHE BOOL "")
        set(PHYSFS_BUILD_STATIC ON CACHE BOOL "")
    endif()
    set(PHYSFS_BUILD_TEST OFF CACHE BOOL "")
    set(PHYSFS_BUILD_DOCS OFF CACHE BOOL "")

    add_subdirectory(external/physfs)
endif()

# ------------------------------------------------------------------------------
# Format
#include(CheckIncludeFileCXX)
#check_include_file_cxx("format" CXX_FORMAT_SUPPORT)
try_compile(CXX_FORMAT_SUPPORT ${CMAKE_BINARY_DIR}/format_check SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/test/format_check.cpp)
if(NOT CXX_FORMAT_SUPPORT AND NOT WIZARD_USE_EXTERNAL_FMT)
    add_subdirectory(external/fmt)
endif()

# ------------------------------------------------------------------------------
# Wizard
file(GLOB_RECURSE WIZARD_CORE_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "src/*.cpp")

if(WIZARD_BUILD_DLL)
    if(WIZARD_BUILD_SHARED_LIB)
        add_library(${PROJECT_NAME} SHARED ${WIZARD_CORE_SOURCES})
        set_target_properties(${PROJECT_NAME} PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
    else()
        add_library(${PROJECT_NAME} STATIC ${WIZARD_CORE_SOURCES})
    endif()
else()
    add_executable(${PROJECT_NAME} ${WIZARD_CORE_SOURCES})
endif()

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_precompile_headers(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/pch.h)

# ------------------------------------------------------------------------------
# Glaze
if(WIZARD_USE_EXTERNAL_GLAZE)
	find_package(glaze REQUIRED)
    target_link_libraries(${PROJECT_NAME} PRIVATE glaze::glaze)
else()
    target_link_libraries(${PROJECT_NAME} PRIVATE glaze_glaze)
endif()

# ------------------------------------------------------------------------------
# Asmjit
if(WIZARD_USE_EXTERNAL_ASMJIT)
    find_package(asmjit REQUIRED)
    target_link_libraries(${PROJECT_NAME} PUBLIC asmjit::asmjit)
else()
    target_link_libraries(${PROJECT_NAME} PUBLIC asmjit)
    target_include_directories(${PROJECT_NAME} PUBLIC ${ASMJIT_SRC})

    if(MSVC)
        target_compile_options(asmjit PUBLIC /wd5054)
    else()
        if(MINGW)
            target_compile_options(asmjit PUBLIC -Wno-deprecated-enum-enum-conversion)
        else()
            target_compile_options(asmjit PUBLIC -Wno-deprecated-anon-enum-enum-conversion)
        endif()
        target_compile_options(asmjit PUBLIC -Wno-deprecated-declarations -Wno-error -Wno-pedantic -Wno-shadow)
    endif()
endif()

# ------------------------------------------------------------------------------
# Physfs
if(WIZARD_USE_EXTERNAL_PHYSFS)
	find_package(physfs REQUIRED)
	if(WIZARD_BUILD_SHARED_PHYSFS)
		target_link_libraries(${PROJECT_NAME} PRIVATE physfs::physfs)
	else()
		target_link_libraries(${PROJECT_NAME} PRIVATE physfs::physfs-static)
	endif()
else()
    if(WIZARD_BUILD_SHARED_PHYSFS)
        target_link_libraries(${PROJECT_NAME} PRIVATE physfs)
    else()
        target_link_libraries(${PROJECT_NAME} PRIVATE physfs-static)

        include(CheckCCompilerFlag)
        check_c_compiler_flag("-fPIC" COMPILER_SUPPORTS_FPIC)
        if(COMPILER_SUPPORTS_FPIC)
            target_compile_options(physfs-static PRIVATE -fPIC)
        endif()
    endif()
endif()

# ------------------------------------------------------------------------------
# Format
if(NOT CXX_FORMAT_SUPPORT)
    if(WIZARD_USE_EXTERNAL_FMT)
        find_package(fmt REQUIRED)
        target_link_libraries(${PROJECT_NAME} PUBLIC fmt::fmt)
    else()
        target_link_libraries(${PROJECT_NAME} PUBLIC fmt)
    endif()
endif()

# ------------------------------------------------------------------------------
# System
target_link_libraries(${PROJECT_NAME} PRIVATE git-info ${CMAKE_DL_LIBS})

if(WIZARD_BUILD_DLL)
    include(GenerateExportHeader)
    generate_export_header(${PROJECT_NAME}
            BASE_NAME WIZARD
            EXPORT_MACRO_NAME WIZARD_API
            NO_EXPORT_MACRO_NAME WIZARD_PRIVATE
            EXPORT_FILE_NAME ${CMAKE_BINARY_DIR}/exports/${PROJECT_NAME}_export.h
            STATIC_DEFINE WIZARD_STATIC
            )
    target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_BINARY_DIR}/exports)
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
    set(WIZARD_PLATFORM "windows")
    target_compile_definitions(${PROJECT_NAME} PUBLIC WIZARD_PLATFORM_WINDOWS)
elseif(APPLE)
    set(WIZARD_PLATFORM "apple")
    target_compile_definitions(${PROJECT_NAME} PUBLIC WIZARD_PLATFORM_APPLE)
elseif(UNIX)
    set(WIZARD_PLATFORM "linux")
    target_compile_definitions(${PROJECT_NAME} PUBLIC WIZARD_PLATFORM_LINUX)
endif()

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(WIZARD_ARCH_X86 64)
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(WIZARD_ARCH_X86 32)
endif()

target_compile_definitions(${PROJECT_NAME} PUBLIC
        WIZARD_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
        WIZARD_VERSION_MINOR=${PROJECT_VERSION_MINOR}
        WIZARD_VERSION_PATCH=${PROJECT_VERSION_PATCH}
        WIZARD_VERSION_TWEAK=${PROJECT_VERSION_TWEAK}
        WIZARD_PLATFORM="${WIZARD_PLATFORM}"
        WIZARD_ARCH_X86=${WIZARD_ARCH_X86}
        WIZARD_LOGGING=$<BOOL:${WIZARD_LOGGING}>
        WIZARD_FORMAT_SUPPORT=$<BOOL:${CXX_FORMAT_SUPPORT}>
        )

target_compile_definitions(${PROJECT_NAME} PRIVATE
        WIZARD_COMPILED_COMPILER="${CMAKE_CXX_COMPILER}"
        WIZARD_COMPILED_SYSTEM="${CMAKE_SYSTEM}"
        WIZARD_COMPILED_GENERATOR="${CMAKE_GENERATOR}"
        WIZARD_MODULE_SUFFIX="${CMAKE_SHARED_LIBRARY_SUFFIX}"
        WIZARD_MODULE_PREFIX="${CMAKE_SHARED_LIBRARY_PREFIX}")

if(WIZARD_BUILD_TESTS)
    add_subdirectory(test/hat)
endif()
