function(disable_compiler_warnings_for_target target)
    if(MSVC)
        target_compile_options(${target} PRIVATE "/W0")
    else()
        target_compile_options(${target} PRIVATE "-w")
    endif()
endfunction()

function(detect_system)
    message(STATUS "CMake Version: ${CMAKE_VERSION}")
    message(STATUS "CMake System Name: ${CMAKE_SYSTEM_NAME}")
    message(STATUS "CMake System Processor: ${CMAKE_SYSTEM_PROCESSOR}")

    string(TOLOWER "${CMAKE_SYSTEM_NAME}" PLUGIFY_SYSTEM)
    string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" PLUGIFY_ARCH)

    if(CMAKE_SYSTEM_NAME MATCHES "Linux")
        set(LINUX TRUE PARENT_SCOPE)

        # Check if /etc/os-release exists
        if(EXISTS "/etc/os-release")
            file(STRINGS "/etc/os-release" OS_RELEASE_ID_LINE REGEX "^ID=")
            string(REPLACE "ID=" "" OS_ID "${OS_RELEASE_ID_LINE}")
            string(REGEX REPLACE "^\"(.*)\"$" "\\1" OS_ID "${OS_ID}")
            string(TOLOWER "${OS_ID}" PLUGIFY_SYSTEM)
            message(STATUS "Detected Linux distribution: ${OS_ID}")
        else()
            message(WARNING "Cannot detect Linux distribution: /etc/os-release not found")
        endif()
    endif()

    if(UNIX OR CYGWIN)
        set(POSIX TRUE PARENT_SCOPE)
    endif()

    if(APPLE AND NOT "${CMAKE_OSX_ARCHITECTURES}" STREQUAL "")
        list(LENGTH CMAKE_OSX_ARCHITECTURES ARCH_COUNT)
        if(ARCH_COUNT GREATER 1)
            message(FATAL_ERROR "More than one architecture is not supported!")
        endif()
        list(GET CMAKE_OSX_ARCHITECTURES 0 ARCH_NAME)
        if(ARCH_NAME STREQUAL "x86_64")
            message(STATUS "Building x86_64 MacOS binaries")
            set(PLUGIFY_ARCH "x64")
            set(PLUGIFY_CPU_ARCH_X64 TRUE PARENT_SCOPE)
        elseif(ARCH_NAME STREQUAL "arm64")
            message(STATUS "Building ARM64 MacOS binaries")
            set(PLUGIFY_ARCH "arm64")
            set(PLUGIFY_CPU_ARCH_ARM64 TRUE PARENT_SCOPE)
        else()
            message(FATAL_ERROR "Unknown architecture: ${CMAKE_OSX_ARCHITECTURES}")
        endif()
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86|i[3456]86)$" OR CMAKE_GENERATOR_PLATFORM MATCHES "^(x86|Win32)$")
        message(STATUS "Building x86 binaries")
        set(PLUGIFY_ARCH "x86")
        set(PLUGIFY_CPU_ARCH_X86 TRUE PARENT_SCOPE)
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|x64|amd64|AMD64)$" OR CMAKE_GENERATOR_PLATFORM STREQUAL "x64") # must be before arm64
        message(STATUS "Building x86_64 binaries")
        set(PLUGIFY_ARCH "x64")
        set(PLUGIFY_CPU_ARCH_X64 TRUE PARENT_SCOPE)
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^(aarch64|arm64|armv8.?|ARM64)$" OR CMAKE_GENERATOR_PLATFORM STREQUAL "ARM64")
        message(STATUS "Building ARM64 binaries")
        set(PLUGIFY_ARCH "arm64")
        set(PLUGIFY_CPU_ARCH_ARM64 TRUE PARENT_SCOPE)
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^(arm|armv[34567]|ARM)$")
        message(STATUS "Building ARM32 binaries")
        set(PLUGIFY_ARCH "arm32")
        set(PLUGIFY_CPU_ARCH_ARM32 TRUE PARENT_SCOPE)
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^(riscv|riscv32|riscv64)$")
        if(CMAKE_SIZEOF_VOID_P EQUAL 4)
            message(STATUS "Building RISC-V 32 binaries")
            set(PLUGIFY_ARCH "riscv32")
            set(PLUGIFY_CPU_ARCH_RISCV32 TRUE PARENT_SCOPE)
        elseif(CMAKE_SIZEOF_VOID_P EQUAL 8)
            message(STATUS "Building RISC-V 64 binaries")
            set(PLUGIFY_ARCH "riscv64")
            set(PLUGIFY_CPU_ARCH_RISCV64 TRUE PARENT_SCOPE)
        else()
            message(FATAL_ERROR "Unknown sizeof void: ${CMAKE_SIZEOF_VOID_P}")
        endif()
    else()
        message(FATAL_ERROR "Unknown architecture: ${CMAKE_SYSTEM_PROCESSOR}")
    endif()

    set(PLUGIFY_PLATFORM "${PLUGIFY_SYSTEM}-${PLUGIFY_ARCH}" PARENT_SCOPE)
    message(STATUS "Building for ${PLUGIFY_SYSTEM}-${PLUGIFY_ARCH}")
endfunction()

function(detect_compiler)
    if(MSVC AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        message(STATUS "Building with Clang-CL")
        set(PLUGIFY_COMPILER_CLANG_CL TRUE PARENT_SCOPE)
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
        message(STATUS "Building with Clang/LLVM")
        set(PLUGIFY_COMPILER_CLANG TRUE PARENT_SCOPE)
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        message(STATUS "Building with GNU GCC")
        set(PLUGIFY_COMPILER_GCC TRUE PARENT_SCOPE)
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
        message(STATUS "Building with Intel")
        set(PLUGIFY_COMPILER_INTEL TRUE PARENT_SCOPE)
    elseif(MSVC)
        message(STATUS "Building with MSVC")
        set(PLUGIFY_COMPILER_MSVC TRUE PARENT_SCOPE)
    else()
        message(FATAL_ERROR "Unknown compiler: ${CMAKE_CXX_COMPILER_ID}")
    endif()
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
