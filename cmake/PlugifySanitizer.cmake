if(PLUGIFY_USE_SANITIZER)
    if(PLUGIFY_COMPILER_GCC OR PLUGIFY_COMPILER_CLANG)
        set(PLUGIFY_HAS_SANITIZER TRUE CACHE BOOL "" FORCE)
        mark_as_advanced(PLUGIFY_HAS_SANITIZER)
    endif()

    if(PLUGIFY_HAS_SANITIZER)
        target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>: -fsanitize=address -fno-omit-frame-pointer -fsanitize=undefined>)
        target_link_libraries(${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>: -fsanitize=address -fno-omit-frame-pointer -fsanitize=undefined>)
    else()
        message(VERBOSE "The option PLUGIFY_USE_SANITIZER is set but sanitizer support is not available.")
    endif()
endif()

if(PLUGIFY_USE_CLANG_TIDY)
    find_program(PLUGIFY_CLANG_TIDY_EXECUTABLE "clang-tidy")

    if(PLUGIFY_CLANG_TIDY_EXECUTABLE)
        set(CMAKE_CXX_CLANG_TIDY "${PLUGIFY_CLANG_TIDY_EXECUTABLE}; --config-file=.clang-tidy; --header-filter=src/.*; --extra-arg=/EHsc")
    else()
        message(VERBOSE "The option PLUGIFY_USE_CLANG_TIDY is set but clang-tidy executable is not available.")
    endif()
endif()
