include(FetchContent)

message(STATUS "Pulling and configuring miniz")

FetchContent_Declare(
        miniz
        GIT_REPOSITORY https://github.com/richgel999/miniz.git
        GIT_TAG 3.0.2
)

FetchContent_MakeAvailable(miniz)
