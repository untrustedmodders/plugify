include(FetchContent)

message(STATUS "Pulling and configuring fmt")

FetchContent_Declare(
        fmt
        GIT_REPOSITORY ${PLUGIFY_FMT_REPO}
        GIT_TAG ${PLUGIFY_FMT_TAG}
        GIT_SHALLOW TRUE
)

set(FMT_DOC OFF CACHE INTERNAL "Generate the doc target.")
set(FMT_INSTALL OFF CACHE INTERNAL "Generate the install target.")
set(FMT_TEST OFF CACHE INTERNAL "Generate the test target.")
set(FMT_FUZZ OFF CACHE INTERNAL "Generate the fuzz target.")
set(FMT_CUDA_TEST OFF CACHE INTERNAL "Generate the cuda-test target.")

FetchContent_MakeAvailable(fmt)
