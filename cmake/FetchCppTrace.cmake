cmake_policy(PUSH)

if(POLICY CMP0169)
    # Allow calling FetchContent_Populate directly.
    cmake_policy(SET CMP0169 OLD)
endif()

include(FetchContent)

message(STATUS "Pulling and configuring cpptrace")

FetchContent_Declare(
        cpptrace
        GIT_REPOSITORY ${PLUGIFY_CPPTRACE_REPO}
        GIT_TAG ${PLUGIFY_CPPTRACE_TAG}
        GIT_SHALLOW TRUE
)

if(LINUX OR MINGW)
    add_compile_definitions(_GNU_SOURCE)
endif()

FetchContent_MakeAvailable(cpptrace)

cmake_policy(POP)
