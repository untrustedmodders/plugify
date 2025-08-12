include(FetchContent)

message(STATUS "Pulling and configuring libsolv")

FetchContent_Declare(
        libsolv
        GIT_REPOSITORY ${PLUGIFY_LIBSOLV_REPO}
        GIT_TAG ${PLUGIFY_LIBSOLV_TAG}
        GIT_SHALLOW TRUE
        PATCH_COMMAND git reset --hard && git apply ${CMAKE_CURRENT_SOURCE_DIR}/patches/libsolv.patch
)

set(ENABLE_STATIC ON CACHE INTERNAL "Build a static version of the libraries?" OFF)
set(DISABLE_SHARED OFF CACHE INTERNAL "Do not build a shared version of the libraries?" OFF)

FetchContent_MakeAvailable(libsolv)
