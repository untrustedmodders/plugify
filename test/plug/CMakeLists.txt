cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

if(POLICY CMP0092)
    cmake_policy(SET CMP0092 NEW) # Don't add -W3 warning level by default.
endif()


project(plug VERSION 1.0.0.0  DESCRIPTION "Plugify Sample App" HOMEPAGE_URL "https://github.com/untrustedmodders/plugify" LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchCLI)

#
# Plug
#
file(GLOB_RECURSE PLUG_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.cpp")

add_executable(${PROJECT_NAME} ${PLUG_SOURCES})

find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE plugify::plugify glaze::glaze Threads::Threads CLI11 ${CMAKE_DL_LIBS})

target_compile_definitions(${PROJECT_NAME} PRIVATE ${PLUGIFY_COMPILE_DEFINITIONS})

if(NOT COMPILER_SUPPORTS_FORMAT)
    target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt-header-only)
endif()

if(MSVC)
    target_compile_definitions(${PROJECT_NAME} PRIVATE TEST_HAS_NO_EXCEPTIONS=1)
endif()
target_compile_options(${PROJECT_NAME} PRIVATE ${PLUGIFY_COMPILE_OPTIONS})

if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE NOMINMAX=1)
elseif(APPLE)
    target_compile_options(${PROJECT_NAME} PRIVATE -Wno-sign-conversion)
endif()

add_custom_command(TARGET plug POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:plugify> $<TARGET_FILE_DIR:${PROJECT_NAME}>
)
