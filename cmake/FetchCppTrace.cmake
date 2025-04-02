include(FetchContent)

message(STATUS "Pulling and configuring cpptrace")

FetchContent_Declare(
        cpptrace
        GIT_REPOSITORY ${PLUGIFY_CPPTRACE_REPO}
        GIT_TAG ${PLUGIFY_CPPTRACE_TAG}
        GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(cpptrace)
