cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

if(POLICY CMP0092)
    cmake_policy(SET CMP0092 NEW) # Don't add -W3 warning level by default.
endif()


project(plug VERSION 1.0.0.0  DESCRIPTION "Plugify Sample App" HOMEPAGE_URL "https://github.com/untrustedmodders/plugify" LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

#
# Catch2
#
message(STATUS "Pulling and configuring catch2")

FetchContent_Declare(
        cli
        GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
        GIT_TAG v2.5.0
        GIT_PROGRESS TRUE
        GIT_SHALLOW TRUE
        OVERRIDE_FIND_PACKAGE
)
FetchContent_MakeAvailable(cli)

#
# Plug
#
file(GLOB_RECURSE PLUG_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.cpp")

add_executable(${PROJECT_NAME} ${PLUG_SOURCES})

target_link_libraries(${PROJECT_NAME} PRIVATE plugify::plugify plugify::plugify-assembly CLI11 ${CMAKE_DL_LIBS}) # plugify::plugify-jit

target_compile_definitions(${PROJECT_NAME} PRIVATE ${PLUGIFY_COMPILE_DEFINITIONS})

if(NOT COMPILER_SUPPORTS_FORMAT)
    target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt-header-only)
endif()

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
    target_compile_definitions(${PROJECT_NAME} PRIVATE TEST_HAS_NO_EXCEPTIONS=1)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wshadow -Wconversion -Wpedantic -Werror)
endif()

if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE NOMINMAX=1)
elseif(APPLE)
    target_compile_options(${PROJECT_NAME} PRIVATE -Wno-sign-conversion)
endif()

add_custom_command(TARGET plug POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:plugify> $<TARGET_FILE_DIR:${PROJECT_NAME}>
)

#add_custom_command(TARGET plug POST_BUILD
#        COMMAND ${CMAKE_COMMAND} -E copy_if_different
#        $<TARGET_FILE:crashpad_handler> $<TARGET_FILE_DIR:${PROJECT_NAME}>
#)
#
#file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/crashpad)
