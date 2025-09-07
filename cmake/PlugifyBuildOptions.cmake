# ------------------------------------------------------------------------------
# Compiler stuff
set(PLUGIFY_DEFAULT_CXX_STANDARD 23 CACHE STRING "Default C++ standard to use")
set(PLUGIFY_DEFAULT_C_STANDARD 11 CACHE STRING "Default C standard to use")
option(PLUGIFY_USE_LIBCPP "Use libc++ by adding -stdlib=libc++ flag if available." OFF)
option(PLUGIFY_USE_STATIC_STDLIB "Enable static std library linkage to avoid ABI issues by adding -static-* flags if available." OFF)
option(PLUGIFY_USE_SANITIZER "Enable sanitizers by adding -fsanitize=address -fno-omit-frame-pointer -fsanitize=undefined flags if available." OFF)
option(PLUGIFY_USE_CLANG_TIDY "Enable static analysis with clang-tidy" OFF)
option(PLUGIFY_USE_ABI0 "Enable use of the older C++ ABI, which was the default in GCC versions before GCC 5" OFF)

# ------------------------------------------------------------------------------
# Compilation options
option(PLUGIFY_BUILD_TESTS "Enable building tests." ON)
option(PLUGIFY_BUILD_DOCS "Enable building with documentation." OFF)

option(PLUGIFY_BUILD_OBJECT_LIB "Build plugify as object library." OFF)
option(PLUGIFY_BUILD_SHARED_LIB "Build plugify as shared library." ON)
option(PLUGIFY_BUILD_SHARED_ASMJIT "Build asmjit as shared library." OFF)

option(PLUGIFY_USE_EXTERNAL_GLAZE "Use external glaze library." OFF)
option(PLUGIFY_USE_EXTERNAL_ASMJIT "Use external asmjit library." OFF)
option(PLUGIFY_USE_EXTERNAL_LIBSOLV "Use external libsolv library." OFF)
option(PLUGIFY_USE_EXTERNAL_FMT "Use external fmt library." OFF)

# ------------------------------------------------------------------------------
# Dependencies
set(PLUGIFY_LIBSOLV_REPO "https://github.com/openSUSE/libsolv.git" CACHE STRING "")
set(PLUGIFY_LIBSOLV_TAG "0.7.35" CACHE STRING "")
set(PLUGIFY_ASMJIT_REPO "https://github.com/asmjit/asmjit.git" CACHE STRING "")
set(PLUGIFY_ASMJIT_TAG "7596c6d035c27c9e5faad445f3214f2c971b2f2b" CACHE STRING "")
set(PLUGIFY_FMT_REPO "https://github.com/fmtlib/fmt.git" CACHE STRING "")
set(PLUGIFY_FMT_TAG "10.2.1" CACHE STRING "")
set(PLUGIFY_GLAZE_REPO "https://github.com/stephenberry/glaze.git" CACHE STRING "")
if (PLUGIFY_DEFAULT_CXX_STANDARD GREATER 20)
    set(PLUGIFY_GLAZE_TAG "v5.5.5" CACHE STRING "")
else()
    set(PLUGIFY_GLAZE_TAG "v2.9.5" CACHE STRING "") # Use older version for C++20 compatibility
endif()
