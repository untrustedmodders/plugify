include(FetchContent)

message(STATUS "Pulling and configuring asmjit")

FetchContent_Declare(
        asmjit
        GIT_REPOSITORY https://github.com/asmjit/asmjit.git
        GIT_TAG a63d41e80ba72f652c450d3bd6614327cde8531c
)

set(ASMJIT_STATIC $<BOOL:${PLUGIFY_BUILD_SHARED_ASMJIT}> CACHE BOOL "Build static library")

FetchContent_MakeAvailable(asmjit)