include(FetchContent)

message(STATUS "Pulling and configuring glaze")

FetchContent_Declare(
        reproc
        GIT_REPOSITORY ${PLUGIFY_REPROC_REPO}
        GIT_TAG ${PLUGIFY_REPROC_TAG}
        GIT_SHALLOW TRUE
)

set(REPROC++ ON "Build reproc++" CACHE STRING "")
SET(REPROC_TEST OFF)
SET(REPROC_EXAMPLES OFF)

#FetchContent_GetProperties(reproc)
#if(NOT reproc_POPULATED)
#    FetchContent_Populate(reproc)
#    add_subdirectory(${reproc_SOURCE_DIR} ${reproc_BINARY_DIR} EXCLUDE_FROM_ALL)
#endif()
FetchContent_MakeAvailable(reproc)

# 1. Set the variables that are usually set by FindZLIB.cmake.
# 2. Add the module that stubs out `find_package(ZLIB ...)` calls.
set(reproc_FOUND ON)
set(reproc_LIBRARY reproc::reproc)
set(reproc_LIBRARIES reproc::reproc)
set(reproc_INCLUDE_DIR ${reproc_SOURCE_DIR} ${reproc_BINARY_DIR})
set(reproc_INCLUDE_DIRS ${reproc_SOURCE_DIR} ${reproc_BINARY_DIR})