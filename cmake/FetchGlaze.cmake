include(FetchContent)

message(STATUS "Pulling and configuring glaze")

FetchContent_Declare(
        glaze
        GIT_REPOSITORY ${PLUGIFY_GLAZE_REPO}
        GIT_TAG ${PLUGIFY_GLAZE_TAG}
        GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(glaze)
