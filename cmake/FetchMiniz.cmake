include(FetchContent)

message(STATUS "Pulling and configuring miniz")

FetchContent_Declare(
        miniz
        GIT_REPOSITORY https://github.com/richgel999/miniz.git
        GIT_TAG 3.0.2
        PATCH_COMMAND git apply ${CMAKE_CURRENT_SOURCE_DIR}/cmake/patches/miniz.patch
)

FetchContent_MakeAvailable(miniz)
