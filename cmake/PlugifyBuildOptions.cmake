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
option(PLUGIFY_BUILD_CRASHLOGS "Build crashlogs object library." OFF)
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
