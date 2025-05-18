function(disable_compiler_warnings_for_target target)
    if(MSVC)
        target_compile_options(${target} PRIVATE "/W0")
    else()
        target_compile_options(${target} PRIVATE "-w")
    endif()
endfunction()

function(detect_operating_system)
    message(STATUS "CMake Version: ${CMAKE_VERSION}")
    message(STATUS "CMake System Name: ${CMAKE_SYSTEM_NAME}")

    if (CMAKE_SYSTEM_NAME MATCHES "Linux")
        # Have to make it visible in this scope as well for below.
        set(LINUX TRUE PARENT_SCOPE)
        set(LINUX TRUE)
    endif()

    if(UNIX OR CYGWIN)
        set(POSIX TRUE PARENT_SCOPE)
    endif()

    if(WIN32)
        set(PLUGIFY_PLATFORM "windows" PARENT_SCOPE)
        message(STATUS "Building for Windows.")
    elseif(APPLE AND NOT IOS)
        set(PLUGIFY_PLATFORM "apple" PARENT_SCOPE)
        message(STATUS "Building for MacOS.")
    elseif(LINUX)
        set(PLUGIFY_PLATFORM "linux" PARENT_SCOPE)
        message(STATUS "Building for Linux.")
    elseif(BSD)
        set(PLUGIFY_PLATFORM "bsd" PARENT_SCOPE)
        message(STATUS "Building for BSD.")
    elseif(ORBIS)
        set(PLUGIFY_PLATFORM "orbis" PARENT_SCOPE)
        message(STATUS "Building for Orbis.")
    elseif(PROSPERO)
        set(PLUGIFY_PLATFORM "prospero" PARENT_SCOPE)
        message(STATUS "Building for Prospero.")
    elseif(SWITCH)
        set(PLUGIFY_PLATFORM "switch" PARENT_SCOPE)
        message(STATUS "Building for Switch.")
    else()
        message(FATAL_ERROR "Unsupported platform.")
    endif()
endfunction()

