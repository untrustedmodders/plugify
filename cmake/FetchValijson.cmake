include(FetchContent)

message(STATUS "Pulling and configuring valijson")

FetchContent_Declare(
        valijson
        GIT_REPOSITORY ${PLUGIFY_VALIJSON_REPO}
        GIT_TAG ${PLUGIFY_VALIJSON_TAG}
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
        PATCH_COMMAND git reset --hard && git apply --ignore-whitespace --whitespace=nowarn --reject ${CMAKE_CURRENT_SOURCE_DIR}/patches/valijson.patch
        GIT_SUBMODULES ""
        EXCLUDE_FROM_ALL
)

set(valijson_BUILD_TESTS OFF CACHE BOOL "Don't build valijson tests" FORCE)
set(valijson_BUILD_EXAMPLES OFF CACHE BOOL "Don't build valijson examples" FORCE)

FetchContent_MakeAvailable(valijson)
