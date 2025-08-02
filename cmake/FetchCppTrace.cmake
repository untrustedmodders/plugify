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
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-implicit-function-declaration -Wno-int-conversion" CACHE STRING "" FORCE)
endif()

FetchContent_MakeAvailable(cpptrace)

cmake_policy(POP)