cmake_minimum_required(VERSION 3.21)
project(wizard)

# Force all compilers to use the correct C and C++ standard versions.
# Needed to allow for CheckCXXSourceCompiles to work correctly.
set(CMAKE_REQUIRED_FLAGS "-std=c++20 -std=c11")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Speed up building on MinGW
if(WIN32)
    add_link_options(-fuse-ld=lld)
endif()

find_package(PkgConfig REQUIRED)
pkg_check_modules(MONO REQUIRED mono-2)

find_package(Lua 5.1 REQUIRED)

file(GLOB_RECURSE WIZARD_SRC_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "src/wizard/*.cpp")
file(GLOB_RECURSE WIZARD_HDR_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "src/wizard/*.h")
set(WIZARD_PCH_FILE "src/pch.h")

add_executable(${PROJECT_NAME} ${WIZARD_SRC_FILES} ${WIZARD_HDR_FILES})

target_include_directories(${PROJECT_NAME} PUBLIC ${LUA_INCLUDE_DIR})
include_directories(SYSTEM "${MONO_INCLUDE_DIRS}")
target_link_directories(${PROJECT_NAME} PUBLIC ${MONO_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME} PUBLIC ${MONO_LIBRARIES} ${LUA_LIBRARIES})
target_precompile_headers(${PROJECT_NAME} PUBLIC ${WIZARD_PCH_FILE})

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wextra -Wshadow -Wconversion -Wpedantic -Werror)
endif()

execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
        ${CMAKE_CURRENT_SOURCE_DIR}/mono
        ${CMAKE_CURRENT_BINARY_DIR}/mono)
