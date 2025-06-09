# ------------------------------------------------------------------------------
# Compiler stuff
option(PLUGIFY_USE_LIBCPP "Use libc++ by adding -stdlib=libc++ flag if available." OFF)
option(PLUGIFY_USE_STATIC_STDLIB "Enable static std library linkage to avoid ABI issues by adding -static-* flags if available." OFF)
option(PLUGIFY_USE_SANITIZER "Enable sanitizers by adding -fsanitize=address -fno-omit-frame-pointer -fsanitize=undefined flags if available." OFF)
option(PLUGIFY_USE_CLANG_TIDY "Enable static analysis with clang-tidy" OFF)
option(PLUGIFY_USE_ABI0 "Enable use of the older C++ ABI, which was the default in GCC versions before GCC 5" OFF)

# ------------------------------------------------------------------------------
# Compilation options
option(PLUGIFY_BUILD_TESTS "Enable building tests." ON)
option(PLUGIFY_BUILD_JIT "Build jit object library." OFF)
option(PLUGIFY_BUILD_ASSEMBLY "Build assembly object library." OFF)
option(PLUGIFY_BUILD_CRASHPAD "Build with crashpad library." OFF)
option(PLUGIFY_BUILD_DOCS "Enable building with documentation." OFF)

option(PLUGIFY_BUILD_OBJECT_LIB "Build plugify as object library." OFF)
option(PLUGIFY_BUILD_SHARED_LIB "Build plugify as shared library." ON)
option(PLUGIFY_BUILD_SHARED_ASMJIT "Build asmjit as shared library." OFF)
option(PLUGIFY_BUILD_SHARED_CURL "Build curl as shared library." OFF)

option(PLUGIFY_USE_EXTERNAL_ASMJIT "Use external asmjit library." OFF)
option(PLUGIFY_USE_EXTERNAL_GLAZE "Use external glaze library." OFF)
option(PLUGIFY_USE_EXTERNAL_CURL "Use external curl library." ON)
option(PLUGIFY_USE_EXTERNAL_FMT "Use external fmt library." OFF)
option(PLUGIFY_USE_EXTERNAL_CPPTRACE "Use external cpptrace library." OFF)

option(PLUGIFY_INTERFACE "Build as lightweight interface for language modules." OFF)
option(PLUGIFY_DOWNLOADER "Enable downloader for package manager." ON)
option(PLUGIFY_LOGGING "Enable logging system." ON)
option(PLUGIFY_DEBUG "Enable debuging mode (asserts)." ON)

# ------------------------------------------------------------------------------
# Dependencies
set(PLUGIFY_ASMJIT_REPO "https://github.com/asmjit/asmjit.git" CACHE STRING "")
set(PLUGIFY_ASMJIT_TAG "f8e7f67b015cb39859f88020183ffb557edf4b8e" CACHE STRING "")
set(PLUGIFY_CPPTRACE_REPO "https://github.com/jeremy-rifkin/cpptrace.git" CACHE STRING "")
set(PLUGIFY_CPPTRACE_TAG "7543677d6f39a38de9e9248e2331015d5235b175" CACHE STRING "")
set(PLUGIFY_CURL_REPO "https://github.com/curl/curl.git" CACHE STRING "")
set(PLUGIFY_CURL_TAG "curl-8_6_0" CACHE STRING "")
set(PLUGIFY_FMT_REPO "https://github.com/fmtlib/fmt.git" CACHE STRING "")
set(PLUGIFY_FMT_TAG "10.2.1" CACHE STRING "")
set(PLUGIFY_GLAZE_REPO "https://github.com/stephenberry/glaze.git" CACHE STRING "")
set(PLUGIFY_GLAZE_TAG "v2.9.5" CACHE STRING "")
set(PLUGIFY_MINIZ_REPO "https://github.com/richgel999/miniz.git" CACHE STRING "")
set(PLUGIFY_MINIZ_TAG "3.0.2" CACHE STRING "")
set(PLUGIFY_CRASHPAD_REPO "https://github.com/TheAssemblyArmada/crashpad-cmake.git" CACHE STRING "")
set(PLUGIFY_CRASHPAD_TAG "80573adcc845071401c73c99eaec7fd9847d45fb" CACHE STRING "")
set(PLUGIFY_BREAKPAD_REPO "https://chromium.googlesource.com/breakpad/breakpad" CACHE STRING "")
set(PLUGIFY_BREAKPAD_TAG "2625edb085169e92cf036c236ac79ab594a7b1cc" CACHE STRING "")
