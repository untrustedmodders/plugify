message(STATUS "Pulling and configuring zlib")

set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

FetchContent_Declare(
      zlib
      GIT_REPOSITORY https://github.com/madler/zlib.git
      GIT_TAG v1.3.1
      GIT_SHALLOW TRUE
      GIT_PROGRESS TRUE
      OVERRIDE_FIND_PACKAGE
)

FetchContent_MakeAvailable(zlib)

# Make sure headers are available for the static target and make an alias to match the Find module output.
target_include_directories(zlibstatic INTERFACE ${zlib_SOURCE_DIR} ${zlib_BINARY_DIR})
add_library(ZLIB::ZLIB ALIAS zlibstatic)

set(ZLIB_LIBRARY zlibstatic)
set(ZLIB_INCLUDE_DIR ${zlib_SOURCE_DIR} ${zlib_BINARY_DIR})