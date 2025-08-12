set(CMAKE_POLICY_DEFAULT_CMP0048 NEW)

include(FetchContent)

message(STATUS "Pulling and configuring zlib")

FetchContent_Declare(
        zlib
        URL ${PLUGIFY_ZLIB_LINK}
        URL_HASH SHA256=${PLUGIFY_ZLIB_HASH}
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

FetchContent_MakeAvailable(zlib)

# Set ZLIB variables for libsolv to find
add_library(ZLIB::ZLIB ALIAS zlibstatic)
target_include_directories(zlibstatic INTERFACE ${zlib_BINARY_DIR} ${zlib_SOURCE_DIR})

# 1. Set the variables that are usually set by FindZLIB.cmake.
# 2. Add the module that stubs out `find_package(ZLIB ...)` calls.
set(ZLIB_FOUND ON)
set(ZLIB_LIBRARY ZLIB::ZLIB)
set(ZLIB_LIBRARIES ZLIB::ZLIB)
set(ZLIB_INCLUDE_DIR ${zlib_SOURCE_DIR} ${zlib_BINARY_DIR})
set(ZLIB_INCLUDE_DIRS ${zlib_SOURCE_DIR} ${zlib_BINARY_DIR})