cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

if(POLICY CMP0092)
    cmake_policy(SET CMP0092 NEW) # Don't add -W3 warning level by default.
endif()


project(wizard)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Speed up building on MinGW
if(MINGW)
    add_link_options(-fuse-ld=lld)
endif()

include(cmake/git.cmake)

#
# Options
#
option(WIZARD_BUILD_DLL "Build dll & lib instead of tests" OFF)
option(WIZARD_BUILD_SHARED_LIB "Build wizard as shared library" OFF)
option(WIZARD_BUILD_SHARED_ASMJIT "Build asmjit as shared library" OFF)
option(WIZARD_BUILD_SHARED_PHYSFS "Build physfs as shared library" OFF)

option(WIZARD_USE_EXTERNAL_ASMJIT "Use external asmjit library" OFF)
option(WIZARD_USE_EXTERNAL_RAPIDJSON "Use external rapidjson library" OFF)
option(WIZARD_USE_EXTERNAL_PHYSFS "Use external physfs library" OFF)

option(WIZARD_LOGGING "Enable logs" ON)

#
# Asmjit
#
if(NOT WIZARD_USE_EXTERNAL_ASMJIT)
    set(ASMJIT_STATIC $<BOOL:${WIZARD_BUILD_SHARED_ASMJIT}> CACHE BOOL "")

    add_subdirectory(external/asmjit)
else()
    find_package(asmjit)
endif()

#
# Rapidjson
#
if(NOT WIZARD_USE_EXTERNAL_RAPIDJSON)
    set(RAPIDJSON_BUILD_DOC OFF CACHE INTERNAL "")
    set(RAPIDJSON_BUILD_EXAMPLES OFF CACHE INTERNAL "")
    set(RAPIDJSON_BUILD_TESTS OFF CACHE INTERNAL "")
    set(RAPIDJSON_BUILD_THIRDPARTY_GTEST OFF CACHE INTERNAL "")
    set(RAPIDJSON_BUILD_CXX11 OFF CACHE INTERNAL "")
    set(RAPIDJSON_BUILD_CXX17 ON CACHE INTERNAL "")

    add_subdirectory(external/rapidjson)
endif()

#
# Physfs
#
if(NOT WIZARD_USE_EXTERNAL_PHYSFS)
    set(PHYSFS_BUILD_SHARED ${WIZARD_BUILD_SHARED_PHYSFS} CACHE BOOL "")
    set(PHYSFS_BUILD_TEST OFF CACHE BOOL "")
    set(PHYSFS_BUILD_DOCS OFF CACHE BOOL "")

    add_subdirectory(external/physfs)
endif()

#
# Wizard
#
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

if(WIZARD_USE_EXTERNAL_RAPIDJSON)
	find_package(rapidjson REQUIRED)
    target_include_directories(${PROJECT_NAME} PRIVATE ${RAPIDJSON_INCLUDE_DIRS})
else()
    target_include_directories(${PROJECT_NAME} PRIVATE external/rapidjson/include)
endif()

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
    endif()
endif()
target_link_libraries(${PROJECT_NAME} PRIVATE git-info ${CMAKE_DL_LIBS})

if(WIZARD_BUILD_DLL)
    include(GenerateExportHeader)
    generate_export_header(${PROJECT_NAME} EXPORT_FILE_NAME ${CMAKE_BINARY_DIR}/exports/${PROJECT_NAME}_export.h)
    target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_BINARY_DIR}/exports)
endif()

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
    if(NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_compile_options(${PROJECT_NAME} PRIVATE /MP)
    endif()
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wextra -Wshadow -Wconversion -Wpedantic -Werror)
endif()

string(TOLOWER ${CMAKE_SYSTEM_NAME} WIZARD_PLATFORM)

target_compile_definitions(${PROJECT_NAME} PUBLIC
        WIZARD_PLATFORM="${WIZARD_PLATFORM}"
        WIZARD_LOGGING=$<BOOL:${WIZARD_LOGGING}>
        WIZARD_COMPILED_COMPILER="${CMAKE_CXX_COMPILER}"
        WIZARD_COMPILED_SYSTEM="${CMAKE_SYSTEM}"
        WIZARD_COMPILED_GENERATOR="${CMAKE_GENERATOR}"
        WIZARD_MODULE_SUFFIX="${CMAKE_SHARED_LIBRARY_SUFFIX}"
        WIZARD_MODULE_PREFIX="${CMAKE_SHARED_LIBRARY_PREFIX}"
        WIZARD_BUILD_SHARED_LIB=$<BOOL:${WIZARD_BUILD_SHARED_LIB}>)

# If building shared library, set dllimport/dllexport proper.
if (WIZARD_BUILD_DLL)
    target_compile_options(${PROJECT_NAME} PRIVATE WIZARD_BUILD_MAIN_LIB)
endif()