function(detect_compiler)
    if(MSVC AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        set(PLUGIFY_COMPILER_CLANG_CL TRUE PARENT_SCOPE)
        message(STATUS "Building with Clang-CL.")
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
        set(PLUGIFY_COMPILER_CLANG TRUE PARENT_SCOPE)
        message(STATUS "Building with Clang/LLVM.")
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set(PLUGIFY_COMPILER_GCC TRUE PARENT_SCOPE)
        message(STATUS "Building with GNU GCC.")
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
        set(PLUGIFY_COMPILER_INTEL TRUE PARENT_SCOPE)
        message(STATUS "Building with Intel.")
    elseif(MSVC)
        set(PLUGIFY_COMPILER_MSVC TRUE PARENT_SCOPE)
        message(STATUS "Building with MSVC.")
    else()
        message(FATAL_ERROR "Unknown compiler: ${CMAKE_CXX_COMPILER_ID}")
    endif()
endfunction()

function(detect_architecture)
    if(APPLE AND NOT "${CMAKE_OSX_ARCHITECTURES}" STREQUAL "")
        # Universal binaries.
        if("x86_64" IN_LIST CMAKE_OSX_ARCHITECTURES)
            message(STATUS "Building x86_64 MacOS binaries.")
            set(PLUGIFY_CPU_ARCH_X64 TRUE PARENT_SCOPE)
        endif()
        if("arm64" IN_LIST CMAKE_OSX_ARCHITECTURES)
            message(STATUS "Building ARM64 MacOS binaries.")
            set(PLUGIFY_CPU_ARCH_ARM64 TRUE PARENT_SCOPE)
        endif()
    elseif(("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_64" OR "${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "amd64" OR
            "${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "AMD64" OR "${CMAKE_OSX_ARCHITECTURES}" STREQUAL "x86_64") AND
            CMAKE_SIZEOF_VOID_P EQUAL 8)
        message(STATUS "Building x86_64 binaries.")
        set(PLUGIFY_CPU_ARCH_X64 TRUE PARENT_SCOPE)
    elseif(("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "aarch64" OR "${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "arm64") AND
            CMAKE_SIZEOF_VOID_P EQUAL 8)
        message(STATUS "Building ARM64 binaries.")
        set(PLUGIFY_CPU_ARCH_ARM64 TRUE PARENT_SCOPE)
    elseif("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "arm" OR "${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "armv7-a" OR
            "${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "armv7l" OR
    (("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "aarch64" OR "${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "arm64")
            AND CMAKE_SIZEOF_VOID_P EQUAL 4))
        message(STATUS "Building ARM32 binaries.")
        set(PLUGIFY_CPU_ARCH_ARM32 TRUE PARENT_SCOPE)
    elseif("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "riscv64")
        message(STATUS "Building RISC-V 64 binaries.")
        set(PLUGIFY_CPU_ARCH_RISCV64 TRUE PARENT_SCOPE)
    else()
        message(FATAL_ERROR "Unknown system processor: ${CMAKE_SYSTEM_PROCESSOR}")
    endif()
endfunction()

function(detect_system outvar)
    string(TOLOWER "${CMAKE_SYSTEM_NAME}" system)
    string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" arch)
    if(system STREQUAL "darwin")
        set(system "osx")
        list(LENGTH CMAKE_OSX_ARCHITECTURES osx_archcount)
        if(osx_archcount GREATER 1)
            message(FATAL_ERROR "More than one CMAKE_OSX_ARCHITECTURES is not supported")
        endif()
        set(arch "${CMAKE_OSX_ARCHITECTURES}")
    endif()
    message(STATUS "system: ${system}")
    message(STATUS "arch: ${arch}")

    if(arch MATCHES "ar.*64|armv8l")
        set(arch "arm64")
    elseif(arch MATCHES "arm")
        set(arch "arm")
    elseif(arch MATCHES "64")
        set(arch "x64")
    elseif(arch MATCHES "86")
        set(arch "x86")
    else()
        message(FATAL_ERROR "Unrecognized CMAKE_SYSTEM_PROCESSOR: ${CMAKE_SYSTEM_PROCESSOR}")
    endif()
    set(${outvar} "${system}-${arch}" PARENT_SCOPE)
endfunction()

include(CheckCXXSourceCompiles)
include(CMakePushCheckState)

function(check_cpp20_feature MACRO MINIMUM_VALUE)
    set(CACHE_VAR "PLUGIFY_CHECK_CPP20_FEATURE_${MACRO}")
    if(NOT DEFINED ${CACHE_VAR})
        cmake_push_check_state() # Save current state

        set(TEST_CODE "#include <version>
#if !defined(${MACRO}) || ${MACRO} < ${MINIMUM_VALUE}L
#error Missing feature
#endif
        int main() { return 0; }")

        check_cxx_source_compiles("${TEST_CODE}" HAS_FEATURE)
        set(${CACHE_VAR} ${HAS_FEATURE} CACHE INTERNAL "Cached feature test result for ${MACRO}")

        cmake_pop_check_state() # Restore previous state
    endif()

    if(NOT HAS_FEATURE)
        message(FATAL_ERROR "${MACRO} is not supported by your compiler, at least ${MINIMUM_VALUE} is required.")
    endif()
endfunction()

function(check_cpp20_attribute ATTRIBUTE MINIMUM_VALUE)
    set(CACHE_VAR "PLUGIFY_CHECK_CPP20_ATTRIBUTE_${ATTRIBUTE}")
    if(NOT DEFINED ${CACHE_VAR})
        cmake_push_check_state()

        set(TEST_CODE "#include <version>
#if !defined(__has_cpp_attribute) || __has_cpp_attribute(${ATTRIBUTE}) < ${MINIMUM_VALUE}L
#error Missing feature
#endif
        int main() { return 0; }")

        check_cxx_source_compiles("${TEST_CODE}" HAS_FEATURE)
        set(${CACHE_VAR} ${HAS_FEATURE} CACHE INTERNAL "Cached attribute test result for ${ATTRIBUTE}")

        cmake_pop_check_state()
    endif()

    if(NOT HAS_FEATURE)
        message(FATAL_ERROR "${ATTRIBUTE} is not supported by your compiler, at least ${MINIMUM_VALUE} is required.")
    endif()
endfunction()
