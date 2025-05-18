include(FetchContent)

message(STATUS "Pulling and configuring asmjit")

FetchContent_Declare(
        asmjit
        GIT_REPOSITORY ${PLUGIFY_ASMJIT_REPO}
        GIT_TAG ${PLUGIFY_ASMJIT_TAG}
        PATCH_COMMAND git reset --hard && git apply --ignore-whitespace ${CMAKE_CURRENT_SOURCE_DIR}/.github/patches/asmjit.patch
)

set(ASMJIT_STATIC $<BOOL:${PLUGIFY_BUILD_SHARED_ASMJIT}> CACHE BOOL "Build static library")

FetchContent_MakeAvailable(asmjit)
