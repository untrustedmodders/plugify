include(FetchContent)

message(STATUS "Pulling and configuring asmjit")

FetchContent_Declare(
        asmjit
        GIT_REPOSITORY ${PLUGIFY_ASMJIT_REPO}
        GIT_TAG ${PLUGIFY_ASMJIT_TAG}
        GIT_PROGRESS TRUE
        #GIT_SHALLOW TRUE
        OVERRIDE_FIND_PACKAGE
)

set(ASMJIT_STATIC $<BOOL:${PLUGIFY_BUILD_SHARED_ASMJIT}> CACHE BOOL "Build static library")

FetchContent_MakeAvailable(asmjit)

if(MSVC)
    target_compile_options(asmjit PUBLIC /wd5054)
elseif(MINGW)
    target_compile_options(asmjit PUBLIC -Wno-deprecated-enum-enum-conversion -Wno-shadow -Wno-pedantic)
else()
    target_compile_options(asmjit PUBLIC -Wno-deprecated-anon-enum-enum-conversion -Wno-deprecated-enum-enum-conversion -Wno-shadow -Wno-pedantic)
endif()

set_target_properties(asmjit PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        POSITION_INDEPENDENT_CODE ON
)