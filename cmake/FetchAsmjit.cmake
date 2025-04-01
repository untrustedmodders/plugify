include(FetchContent)

message(STATUS "Pulling and configuring asmjit")

FetchContent_Declare(
        asmjit
        GIT_REPOSITORY https://github.com/asmjit/asmjit.git
        GIT_TAG e8c8e2e48a1a38154c8e8864eb3bc61db80a1e31
)

set(ASMJIT_STATIC $<BOOL:${PLUGIFY_BUILD_SHARED_ASMJIT}> CACHE BOOL "Build static library")

FetchContent_MakeAvailable(asmjit)
