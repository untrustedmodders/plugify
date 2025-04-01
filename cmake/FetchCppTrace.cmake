include(FetchContent)

message(STATUS "Pulling and configuring cpptrace")

FetchContent_Declare(
        cpptrace
        GIT_REPOSITORY https://github.com/jeremy-rifkin/cpptrace.git
        GIT_TAG 7543677d6f39a38de9e9248e2331015d5235b175
)

FetchContent_MakeAvailable(cpptrace)
