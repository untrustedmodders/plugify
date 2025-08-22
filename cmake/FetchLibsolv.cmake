message(STATUS "Pulling and configuring libsolv")

set(ENABLE_STATIC ON CACHE INTERNAL "Build a static version of the libraries?")
set(DISABLE_SHARED ON CACHE INTERNAL "Do not build a shared version of the libraries?")

FetchContent_Declare(
        libsolv
        GIT_REPOSITORY https://github.com/openSUSE/libsolv.git
        GIT_TAG 0.7.35
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
        PATCH_COMMAND git reset --hard && git apply --ignore-whitespace --whitespace=nowarn --reject ${CMAKE_CURRENT_SOURCE_DIR}/patches/libsolv.patch
)

FetchContent_MakeAvailable(libsolv)

# Set libsolv variables for mamba to find
add_library(solv::libsolv ALIAS libsolv)
add_library(solv::libsolvext ALIAS libsolvext)
add_library(solv::libsolv_static ALIAS libsolv)
add_library(solv::libsolvext_static ALIAS libsolvext)

set_property(TARGET libsolv PROPERTY POSITION_INDEPENDENT_CODE ON)
set_property(TARGET libsolvext PROPERTY POSITION_INDEPENDENT_CODE ON)
file(COPY ${libsolv_SOURCE_DIR}/src/ DESTINATION ${libsolv_BINARY_DIR}/include/solv)
file(COPY ${libsolv_SOURCE_DIR}/ext/ DESTINATION ${libsolv_BINARY_DIR}/ext/include/solv)
target_include_directories(libsolv PUBLIC ${libsolv_BINARY_DIR}/include ${libsolv_BINARY_DIR}/src)
target_include_directories(libsolvext PUBLIC ${libsolv_SOURCE_DIR}/src ${libsolv_BINARY_DIR}/ext/include ${libsolv_BINARY_DIR}/src/ext)

target_compile_definitions(libsolvext PUBLIC WITHOUT_COOKIEOPEN)

# 1. Set the variables that are usually set by FindLibsolv.cmake.
# 2. Add the module that stubs out `find_package(Libsolv ...)` calls.
set(Libsolv_FOUND ON)
set(Libsolv_LIBRARY solv::libsolv solv::libsolvext)
set(Libsolv_LIBRARIES solv::libsolv solv::libsolvext)
set(Libsolv_INCLUDE_DIR ${libsolv_SOURCE_DIR} ${libsolv_BINARY_DIR})
set(Libsolv_INCLUDE_DIRS ${libsolv_SOURCE_DIR} ${libsolv_BINARY_DIR})
