message(STATUS "Pulling and configuring libsolv")

set(ENABLE_STATIC ON CACHE INTERNAL "Build a static version of the libraries?")
set(DISABLE_SHARED ON CACHE INTERNAL "Do not build a shared version of the libraries?")
set(ENABLE_ZLIB_COMPRESSION OFF CACHE INTERNAL "Build with zlib compression support?")

FetchContent_Declare(
        libsolv
        GIT_REPOSITORY ${PLUGIFY_LIBSOLV_REPO}
        GIT_TAG ${PLUGIFY_LIBSOLV_TAG}
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
        PATCH_COMMAND git reset --hard && git apply --ignore-whitespace --whitespace=nowarn --reject ${CMAKE_CURRENT_SOURCE_DIR}/patches/libsolv.patch
)

FetchContent_MakeAvailable(libsolv)

add_library(solv::libsolv ALIAS libsolv)
add_library(solv::libsolvext ALIAS libsolvext)
add_library(solv::libsolv_static ALIAS libsolv)
add_library(solv::libsolvext_static ALIAS libsolvext)

file(COPY ${libsolv_SOURCE_DIR}/src/ DESTINATION ${libsolv_BINARY_DIR}/include/solv)
file(COPY ${libsolv_SOURCE_DIR}/ext/ DESTINATION ${libsolv_BINARY_DIR}/ext/inlcude/solv)
target_include_directories(libsolv PUBLIC ${libsolv_BINARY_DIR}/include ${libsolv_BINARY_DIR}/src)
target_include_directories(libsolvext PUBLIC ${libsolv_SOURCE_DIR}/src ${libsolv_BINARY_DIR}/ext/include ${libsolv_BINARY_DIR}/src/ext)

# 1. Set the variables that are usually set by FindLibsolv.cmake.
# 2. Add the module that stubs out `find_package(Libsolv ...)` calls.
set(Libsolv_FOUND ON)
set(Libsolv_LIBRARY solv::libsolv solv::libsolvext)
set(Libsolv_LIBRARIES solv::libsolv solv::libsolvext)
set(Libsolv_INCLUDE_DIR ${libsolv_SOURCE_DIR} ${libsolv_BINARY_DIR})
set(Libsolv_INCLUDE_DIRS ${libsolv_SOURCE_DIR} ${libsolv_BINARY_DIR})

foreach(_t IN ITEMS libsolv libsolvext)
    if(TARGET ${_t})
        # treat libsolv headers as system headers -> suppresses warnings coming from headers
        target_include_directories(${_t} SYSTEM PUBLIC
                ${libsolv_BINARY_DIR}/include
                ${libsolv_BINARY_DIR}/src
        )

        # force linker language to C (helps when top-level project is C++)
        set_target_properties(${_t} PROPERTIES
                LINKER_LANGUAGE C
                C_STANDARD 99
                C_STANDARD_REQUIRED ON
                POSITION_INDEPENDENT_CODE ON
        )

        # define WITHOUT_COOKIEOPEN to disable cookieopen support
        target_compile_definitions(${_t} PRIVATE WITHOUT_COOKIEOPEN)

        # disable warnings only for this third-party target
        if(MSVC)
            # /w disables all warnings on MSVC
            target_compile_options(${_t} PRIVATE /w)
        else()
            # -w disables all warnings for GCC/Clang
            target_compile_options(${_t} PRIVATE -w)
        endif()
    endif()
endforeach()

