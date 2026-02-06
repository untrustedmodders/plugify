include(FetchContent)

message(STATUS "Pulling and configuring catch2")

FetchContent_Declare(
        Catch2
        GIT_REPOSITORY ${PLUGIFY_CATCH2_REPO}
        GIT_TAG ${PLUGIFY_CATCH2_TAG}
        GIT_PROGRESS TRUE
        GIT_SHALLOW TRUE
        OVERRIDE_FIND_PACKAGE
        EXCLUDE_FROM_ALL
)

FetchContent_MakeAvailable(Catch2)

if(NOT MSVC)
    target_compile_options(Catch2 PUBLIC -Wno-conversion)
endif()