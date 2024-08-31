include(FetchContent)

message(STATUS "Pulling and configuring glaze")

FetchContent_Declare(
        glaze
        GIT_REPOSITORY https://github.com/stephenberry/glaze.git
        GIT_TAG v2.9.0
)

FetchContent_MakeAvailable(glaze)