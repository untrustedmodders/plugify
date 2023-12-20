cmake_minimum_required(VERSION 3.9 FATAL_ERROR)
project(wizard)

# Force all compilers to use the correct C and C++ standard versions.
# Needed to allow for CheckCXXSourceCompiles to work correctly.
set(CMAKE_REQUIRED_FLAGS "-std=c++20 -std=c11")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Speed up building on MinGW
if(WIN32)
    add_link_options(-fuse-ld=lld)
endif()

#
# Options
#
option(WIZARD_BUILD_DLL "Build dll & lib instead of tests" ON)
option(WIZARD_BUILD_SHARED_LIB "Build wizard as shared libary" ON)

option(WIZARD_BUILD_SHARED_ASMJIT "Build asmjit as shared libary" OFF)
option(WIZARD_USE_EXTERNAL_ASMJIT "Use external asmjit libary" OFF)
option(WIZARD_USE_EXTERNAL_NLOHMANN_JSON "Use external nlohmann/json libary" OFF)

#
# ASMJIT
#
if(NOT WIZARD_USE_EXTERNAL_ASMJIT)
    set(ASMJIT_STATIC ${WIZARD_BUILD_SHARED_ASMJIT} CACHE BOOL "")

    add_subdirectory(libraries/asmjit)

    if(MSVC)
        if(ASMJIT_STATIC)
            set_target_properties(asmjit PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
        else()
            set_target_properties(asmjit PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
        endif()
    endif()
endif()

#
# Nlohmann/json
#
if(NOT WIZARD_USE_EXTERNAL_NLOHMANN_JSON)
    set(JSON_BuildTests OFF CACHE INTERNAL "")

    add_subdirectory(libraries/nlohmann_json)

    if(MSVC)
        set_target_properties(nlohmann_json PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    endif()
endif()


#
# Wizard
#
if(WIZARD_BUILD_DLL)
    if(WIZARD_BUILD_SHARED_LIB)
        add_library(${PROJECT_NAME} SHARED)
        set_target_properties(${PROJECT_NAME} PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
    else()
        add_library(${PROJECT_NAME} STATIC)
    endif()
else()
    add_executable(${PROJECT_NAME})
endif()

if(WIZARD_USE_EXTERNAL_ASMJIT)
    find_library(ASMJIT_LIBRARY NAMES asmjit)
    find_path(ASMJIT_INCLUDE_DIR NAMES asmjit/asmjit.h)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${ASMJIT_LIBRARY})
    target_include_directories(${PROJECT_NAME} PUBLIC ${ASMJIT_INCLUDE_DIR})
else()
    target_link_libraries(${PROJECT_NAME} PRIVATE $<BUILD_INTERFACE:asmjit>)
    target_include_directories(${PROJECT_NAME} PUBLIC "$<BUILD_INTERFACE:${ASMJIT_SRC}>")
endif()

if(WIZARD_USE_EXTERNAL_NLOHMANN_JSON)
    find_library(NLOHMANN_JSON_LIBRARY NAMES nlohmann_json)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${NLOHMANN_JSON_LIBRARY})
else()
    target_link_libraries(${PROJECT_NAME} PRIVATE $<BUILD_INTERFACE:nlohmann_json>)
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 20)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD_REQUIRED ON)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_EXTENSIONS ON)

target_include_directories(${PROJECT_NAME}
        PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
        INTERFACE
        $<INSTALL_INTERFACE:include>
        )

#Core
set(WIZARD_CORE_HEADERS
        ${PROJECT_SOURCE_DIR}/src/core/language_module.h
        ${PROJECT_SOURCE_DIR}/src/core/plugin_manager.h)

install(FILES ${WIZARD_CORE_HEADERS} DESTINATION include/wizard)

target_sources(${PROJECT_NAME} PRIVATE
        ${PROJECT_SOURCE_DIR}/src/core/plugin_manager.cpp)

target_precompile_headers(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/src/pch.h)

if(MSVC)
    if(MSVC)
        set(COMPILE_FLAGS_PLH "/W4 /Z7")
        if(NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
            set(COMPILE_FLAGS_PLH "/MP ${COMPILE_FLAGS_PLH}")
        endif()

        set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS ${COMPILE_FLAGS_PLH})
        target_link_libraries(${PROJECT_NAME} PRIVATE optimized -DEBUG)
    endif()
endif()

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wextra -Wshadow -Wconversion -Wpedantic -Werror)
endif()