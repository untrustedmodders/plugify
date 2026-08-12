include(FetchContent)

message(STATUS "Pulling and configuring valijson")

FetchContent_Declare(
        valijson
        GIT_REPOSITORY ${PLUGIFY_VALIJSON_REPO}
        GIT_TAG ${PLUGIFY_VALIJSON_TAG}
        GIT_SHALLOW TRUE
        GIT_SUBMODULES ""
)

set(valijson_BUILD_TESTS OFF CACHE BOOL "Don't build valijson tests" FORCE)
set(valijson_BUILD_EXAMPLES OFF CACHE BOOL "Don't build valijson examples" FORCE)

FetchContent_MakeAvailable(valijson)

if(TARGET valijson)
    # treat valijson headers as system headers -> suppresses warnings coming from headers
    target_include_directories(valijson SYSTEM INTERFACE ${valijson_SOURCE_DIR}/include)
endif()