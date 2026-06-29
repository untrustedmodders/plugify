include(FetchContent)

message(STATUS "Pulling and configuring tracy")

set(TRACY_NO_CRASH_HANDLER ON CACHE BOOL "" FORCE)
set(TRACY_ON_DEMAND ON CACHE BOOL "" FORCE)
set(TRACY_STATIC ON CACHE BOOL "" FORCE)

FetchContent_Declare(
        tracy
        GIT_REPOSITORY ${PLUGIFY_TRACY_REPO}
        GIT_TAG ${PLUGIFY_TRACY_TAG}
        GIT_PROGRESS TRUE
        GIT_SHALLOW TRUE
        OVERRIDE_FIND_PACKAGE
        EXCLUDE_FROM_ALL
)

FetchContent_MakeAvailable(tracy)