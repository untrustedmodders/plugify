cmake_policy(PUSH)

if(POLICY CMP0169)
    # Allow calling FetchContent_Populate directly.
    cmake_policy(SET CMP0169 OLD)
endif()

include(FetchContent)

message(STATUS "Pulling and configuring crashpad_cmake")

FetchContent_Declare(
        crashpad_cmake
        GIT_REPOSITORY ${PLUGIFY_CRASHPAD_REPO}
        GIT_TAG ${PLUGIFY_CRASHPAD_TAG}
        GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(crashpad_cmake)

cmake_policy(POP)

if(LINUX)
    file(GENERATE
            OUTPUT "${CMAKE_BINARY_DIR}/pch.h"
            CONTENT "#pragma once\n#include <cstdint>\n"
    )
    target_precompile_headers(minichromium PUBLIC "${CMAKE_BINARY_DIR}/pch.h")
    if (PLUGIFY_COMPILER_GCC)
        target_compile_options(crashpad_snapshot PRIVATE -Wno-template-id-cdtor)
    endif()
    target_compile_definitions(minichromium PUBLIC -DCRASHPAD_USE_BORINGSSL=1)
    find_package(OpenSSL REQUIRED)
    target_link_libraries(minichromium PUBLIC OpenSSL::SSL OpenSSL::Crypto)
elseif(WIN32)
    add_compile_definitions(-DNOMINMAX=1)
endif()

add_dependencies(${PROJECT_NAME} crashpad_handler)