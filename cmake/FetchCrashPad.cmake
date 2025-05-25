include(FetchContent)

message(STATUS "Pulling and configuring crashpad_cmake")

FetchContent_Declare(
        crashpad_cmake
        GIT_REPOSITORY ${PLUGIFY_CRASHPAD_REPO}
        GIT_TAG        ${PLUGIFY_CRASHPAD_TAG}
)

FetchContent_MakeAvailable(crashpad_cmake)

if(LINUX)
    file(GENERATE
            OUTPUT "${CMAKE_BINARY_DIR}/pch.h"
            CONTENT "#pragma once\n#include <cstdint>\n"
    )
    target_compile_options(crashpad_snapshot PRIVATE -Wno-template-id-cdtor)
    target_precompile_headers(minichromium PUBLIC "${CMAKE_BINARY_DIR}/pch.h")
    target_compile_definitions(minichromium PUBLIC -DCRASHPAD_USE_BORINGSSL=1)
    find_package(OpenSSL REQUIRED)
    target_link_libraries(minichromium PUBLIC OpenSSL::SSL OpenSSL::Crypto)
endif()

add_dependencies(${PROJECT_NAME} crashpad_handler)