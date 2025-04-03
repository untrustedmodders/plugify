include(FetchContent)

message(STATUS "Pulling and configuring miniz")

FetchContent_Declare(
        miniz
        GIT_REPOSITORY ${PLUGIFY_MINIZ_REPO}
        GIT_TAG ${PLUGIFY_MINIZ_TAG}
)

FetchContent_MakeAvailable(miniz)
