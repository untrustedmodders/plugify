include(FetchContent)

message(STATUS "Pulling and configuring mamba")

FetchContent_Declare(
        mamba
        GIT_REPOSITORY ${PLUGIYFY_MAMBA_REPO}
        GIT_TAG ${PLUGIYFY_MAMBA_TAG}
        GIT_SHALLOW TRUE
)

set(BUILD_MICROMAMBA OFF CACHE INTERNAL "Build micromamba" OFF)

FetchContent_MakeAvailable(mamba)
