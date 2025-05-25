if(PLUGIFY_BUILD_TESTS)
    set(PLUGIFY_BUILD_JIT ON)
    set(PLUGIFY_BUILD_ASSEMBLY ON)
    #set(PLUGIFY_BUILD_CRASHPAD ON)
endif()

# ------------------------------------------------------------------------------
# Jit
if(PLUGIFY_BUILD_JIT)
    if(PLUGIFY_CPU_ARCH_ARM64)
        set(PLUGIYFY_JIT_ARCH "arm64")
    elseif(PLUGIFY_CPU_ARCH_ARM32)
        set(PLUGIYFY_JIT_ARCH "arm32")
    else()
        set(PLUGIYFY_JIT_ARCH "x86")
    endif()
    set(PLUGIFY_JIT_SOURCES
            "${CMAKE_CURRENT_SOURCE_DIR}/include/plugify/jit/callback_${PLUGIYFY_JIT_ARCH}.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/include/plugify/jit/call_${PLUGIYFY_JIT_ARCH}.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/include/plugify/jit/helpers_${PLUGIYFY_JIT_ARCH}.cpp"
    )
    add_library(${PROJECT_NAME}-jit OBJECT ${PLUGIFY_JIT_SOURCES})
    add_library(${PROJECT_NAME}::${PROJECT_NAME}-jit ALIAS ${PROJECT_NAME}-jit)
    target_include_directories(${PROJECT_NAME}-jit PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
    if(PLUGIFY_USE_EXTERNAL_ASMJIT)
        find_package(asmjit REQUIRED)
    else()
        include(FetchAsmjit)
        target_include_directories(${PROJECT_NAME}-jit PRIVATE ${ASMJIT_SRC})
    endif()
    target_link_libraries(${PROJECT_NAME}-jit PRIVATE asmjit::asmjit)
    if(MSVC)
        target_compile_options(asmjit PUBLIC /wd5054)
    elseif(MINGW)
        target_compile_options(asmjit PUBLIC -Wno-deprecated-enum-enum-conversion)
    else()
        target_compile_options(asmjit PUBLIC -Wno-deprecated-anon-enum-enum-conversion -Wno-deprecated-enum-enum-conversion)
    endif()
    target_compile_definitions(${PROJECT_NAME}-jit PRIVATE
            ${PLUGIFY_COMPILE_DEFINITIONS}
            PLUGIFY_SEPARATE_SOURCE_FILES=1
    )
    target_include_directories(${PROJECT_NAME}-jit PUBLIC ${CMAKE_BINARY_DIR}/exports)
    if(LINUX)
        target_compile_definitions(${PROJECT_NAME}-jit PUBLIC _GLIBCXX_USE_CXX11_ABI=$<IF:$<BOOL:${PLUGIFY_USE_ABI0}>,0,1>)
    endif()
    if(PLUGIFY_HAS_LIBCPP)
        target_compile_options(${PROJECT_NAME}-jit PUBLIC -stdlib=libc++)
    endif()
endif()

# ------------------------------------------------------------------------------
# Assembly
if(PLUGIFY_BUILD_ASSEMBLY)
    add_library(${PROJECT_NAME}-assembly OBJECT "${CMAKE_CURRENT_SOURCE_DIR}/src/utils/assembly.cpp")
    add_library(${PROJECT_NAME}::${PROJECT_NAME}-assembly ALIAS ${PROJECT_NAME}-assembly)
    target_include_directories(${PROJECT_NAME}-assembly PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
    target_compile_definitions(${PROJECT_NAME}-assembly PRIVATE
            ${PLUGIFY_COMPILE_DEFINITIONS}
            PLUGIFY_SEPARATE_SOURCE_FILES=1
    )
    target_include_directories(${PROJECT_NAME}-assembly PUBLIC ${CMAKE_BINARY_DIR}/exports)
    if(LINUX)
        target_compile_definitions(${PROJECT_NAME}-assembly PUBLIC _GLIBCXX_USE_CXX11_ABI=$<IF:$<BOOL:${PLUGIFY_USE_ABI0}>,0,1>)
    endif()
    if(PLUGIFY_HAS_LIBCPP)
        target_compile_options(${PROJECT_NAME}-assembly PUBLIC -stdlib=libc++)
    endif()
endif()

