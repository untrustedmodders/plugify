include(FetchContent)

message(STATUS "Pulling and configuring asmjit")

FetchContent_Declare(
        asmjit
        GIT_REPOSITORY ${PLUGIFY_ASMJIT_REPO}
        GIT_TAG ${PLUGIFY_ASMJIT_TAG}
        GIT_PROGRESS TRUE
        GIT_SHALLOW TRUE
        #OVERRIDE_FIND_PACKAGE
)

set(ASMJIT_STATIC $<BOOL:${PLUGIFY_BUILD_SHARED_ASMJIT}> CACHE BOOL "Build static library")

FetchContent_MakeAvailable(asmjit)
