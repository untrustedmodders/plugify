include(FetchContent)

message(STATUS "Pulling and configuring CLI11")

FetchContent_Declare(
        cli
        GIT_REPOSITORY ${PLUGIFY_CLI_REPO}
        GIT_TAG ${PLUGIFY_CLI_TAG}
        GIT_PROGRESS TRUE
        GIT_SHALLOW TRUE
        OVERRIDE_FIND_PACKAGE
        EXCLUDE_FROM_ALL
)

FetchContent_MakeAvailable(cli)

