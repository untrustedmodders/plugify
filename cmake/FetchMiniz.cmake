include(FetchContent)

message(STATUS "Pulling and configuring miniz")

FetchContent_Declare(
        miniz
        GIT_REPOSITORY ${PLUGIFY_MINIZ_REPO}
        GIT_TAG ${PLUGIFY_MINIZ_TAG}
        PATCH_COMMAND git reset --hard && git clean -f -d && git apply ${CMAKE_CURRENT_SOURCE_DIR}/cmake/patches/miniz.patch
)

FetchContent_MakeAvailable(miniz)
